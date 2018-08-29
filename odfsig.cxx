/*
 * Copyright 2018 Miklos Vajna. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <functional>
#include <iostream>
#include <memory>
#include <vector>

#include <libxml/parser.h>
#include <xmlsec/io.h>
#include <xmlsec/nss/app.h>
#include <xmlsec/nss/crypto.h>
#include <xmlsec/xmldsig.h>
#include <xmlsec/xmlsec.h>
#include <zip.h>

namespace std
{
template <> struct default_delete<zip_t>
{
    void operator()(zip_t* ptr) { zip_close(ptr); }
};
template <> struct default_delete<zip_file_t>
{
    void operator()(zip_file_t* ptr) { zip_fclose(ptr); }
};
template <> struct default_delete<xmlDoc>
{
    void operator()(xmlDocPtr ptr) { xmlFreeDoc(ptr); }
};
template <> struct default_delete<xmlSecDSigCtx>
{
    void operator()(xmlSecDSigCtxPtr ptr) { xmlSecDSigCtxDestroy(ptr); }
};
template <> struct default_delete<xmlSecKeysMngr>
{
    void operator()(xmlSecKeysMngrPtr ptr) { xmlSecKeysMngrDestroy(ptr); }
};
}

namespace
{
/// Performs libxml init/deinit.
class XmlGuard
{
  public:
    explicit XmlGuard()
    {
        xmlInitParser();
        LIBXML_TEST_VERSION;
    }

    ~XmlGuard() { xmlCleanupParser(); }
};

/// Provides libxmlsec IO callbacks.
namespace XmlSecIO
{
/// All callbacks work on this zip package.
thread_local zip_t* zipArchive;

int match(const char* uri)
{
    if (!zipArchive)
    {
        std::cerr << "error, XmlSecIO::match: no zip archive" << std::endl;
        return 0;
    }

    zip_int64_t signatureZipIndex = zip_name_locate(zipArchive, uri, 0);
    if (signatureZipIndex < 0)
        return 0;

    return 1;
}

void* open(const char* uri)
{
    if (!zipArchive)
    {
        std::cerr << "error, XmlSecIO::open: no zip archive" << std::endl;
        return 0;
    }

    zip_int64_t signatureZipIndex = zip_name_locate(zipArchive, uri, 0);
    if (signatureZipIndex < 0)
    {
        std::cerr << "error, XmlSecIO::open: can't find file: " << uri
                  << std::endl;
        return nullptr;
    }

    zip_file_t* zipFile(zip_fopen_index(zipArchive, signatureZipIndex, 0));
    if (!zipFile)
    {
        std::cerr << "error, main: can't open file at index "
                  << signatureZipIndex << ": " << zip_strerror(zipArchive)
                  << std::endl;
        return nullptr;
    }

    return zipFile;
}

int read(void* context, char* buffer, int len)
{
    auto zipFile = static_cast<zip_file_t*>(context);
    if (!zipFile)
    {
        std::cerr << "error, XmlSecIO::read: no zip file" << std::endl;
        return 0;
    }

    return zip_fread(zipFile, buffer, len);
}

int close(void* context)
{
    auto zipFile = static_cast<zip_file_t*>(context);
    if (!zipFile)
    {
        std::cerr << "error, XmlSecIO::close: no zip file" << std::endl;
        return 0;
    }

    zip_fclose(zipFile);
    return 0;
}
};

/// Performs libxmlsec init/deinit.
class XmlSecGuard
{
  public:
    explicit XmlSecGuard(zip_t* zipArchive)
    {
        // Initialize nss.
        _good = xmlSecNssAppInit(nullptr) >= 0;
        if (!_good)
        {
            std::cerr << "error, XmlSecGuard ctor: nss init failed"
                      << std::endl;
            return;
        }

        // Initialize xmlsec.
        _good = xmlSecInit() >= 0;
        if (!_good)
        {
            std::cerr << "error, XmlSecGuard ctor: xmlsec init failed"
                      << std::endl;
            return;
        }

        // Initialize the nss backend of xmlsec.
        _good = xmlSecNssInit() >= 0;
        if (!_good)
        {
            std::cerr << "error, XmlSecGuard ctor: xmlsec nss init failed"
                      << std::endl;
            return;
        }

        XmlSecIO::zipArchive = zipArchive;
        xmlSecIOCleanupCallbacks();
        xmlSecIORegisterCallbacks(XmlSecIO::match, XmlSecIO::open,
                                  XmlSecIO::read, XmlSecIO::close);
    }

    ~XmlSecGuard()
    {
        if (!_good)
            return;

        xmlSecIOCleanupCallbacks();
        xmlSecIORegisterDefaultCallbacks();
        XmlSecIO::zipArchive = nullptr;

        _good = xmlSecNssShutdown() >= 0;
        if (!_good)
        {
            std::cerr << "error, XmlSecGuard dtor: xmlsec nss shutdown failed"
                      << std::endl;
            return;
        }

        _good = xmlSecShutdown() >= 0;
        if (!_good)
        {
            std::cerr << "error, XmlSecGuard dtor: xmlsec shutdown failed"
                      << std::endl;
            return;
        }

        _good = xmlSecNssAppShutdown() >= 0;
        if (!_good)
        {
            std::cerr << "error, XmlSecGuard dtor: nss shutdown failed"
                      << std::endl;
            return;
        }
    }

    bool isGood() const { return _good; }

  private:
    bool _good = false;
};

bool verifySignature(xmlNode* signature, size_t signatureIndex)
{
    std::cerr << "Signature #" << (signatureIndex + 1) << ":" << std::endl;

    std::unique_ptr<xmlSecKeysMngr> pKeysMngr(xmlSecKeysMngrCreate());
    if (!pKeysMngr)
    {
        std::cerr << "error, verifySignature: keys manager creation failed"
                  << std::endl;
        return false;
    }

    if (xmlSecNssAppDefaultKeysMngrInit(pKeysMngr.get()) < 0)
    {
        std::cerr << "error, verifySignature: keys manager init failed"
                  << std::endl;
        return false;
    }

    std::unique_ptr<xmlSecDSigCtx> dsigCtx(
        xmlSecDSigCtxCreate(pKeysMngr.get()));
    if (!dsigCtx)
    {
        std::cerr << "error, verifySignature: ctx initialize failed"
                  << std::endl;
        return false;
    }

    dsigCtx->keyInfoReadCtx.flags |=
        XMLSEC_KEYINFO_FLAGS_X509DATA_DONT_VERIFY_CERTS;

    if (xmlSecDSigCtxVerify(dsigCtx.get(), signature) < 0)
    {
        std::cerr << "error, verifySignature: ctx verify failed" << std::endl;
        return false;
    }

    if (dsigCtx->status != xmlSecDSigStatusSucceeded)
    {
        std::cerr << "  - Signature Validation: Failed." << std::endl;
        return false;
    }

    std::cerr << "  - Signature Validation: Succeeded." << std::endl;
    std::cerr << "  - Certificate Validation: Not Implemented." << std::endl;
    return true;
}
}

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << " <ODF-file>" << std::endl;
        return 1;
    }

    std::string odfPath(argv[1]);
    int openFlags = 0;
    int errorCode = 0;
    std::unique_ptr<zip_t> zipArchive(
        zip_open(odfPath.c_str(), openFlags, &errorCode));
    if (!zipArchive)
    {
        zip_error_t zipError;
        zip_error_init_with_code(&zipError, errorCode);
        std::cerr << "Can't open zip archive '" << odfPath
                  << "': " << zip_error_strerror(&zipError) << "." << std::endl;
        zip_error_fini(&zipError);
        return 1;
    }

    zip_flags_t locateFlags = 0;
    zip_int64_t signatureZipIndex = zip_name_locate(
        zipArchive.get(), "META-INF/documentsignatures.xml", locateFlags);
    if (signatureZipIndex < 0)
    {
        std::cerr << "File '" << odfPath << "' does not contain any signatures."
                  << std::endl;
        return 1;
    }

    std::unique_ptr<zip_file_t> zipFile(
        zip_fopen_index(zipArchive.get(), signatureZipIndex, 0));
    if (!zipFile)
    {
        std::cerr << "error, main: can't open file at index "
                  << signatureZipIndex << ": " << zip_strerror(zipArchive.get())
                  << std::endl;
        return 1;
    }

    char readBuffer[8192];
    std::vector<char> signaturesBytes;
    zip_int64_t readSize;
    while ((readSize =
                zip_fread(zipFile.get(), readBuffer, sizeof(readBuffer))) > 0)
    {
        signaturesBytes.insert(signaturesBytes.end(), readBuffer,
                               readBuffer + readSize);
    }
    if (readSize == -1)
    {
        std::cerr << "error, main: can't read file at index "
                  << signatureZipIndex << ": "
                  << zip_file_strerror(zipFile.get()) << std::endl;
        return 1;
    }

    XmlGuard xmlGuard;
    std::unique_ptr<xmlDoc> signaturesDoc(
        xmlParseDoc(reinterpret_cast<xmlChar*>(signaturesBytes.data())));
    if (!signaturesDoc)
    {
        std::cerr << "error, main: parsing the signatures file failed"
                  << std::endl;
        return 1;
    }

    xmlNode* signaturesRoot = xmlDocGetRootElement(signaturesDoc.get());
    if (!signaturesRoot)
    {
        std::cerr << "error, main: could not get signatures root" << std::endl;
        return 1;
    }

    std::vector<xmlNode*> signatures;
    for (xmlNode* signature = signaturesRoot->children; signature;
         signature = signature->next)
    {
        signatures.push_back(signature);
    }
    if (signatures.empty())
    {
        std::cerr << "File '" << odfPath << "' does not contain any signatures."
                  << std::endl;
        return 1;
    }

    std::cerr << "Digital Signature Info of: " << odfPath << std::endl;
    XmlSecGuard xmlSecGuard(zipArchive.get());
    if (!xmlSecGuard.isGood())
    {
        std::cerr << "error, main: xmlsec init failed" << std::endl;
        return 1;
    }

    for (size_t signatureIndex = 0; signatureIndex < signatures.size();
         ++signatureIndex)
    {
        if (!verifySignature(signatures[signatureIndex], signatureIndex))
        {
            std::cerr << "error, main: signature verify failed" << std::endl;
            return 1;
        }
    }

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
