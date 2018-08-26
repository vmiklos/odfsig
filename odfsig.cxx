/*
 * Copyright 2018 Miklos Vajna. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <iostream>
#include <memory>
#include <vector>

#include <libxml/parser.h>
// clang-format off
#include <xmlsec/xmlsec.h>
#include <xmlsec/app.h>
#include <xmlsec/dl.h>
// clang-format on
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

/// Performs libxmlsec init/deinit.
class XmlSecGuard
{
  public:
    explicit XmlSecGuard()
    {
        _good = xmlSecInit() >= 0;
        if (!_good)
        {
            std::cerr << "error, XmlSecGuard ctor: xmlsec init failed"
                      << std::endl;
            return;
        }

        _good = xmlSecCheckVersion() == 1;
        if (!_good)
        {
            std::cerr << "error, XmlSecGuard ctor: xmlsec version check failed"
                      << std::endl;
            return;
        }

        _good = xmlSecCryptoDLLoadLibrary(BAD_CAST("nss")) >= 0;
        if (!_good)
        {
            std::cerr << "error, XmlSecGuard ctor: xmlsec crypto load failed"
                      << std::endl;
            return;
        }

        _good = xmlSecCryptoAppInit(nullptr) >= 0;
        if (!_good)
        {
            std::cerr
                << "error, XmlSecGuard ctor: xmlsec crypto app init failed"
                << std::endl;
            return;
        }

        _good = xmlSecCryptoInit() >= 0;
        if (!_good)
        {
            std::cerr << "error, XmlSecGuard ctor: xmlsec crypto init failed"
                      << std::endl;
            return;
        }
    }

    ~XmlSecGuard()
    {
        if (!_good)
            return;

        _good = xmlSecCryptoShutdown() >= 0;
        if (!_good)
        {
            std::cerr
                << "error, XmlSecGuard dtor: xmlsec crypto shutdown failed"
                << std::endl;
            return;
        }

        _good = xmlSecCryptoAppShutdown() >= 0;
        if (!_good)
        {
            std::cerr
                << "error, XmlSecGuard dtor: xmlsec crypto app shutdown failed"
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
    }

    bool isGood() const { return _good; }

  private:
    bool _good = false;
};

bool verifySignature(xmlNode* signature, size_t signatureIndex)
{
    std::cerr << "Signature #" << (signatureIndex + 1) << ":" << std::endl;

    std::cerr << "todo, verifySignature: verify " << signature << std::endl;

    return false;
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
    XmlSecGuard xmlSecGuard;
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
