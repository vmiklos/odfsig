/*
 * Copyright 2018 Miklos Vajna. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <odfsig/lib.hxx>

#include <algorithm>
#include <cassert>
#include <cstring>
#include <fstream>
#include <iterator>
#include <sstream>
#include <utility>

#include <libxml/globals.h>
#include <libxml/parser.h>
#include <libxml/xmlstring.h>
#include <libxml/xmlversion.h>
#include <xmlsec/base64.h>
#include <xmlsec/buffer.h>
#include <xmlsec/errors.h>
#include <xmlsec/io.h>
#include <xmlsec/keyinfo.h>
#include <xmlsec/keysmngr.h>
#include <xmlsec/strings.h>
#include <xmlsec/transforms.h>
#include <xmlsec/xmldsig.h>
#include <xmlsec/xmlsec.h>
#include <xmlsec/xmltree.h>
#include <zip.h>
#include <zipconf.h>

#include <odfsig/crypto.hxx>
#include <odfsig/string.hxx>

namespace std
{
template <> struct default_delete<zip_t>
{
    void operator()(zip_t* ptr) { zip_close(ptr); }
};
template <> struct default_delete<zip_source_t>
{
    void operator()(zip_source_t* ptr) { zip_source_free(ptr); }
};
template <> struct default_delete<zip_file_t>
{
    void operator()(zip_file_t* ptr) { zip_fclose(ptr); }
};
template <> struct default_delete<zip_error_t>
{
    void operator()(zip_error_t* ptr)
    {
        zip_error_fini(ptr);
        delete ptr;
    }
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
template <> struct default_delete<xmlChar>
{
    void operator()(xmlChar* ptr) { xmlFree(ptr); }
};
template <> struct default_delete<xmlSecTransformCtx>
{
    void operator()(xmlSecTransformCtxPtr ptr)
    {
        xmlSecTransformCtxDestroy(ptr);
    }
};
}

namespace
{
const xmlChar* dateNodeName = BAD_CAST("date");
const xmlChar* dateNsName = BAD_CAST("http://purl.org/dc/elements/1.1/");
const xmlChar* xadesNsName = BAD_CAST("http://uri.etsi.org/01903/v1.3.2#");
const char* signaturesStreamName = "META-INF/documentsignatures.xml";

/// Checks if `big` ends with `suffix`.
bool ends_with(const std::string& big, const std::string& suffix)
{
    return big.compare(big.length() - suffix.length(), suffix.length(),
                       suffix) == 0;
}

/// Converts from libxml char to normal char.
const char* fromXmlChar(const xmlChar* s)
{
    return reinterpret_cast<const char*>(s);
}
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

/// The error callback works on this stream.
thread_local std::ostream* xmlSecErrorStream;

/// Provides a libxmlsec error callback.
void xmlSecErrorsCallback(const char* file, int line, const char* func,
                          const char* errorObject, const char* errorSubject,
                          int reason, const char* msg)
{
    if (xmlSecErrorStream == nullptr)
    {
        return;
    }

    const char* errorMsg = nullptr;
    for (xmlSecSize i = 0;
         i < XMLSEC_ERRORS_MAX_NUMBER && xmlSecErrorsGetMsg(i) != nullptr; ++i)
    {
        if (xmlSecErrorsGetCode(i) == reason)
        {
            errorMsg = xmlSecErrorsGetMsg(i);
            break;
        }
    }

    std::ostream& ostream = *xmlSecErrorStream;
    ostream << "func=" << (func != nullptr ? func : "unknown");
    ostream << ":file=" << (file != nullptr ? file : "unknown");
    ostream << ":line=" << line;
    ostream << ":obj=" << (errorObject != nullptr ? errorObject : "unknown");
    ostream << ":subj=" << (errorSubject != nullptr ? errorSubject : "unknown");
    ostream << ":error=" << reason;
    ostream << ":" << (errorMsg != nullptr ? errorMsg : "unknown");
    ostream << ":" << (msg != nullptr ? msg : "") << std::endl;
}

/// Provides libxmlsec IO callbacks.
namespace XmlSecIO
{
/// All callbacks work on this zip package.
thread_local zip_t* zipArchive;

int match(const char* uri)
{
    assert(zipArchive);

    zip_int64_t signatureZipIndex = zip_name_locate(zipArchive, uri, 0);
    if (signatureZipIndex < 0)
    {
        return 0;
    }

    return 1;
}

void* open(const char* uri)
{
    assert(zipArchive);

    zip_int64_t signatureZipIndex = zip_name_locate(zipArchive, uri, 0);
    if (signatureZipIndex < 0)
    {
        return nullptr;
    }

    zip_file_t* zipFile(zip_fopen_index(zipArchive, signatureZipIndex, 0));
    if (zipFile == nullptr)
    {
        return nullptr;
    }

    return zipFile;
}

int read(void* context, char* buffer, int len)
{
    auto zipFile = static_cast<zip_file_t*>(context);
    assert(zipFile);

    return zip_fread(zipFile, buffer, len);
}

int close(void* context)
{
    auto zipFile = static_cast<zip_file_t*>(context);
    assert(zipFile);

    zip_fclose(zipFile);
    return 0;
}
};

/// Performs libxmlsec init/deinit.
class XmlSecGuard
{
  public:
    explicit XmlSecGuard(zip_t* zipArchive, std::ostream* errorStream,
                         Crypto& crypto)
        : _crypto(crypto)
    {
        // Initialize xmlsec.
        _good = xmlSecInit() >= 0;
        if (!_good)
        {
            return;
        }

        if (!_crypto.xmlSecInitialize())
        {
            return;
        }

        XmlSecIO::zipArchive = zipArchive;
        xmlSecIOCleanupCallbacks();
        xmlSecIORegisterCallbacks(XmlSecIO::match, XmlSecIO::open,
                                  XmlSecIO::read, XmlSecIO::close);

        if (errorStream != nullptr)
        {
            xmlSecErrorStream = errorStream;
            xmlSecErrorsSetCallback(xmlSecErrorsCallback);
        }
    }

    ~XmlSecGuard()
    {
        if (!_good)
        {
            return;
        }

        if (xmlSecErrorStream != nullptr)
        {
            xmlSecErrorsSetCallback(xmlSecErrorsDefaultCallback);
            xmlSecErrorStream = nullptr;
        }

        xmlSecIOCleanupCallbacks();
        xmlSecIORegisterDefaultCallbacks();
        XmlSecIO::zipArchive = nullptr;

        if (!_crypto.xmlSecShutdown())
        {
            return;
        }

        _good = xmlSecShutdown() >= 0;
        if (!_good)
        {
            return;
        }

        if (!_crypto.shutdown())
        {
            return;
        }
    }

    bool isGood() const { return _good; }

  private:
    bool _good = false;
    Crypto& _crypto;
};

/// Implementation of Signature using libxml.
class XmlSignature : public Signature
{
  public:
    explicit XmlSignature(xmlNode* signatureNode, Crypto& crypto,
                          std::vector<std::string> trustedDers, bool insecure);
    ~XmlSignature() override;

    const std::string& getErrorString() const override;

    bool verify() override;

    bool verifyXAdES() override;

    std::string getSubjectName() const override;

    std::string getDate() const override;

    std::string getMethod() const override;

    std::string getType() const override;

    std::set<std::string> getSignedStreams() const override;

  private:
    std::string getObjectDate(xmlNode* objectNode) const;

    std::string
    getSignaturePropertiesDate(xmlNode* signaturePropertiesNode) const;

    std::string getSignaturePropertyDate(xmlNode* signaturePropertyNode) const;

    std::string getDateContent(xmlNode* dateNode) const;

    xmlNodePtr getObjectCertDigestNode(xmlNode* objectNode) const;

    xmlNodePtr getCertDigestNode() const;

    xmlNodePtr getX509CertificateNode() const;

    bool getCertificateBinary(std::vector<xmlChar>& certificate) const;

    bool getDigestValue(xmlNodePtr certDigest,
                        std::vector<xmlChar>& value) const;

    std::unique_ptr<xmlChar> getDigestAlgo(xmlNodePtr certDigest) const;

    bool hash(const std::vector<xmlChar>& in, std::unique_ptr<xmlChar> algo,
              std::vector<unsigned char>& out) const;

    std::string _errorString;

    xmlNode* _signatureNode = nullptr;

    std::vector<std::string> _trustedDers;

    bool _insecure = false;

    Crypto& _crypto;
};

XmlSignature::XmlSignature(xmlNode* signatureNode, Crypto& crypto,
                           std::vector<std::string> trustedDers, bool insecure)
    : _signatureNode(signatureNode), _trustedDers(std::move(trustedDers)),
      _insecure(insecure), _crypto(crypto)
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

    if (!_crypto.initializeKeysManager(pKeysMngr.get(), _trustedDers))
    {
        _errorString = "Keys manager crypto init or cert load failed";
        return false;
    }

    std::unique_ptr<xmlSecDSigCtx> dsigCtx(
        xmlSecDSigCtxCreate(pKeysMngr.get()));
    if (!dsigCtx)
    {
        _errorString = "DSig context initialize failed";
        return false;
    }

    if (_insecure)
    {
        dsigCtx->keyInfoReadCtx.flags |=
            XMLSEC_KEYINFO_FLAGS_X509DATA_DONT_VERIFY_CERTS;
    }

    if (xmlSecDSigCtxVerify(dsigCtx.get(), _signatureNode) < 0)
    {
        _errorString = "DSig context verify failed";
        return false;
    }

    return dsigCtx->status == xmlSecDSigStatusSucceeded;
}

bool XmlSignature::getCertificateBinary(std::vector<xmlChar>& certificate) const
{
    // Look up the encoded certificate.
    xmlNode* x509Certificate = getX509CertificateNode();
    if (x509Certificate == nullptr)
    {
        return false;
    }

    std::unique_ptr<xmlChar> certificateContent(
        xmlNodeGetContent(x509Certificate));
    if (!certificateContent ||
        (xmlSecIsEmptyString(certificateContent.get()) != 0))
    {
        return false;
    }

    // Decode the certificate in-place.
    int certSize = xmlSecBase64Decode(certificateContent.get(),
                                      (xmlSecByte*)certificateContent.get(),
                                      xmlStrlen(certificateContent.get()));
    if (certSize < 0)
    {
        return false;
    }

    std::copy(certificateContent.get(), certificateContent.get() + certSize,
              std::back_inserter(certificate));
    return true;
}

xmlNodePtr XmlSignature::getCertDigestNode() const
{
    xmlNodePtr certDigestNode = nullptr;
    for (xmlNode* signatureChild = _signatureNode->children;
         signatureChild != nullptr; signatureChild = signatureChild->next)
    {
        if (xmlSecCheckNodeName(signatureChild, xmlSecNodeObject,
                                xmlSecDSigNs) == 0)
        {
            continue;
        }

        certDigestNode = getObjectCertDigestNode(signatureChild);
        if (certDigestNode != nullptr)
        {
            break;
        }
    }

    return certDigestNode;
}

bool XmlSignature::getDigestValue(xmlNodePtr certDigest,
                                  std::vector<xmlChar>& value) const
{
    xmlNode* digestValueNode =
        xmlSecFindChild(certDigest, xmlSecNodeDigestValue, xmlSecDSigNs);
    if (digestValueNode == nullptr)
    {
        return false;
    }

    std::unique_ptr<xmlChar> digestValueNodeContent(
        xmlNodeGetContent(digestValueNode));
    if (!digestValueNodeContent ||
        (xmlSecIsEmptyString(digestValueNodeContent.get()) != 0))
    {
        return false;
    }

    // Decode the expected hash in-place.
    int expectedDigestSize = xmlSecBase64Decode(
        digestValueNodeContent.get(), (xmlSecByte*)digestValueNodeContent.get(),
        xmlStrlen(digestValueNodeContent.get()));
    if (expectedDigestSize < 0)
    {
        return false;
    }

    std::copy(digestValueNodeContent.get(),
              digestValueNodeContent.get() + expectedDigestSize,
              std::back_inserter(value));
    return true;
}

std::unique_ptr<xmlChar>
XmlSignature::getDigestAlgo(xmlNodePtr certDigest) const
{
    xmlNode* digestMethodNode =
        xmlSecFindChild(certDigest, xmlSecNodeDigestMethod, xmlSecDSigNs);
    if (digestMethodNode == nullptr)
    {
        return nullptr;
    }

    std::unique_ptr<xmlChar> algo(
        xmlGetProp(digestMethodNode, xmlSecAttrAlgorithm));
    if (!algo)
    {
        return nullptr;
    }

    return algo;
}

bool XmlSignature::hash(const std::vector<xmlChar>& in,
                        std::unique_ptr<xmlChar> algo,
                        std::vector<unsigned char>& out) const
{
    std::unique_ptr<xmlSecTransformCtx> transform(xmlSecTransformCtxCreate());
    if (!transform)
    {
        return false;
    }

    xmlSecTransformId transformId = xmlSecTransformIdListFindByHref(
        xmlSecTransformIdsGet(), algo.get(), xmlSecTransformUsageDigestMethod);
    if (transformId == xmlSecTransformIdUnknown)
    {
        return false;
    }

    xmlSecTransformPtr hash =
        xmlSecTransformCtxCreateAndAppend(transform.get(), transformId);
    if (hash == nullptr)
    {
        return false;
    }

    hash->operation = xmlSecTransformOperationSign;
    if (xmlSecTransformCtxBinaryExecute(transform.get(), in.data(), in.size()) <
        0)
    {
        return false;
    }

    std::copy(transform->result->data,
              transform->result->data + transform->result->size,
              std::back_inserter(out));
    return true;
}

bool XmlSignature::verifyXAdES()
{
    std::vector<xmlChar> certificate;
    if (!getCertificateBinary(certificate))
    {
        _errorString = "could not find certificate";
        return false;
    }

    xmlNodePtr certDigestNode = getCertDigestNode();
    if (certDigestNode == nullptr)
    {
        _errorString = "could not find certificate digest node";
        return false;
    }

    std::vector<xmlChar> expectedDigest;
    if (!getDigestValue(certDigestNode, expectedDigest))
    {
        _errorString = "could not find digest value";
        return false;
    }

    std::unique_ptr<xmlChar> algo = getDigestAlgo(certDigestNode);
    if (!algo)
    {
        _errorString = "could not find digest algo";
        return false;
    }

    std::vector<unsigned char> actualDigest;
    if (!hash(certificate, std::move(algo), actualDigest))
    {
        _errorString = "could not hash certificate";
        return false;
    }

    return std::memcmp(expectedDigest.data(), actualDigest.data(),
                       actualDigest.size()) == 0;
}

xmlNode* XmlSignature::getX509CertificateNode() const
{
    xmlNode* keyInfoNode =
        xmlSecFindChild(_signatureNode, xmlSecNodeKeyInfo, xmlSecDSigNs);
    xmlNode* x509Data =
        xmlSecFindChild(keyInfoNode, xmlSecNodeX509Data, xmlSecDSigNs);
    return xmlSecFindChild(x509Data, xmlSecNodeX509Certificate, xmlSecDSigNs);
}

std::string XmlSignature::getSubjectName() const
{
    std::vector<xmlChar> certificate;
    if (!getCertificateBinary(certificate))
    {
        return std::string();
    }

    return _crypto.getCertificateSubjectName(certificate.data(),
                                             certificate.size());
}

std::string XmlSignature::getMethod() const
{
    xmlNode* signedInfoNode =
        xmlSecFindChild(_signatureNode, xmlSecNodeSignedInfo, xmlSecDSigNs);
    xmlNode* signatureMethodNode = xmlSecFindChild(
        signedInfoNode, xmlSecNodeSignatureMethod, xmlSecDSigNs);
    if (signatureMethodNode == nullptr)
    {
        return std::string();
    }

    std::unique_ptr<xmlChar> href(
        xmlGetProp(signatureMethodNode, xmlSecAttrAlgorithm));
    if (!href)
    {
        return std::string();
    }

    xmlSecTransformId id =
        xmlSecTransformIdListFindByHref(xmlSecTransformIdsGet(), href.get(),
                                        xmlSecTransformUsageSignatureMethod);
    if (id == xmlSecTransformIdUnknown)
    {
        return fromXmlChar(href.get());
    }

    return std::string(fromXmlChar(xmlSecTransformKlassGetName(id)));
}

std::set<std::string> XmlSignature::getSignedStreams() const
{
    xmlNode* signedInfoNode =
        xmlSecFindChild(_signatureNode, xmlSecNodeSignedInfo, xmlSecDSigNs);
    if (signedInfoNode == nullptr)
    {
        return {};
    }

    std::set<std::string> signedStreams;
    for (xmlNode* signedInfoChild = signedInfoNode->children;
         signedInfoChild != nullptr; signedInfoChild = signedInfoChild->next)
    {
        if (xmlSecCheckNodeName(signedInfoChild, xmlSecNodeReference,
                                xmlSecDSigNs) == 0)
        {
            continue;
        }

        std::unique_ptr<xmlChar> uriProp(
            xmlGetProp(signedInfoChild, xmlSecAttrURI));
        if (!uriProp)
        {
            continue;
        }

        std::string uri(fromXmlChar(uriProp.get()));
        if (starts_with(uri, "#"))
        {
            continue;
        }

        signedStreams.insert(uri);
    }

    return signedStreams;
}

std::string XmlSignature::getType() const
{
    if (getCertDigestNode() != nullptr)
    {
        return std::string("XAdES");
    }

    return std::string("XML-DSig");
}

xmlNodePtr XmlSignature::getObjectCertDigestNode(xmlNode* objectNode) const
{
    const xmlChar* qualifyingPropertiesNodeName =
        BAD_CAST("QualifyingProperties");
    const xmlChar* signedPropertiesNodeName = BAD_CAST("SignedProperties");
    const xmlChar* signedSignaturePropertiesNodeName =
        BAD_CAST("SignedSignatureProperties");
    const xmlChar* signingCertificateNodeName = BAD_CAST("SigningCertificate");
    const xmlChar* certNodeName = BAD_CAST("Cert");
    const xmlChar* certDigestNodeName = BAD_CAST("CertDigest");

    xmlNode* qualifyingPropertiesNode =
        xmlSecFindChild(objectNode, qualifyingPropertiesNodeName, xadesNsName);
    if (qualifyingPropertiesNode == nullptr)
    {
        return nullptr;
    }

    xmlNode* signedPropertiesNode = xmlSecFindChild(
        qualifyingPropertiesNode, signedPropertiesNodeName, xadesNsName);
    if (signedPropertiesNode == nullptr)
    {
        return nullptr;
    }

    xmlNode* signedSignaturePropertiesNode = xmlSecFindChild(
        signedPropertiesNode, signedSignaturePropertiesNodeName, xadesNsName);
    if (signedSignaturePropertiesNode == nullptr)
    {
        return nullptr;
    }

    xmlNode* signingCertificateNode = xmlSecFindChild(
        signedSignaturePropertiesNode, signingCertificateNodeName, xadesNsName);
    if (signingCertificateNode == nullptr)
    {
        return nullptr;
    }

    xmlNode* certNode =
        xmlSecFindChild(signingCertificateNode, certNodeName, xadesNsName);
    if (certNode == nullptr)
    {
        return nullptr;
    }

    return xmlSecFindChild(certNode, certDigestNodeName, xadesNsName);
}

std::string XmlSignature::getDate() const
{
    for (xmlNode* signatureChild = _signatureNode->children;
         signatureChild != nullptr; signatureChild = signatureChild->next)
    {
        if (xmlSecCheckNodeName(signatureChild, xmlSecNodeObject,
                                xmlSecDSigNs) == 0)
        {
            continue;
        }

        std::string date = getObjectDate(signatureChild);
        if (!date.empty())
        {
            return date;
        }
    }

    return std::string();
}

std::string XmlSignature::getObjectDate(xmlNode* objectNode) const
{
    for (xmlNode* objectChild = objectNode->children; objectChild != nullptr;
         objectChild = objectChild->next)
    {
        if (xmlSecCheckNodeName(objectChild, xmlSecNodeSignatureProperties,
                                xmlSecDSigNs) == 0)
        {
            continue;
        }

        std::string date = getSignaturePropertiesDate(objectChild);
        if (!date.empty())
        {
            return date;
        }
    }

    return std::string();
}

std::string XmlSignature::getDateContent(xmlNode* dateNode) const
{
    std::unique_ptr<xmlChar> content(xmlNodeGetContent(dateNode));
    if (!content)
    {
        return std::string();
    }

    return std::string(fromXmlChar(content.get()));
}

std::string
XmlSignature::getSignaturePropertiesDate(xmlNode* signaturePropertiesNode) const
{
    const xmlChar* signaturePropertyNodeName = BAD_CAST("SignatureProperty");

    for (xmlNode* signaturePropertiesChild = signaturePropertiesNode->children;
         signaturePropertiesChild != nullptr;
         signaturePropertiesChild = signaturePropertiesChild->next)
    {
        if (xmlSecCheckNodeName(signaturePropertiesChild,
                                signaturePropertyNodeName, xmlSecDSigNs) == 0)
        {
            continue;
        }

        std::string date = getSignaturePropertyDate(signaturePropertiesChild);
        if (!date.empty())
        {
            return date;
        }
    }

    return std::string();
}

std::string
XmlSignature::getSignaturePropertyDate(xmlNode* signaturePropertyNode) const
{
    xmlNode* dateNode =
        xmlSecFindChild(signaturePropertyNode, dateNodeName, dateNsName);
    if (dateNode == nullptr)
    {
        return std::string();
    }

    return getDateContent(dateNode);
}

/// Implementation of Verifier using libzip.
class ZipVerifier : public Verifier
{
  public:
    explicit ZipVerifier(const std::string& cryptoConfig);

    bool openZip(const std::string& path) override;

    bool openZipMemory(const void* data, size_t size) override;

    const std::string& getErrorString() const override;

    void setTrustedDers(const std::vector<std::string>& trustedDers) override;

    void setInsecure(bool insecure) override;

    void setLogger(std::ostream& logger) override;

    bool parseSignatures() override;

    std::vector<std::unique_ptr<Signature>>& getSignatures() override;

    std::set<std::string> getStreams() const override;

  private:
    bool locateSignatures();

    std::vector<char> _zipContents;

    std::unique_ptr<zip_source_t> _zipSource;

    std::unique_ptr<zip_t> _zipArchive;

    std::string _errorString;

    zip_int64_t _signaturesZipIndex = 0;

    std::unique_ptr<XmlGuard> _xmlGuard;

    std::unique_ptr<Crypto> _crypto;

    std::unique_ptr<XmlSecGuard> _xmlSecGuard;

    std::unique_ptr<zip_file_t> _zipFile;

    std::vector<char> _signaturesBytes;

    std::unique_ptr<xmlDoc> _signaturesDoc;

    std::vector<std::unique_ptr<Signature>> _signatures;

    std::string _cryptoConfig;

    std::vector<std::string> _trustedDers;

    bool _insecure = false;

    std::ostream* _logger = nullptr;
};

std::unique_ptr<Verifier> Verifier::create(const std::string& cryptoConfig)
{
    return std::unique_ptr<Verifier>(new ZipVerifier(cryptoConfig));
}

ZipVerifier::ZipVerifier(const std::string& cryptoConfig)
{
    _cryptoConfig = cryptoConfig;
}

bool ZipVerifier::openZip(const std::string& path)
{
    std::ifstream stream(path, std::ios::binary);
    if (!stream.is_open())
    {
        return false;
    }

    _zipContents = std::vector<char>((std::istreambuf_iterator<char>(stream)),
                                     std::istreambuf_iterator<char>());
    return openZipMemory(_zipContents.data(), _zipContents.size());
}

bool ZipVerifier::openZipMemory(const void* data, size_t size)
{
    auto zipError = std::make_unique<zip_error_t>();
    zip_error_init(zipError.get());
    _zipSource.reset(zip_source_buffer_create(data, size, 0, zipError.get()));
    if (!_zipSource)
    {
        _errorString = zip_error_strerror(zipError.get());
        return false;
    }

    const int openFlags = 0;
    _zipArchive.reset(
        zip_open_from_source(_zipSource.get(), openFlags, zipError.get()));
    if (!_zipArchive)
    {
        _errorString = zip_error_strerror(zipError.get());
        return false;
    }

    zip_source_keep(_zipSource.get());
    return true;
}

const std::string& ZipVerifier::getErrorString() const { return _errorString; }

void ZipVerifier::setTrustedDers(const std::vector<std::string>& trustedDers)
{
    _trustedDers = trustedDers;
}

void ZipVerifier::setInsecure(bool insecure) { _insecure = insecure; }

void ZipVerifier::setLogger(std::ostream& logger) { _logger = &logger; }

bool ZipVerifier::parseSignatures()
{
    if (!locateSignatures())
    {
        // No problem, later getSignatures() will return an empty list.
        return true;
    }

    _xmlGuard = std::make_unique<XmlGuard>();

    _crypto = Crypto::create();
    if (!_crypto->initialize(_cryptoConfig))
    {
        _errorString = "Failed to initialize crypto";
        return false;
    }

    _xmlSecGuard =
        std::make_unique<XmlSecGuard>(_zipArchive.get(), _logger, *_crypto);
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

    const int bufferSize = 8192;
    std::vector<char> readBuffer(bufferSize);
    zip_int64_t readSize;
    while ((readSize = zip_fread(_zipFile.get(), readBuffer.data(),
                                 readBuffer.size())) > 0)
    {
        _signaturesBytes.insert(_signaturesBytes.end(), readBuffer.data(),
                                readBuffer.data() + readSize);
    }
    if (readSize == -1)
    {
        std::stringstream ss;
        ss << "Can't read file at index " << _signaturesZipIndex << ": "
           << zip_file_strerror(_zipFile.get());
        _errorString = ss.str();
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
    if (signaturesRoot == nullptr)
    {
        _errorString = "Could not get the signatures root";
        return false;
    }

    for (xmlNode* signatureNode = signaturesRoot->children;
         signatureNode != nullptr; signatureNode = signatureNode->next)
    {
        _signatures.push_back(std::unique_ptr<Signature>(new XmlSignature(
            signatureNode, *_crypto, _trustedDers, _insecure)));
    }

    return true;
}

std::vector<std::unique_ptr<Signature>>& ZipVerifier::getSignatures()
{
    return _signatures;
}

std::set<std::string> ZipVerifier::getStreams() const
{
    std::set<std::string> streams;
    zip_int64_t numEntries = zip_get_num_entries(_zipArchive.get(), 0);
    if (numEntries < 0)
    {
        return streams;
    }

    for (zip_int64_t entry = 0; entry < numEntries; ++entry)
    {
        const char* name = zip_get_name(_zipArchive.get(), entry, 0);
        if (name == nullptr)
        {
            continue;
        }

        std::string stream(name);
        if (ends_with(stream, "/"))
        {
            continue;
        }

        if (stream == signaturesStreamName)
        {
            continue;
        }

        streams.insert(zip_get_name(_zipArchive.get(), entry, 0));
    }
    return streams;
}

bool ZipVerifier::locateSignatures()
{
    zip_flags_t locateFlags = 0;
    _signaturesZipIndex =
        zip_name_locate(_zipArchive.get(), signaturesStreamName, locateFlags);

    return _signaturesZipIndex >= 0;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
