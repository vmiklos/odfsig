/*
 * Copyright 2018 Miklos Vajna. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "odfsig.hxx"

#include <iostream>
#include <sstream>

#include <xmlsec/io.h>
#include <xmlsec/nss/app.h>
#include <xmlsec/nss/crypto.h>
#include <xmlsec/xmldsig.h>
#include <zip.h>

namespace std
{
template <> struct default_delete<zip>
{
    void operator()(zip* ptr) { zip_close(ptr); }
};
template <> struct default_delete<zip_file>
{
    void operator()(zip_file* ptr) { zip_fclose(ptr); }
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

namespace odfsig
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
thread_local zip* zipArchive;

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

    zip_file* zipFile(zip_fopen_index(zipArchive, signatureZipIndex, 0));
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
    auto zipFile = static_cast<zip_file*>(context);
    if (!zipFile)
    {
        std::cerr << "error, XmlSecIO::read: no zip file" << std::endl;
        return 0;
    }

    return zip_fread(zipFile, buffer, len);
}

int close(void* context)
{
    auto zipFile = static_cast<zip_file*>(context);
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
    explicit XmlSecGuard(zip* zipArchive)
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

/// Implementation of Signature using libxml.
class XmlSignature : public Signature
{
  public:
    explicit XmlSignature(xmlNode* signatureNode);
    virtual ~XmlSignature();

    const std::string& getErrorString() const override;

    bool verify() override;

  private:
    std::string _errorString;

    xmlNode* _signatureNode = nullptr;
};

XmlSignature::XmlSignature(xmlNode* signatureNode)
    : _signatureNode(signatureNode)
{
}

XmlSignature::~XmlSignature() = default;

const std::string& XmlSignature::getErrorString() const { return _errorString; }

bool XmlSignature::verify()
{
    std::unique_ptr<xmlSecKeysMngr> pKeysMngr(xmlSecKeysMngrCreate());
    if (!pKeysMngr)
    {
        _errorString = "Keys manager creation failed";
        return false;
    }

    if (xmlSecNssAppDefaultKeysMngrInit(pKeysMngr.get()) < 0)
    {
        _errorString = "Keys manager init failed";
        return false;
    }

    std::unique_ptr<xmlSecDSigCtx> dsigCtx(
        xmlSecDSigCtxCreate(pKeysMngr.get()));
    if (!dsigCtx)
    {
        _errorString = "DSig context initialize failed";
        return false;
    }

    dsigCtx->keyInfoReadCtx.flags |=
        XMLSEC_KEYINFO_FLAGS_X509DATA_DONT_VERIFY_CERTS;

    if (xmlSecDSigCtxVerify(dsigCtx.get(), _signatureNode) < 0)
    {
        _errorString = "DSig context verify failed";
        return false;
    }

    return dsigCtx->status == xmlSecDSigStatusSucceeded;
}

/// Implementation of Verifier using libzip.
class ZipVerifier : public Verifier
{
  public:
    bool openZip(const std::string& path) override;

    const std::string& getErrorString() const override;

    bool parseSignatures() override;

    std::vector<std::unique_ptr<Signature>>& getSignatures() override;

  private:
    bool locateSignatures();

    std::unique_ptr<zip> _zipArchive;
    std::string _errorString;
    zip_int64_t _signaturesZipIndex = 0;
    std::unique_ptr<XmlGuard> _xmlGuard;
    std::unique_ptr<XmlSecGuard> _xmlSecGuard;
    std::unique_ptr<zip_file> _zipFile;
    std::vector<char> _signaturesBytes;
    std::unique_ptr<xmlDoc> _signaturesDoc;
    std::vector<std::unique_ptr<Signature>> _signatures;
};

std::unique_ptr<Verifier> Verifier::create()
{
    return std::unique_ptr<Verifier>(new ZipVerifier());
}

bool ZipVerifier::openZip(const std::string& path)
{
    const int openFlags = 0;
    int errorCode = 0;
    _zipArchive.reset(zip_open(path.c_str(), openFlags, &errorCode));
    if (!_zipArchive)
    {
        char buf[100];
        zip_error_to_str(buf, sizeof(buf), errorCode, errno);
        _errorString = buf;
        return false;
    }

    return true;
}

const std::string& ZipVerifier::getErrorString() const { return _errorString; }

bool ZipVerifier::parseSignatures()
{
    if (!locateSignatures())
        // No problem, later getSignatures() will return an empty list.
        return true;

    _xmlGuard.reset(new XmlGuard());

    _xmlSecGuard.reset(new XmlSecGuard(_zipArchive.get()));
    if (!_xmlSecGuard->isGood())
    {
        _errorString = "Failed to initialize libxmlsec";
        return false;
    }

    _zipFile.reset(zip_fopen_index(_zipArchive.get(), _signaturesZipIndex, 0));
    if (!_zipFile)
    {
        std::stringstream ss;
        ss << "Can't open file at index " << _signaturesZipIndex << ":"
           << zip_strerror(_zipArchive.get());
        _errorString = ss.str();
        return false;
    }

    char readBuffer[8192];
    zip_int64_t readSize;
    while ((readSize =
                zip_fread(_zipFile.get(), readBuffer, sizeof(readBuffer))) > 0)
    {
        _signaturesBytes.insert(_signaturesBytes.end(), readBuffer,
                                readBuffer + readSize);
    }
    if (readSize == -1)
    {
        std::stringstream ss;
        ss << "Can't read file at index " << _signaturesZipIndex << ": "
           << zip_file_strerror(_zipFile.get());
        return false;
    }

    // xmlParseDoc() expects a zero-terminated string.
    _signaturesBytes.push_back(0);
    _signaturesDoc.reset(
        xmlParseDoc(reinterpret_cast<xmlChar*>(_signaturesBytes.data())));
    if (!_signaturesDoc)
    {
        _errorString = "Parsing the signatures file failed";
        return false;
    }

    xmlNode* signaturesRoot = xmlDocGetRootElement(_signaturesDoc.get());
    if (!signaturesRoot)
    {
        _errorString = "Could not get the signatures root";
        return false;
    }

    for (xmlNode* signatureNode = signaturesRoot->children; signatureNode;
         signatureNode = signatureNode->next)
        _signatures.push_back(
            std::unique_ptr<Signature>(new XmlSignature(signatureNode)));

    return true;
}

std::vector<std::unique_ptr<Signature>>& ZipVerifier::getSignatures()
{
    return _signatures;
}

bool ZipVerifier::locateSignatures()
{
    int locateFlags = 0;
    _signaturesZipIndex = zip_name_locate(
        _zipArchive.get(), "META-INF/documentsignatures.xml", locateFlags);

    return _signaturesZipIndex >= 0;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
