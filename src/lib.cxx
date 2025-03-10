/*
 * Copyright 2018 Miklos Vajna
 *
 * SPDX-License-Identifier: MIT
 */

#include <odfsig/lib.hxx>

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iterator>
#include <sstream>
#include <utility>

#include <libxml/parser.h>
#include <libxml/xmlmemory.h>
#include <libxml/xmlstring.h>
#include <libxml/xmlversion.h>
#include <xmlsec/base64.h>
#include <xmlsec/buffer.h>
#include <xmlsec/io.h>
#include <xmlsec/keyinfo.h>
#include <xmlsec/keysmngr.h>
#include <xmlsec/strings.h>
#include <xmlsec/transforms.h>
#include <xmlsec/xmldsig.h>
#include <xmlsec/xmlsec.h>
#include <xmlsec/xmltree.h>

#include <odfsig/crypto.hxx>

#include "zip.hxx"

namespace std
{
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
} // namespace std

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
const char* fromXmlChar(const xmlChar* string)
{
    return reinterpret_cast<const char*>(string);
}
} // namespace

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
thread_local zip::Archive* zipArchive;

int match(const char* uri)
{
    assert(zipArchive);

    int64_t signatureZipIndex = zipArchive->locateName(uri);
    if (signatureZipIndex < 0)
    {
        return 0;
    }

    return 1;
}

void* open(const char* uri)
{
    assert(zipArchive);

    int64_t signatureZipIndex = zipArchive->locateName(uri);
    if (signatureZipIndex < 0)
    {
        return nullptr;
    }

    std::unique_ptr<zip::File> zipFile =
        zip::File::create(zipArchive, signatureZipIndex);
    if (zipFile == nullptr)
    {
        return nullptr;
    }

    return zipFile.release();
}

int read(void* context, char* buffer, int len)
{
    auto* zipFile = static_cast<zip::File*>(context);
    assert(zipFile);

    return static_cast<int>(zipFile->read(buffer, len));
}

int close(void* context)
{
    std::unique_ptr<zip::File> zipFile(static_cast<zip::File*>(context));
    assert(zipFile);

    return 0;
}
}; // namespace XmlSecIO

/// Performs libxmlsec init/deinit.
class XmlSecGuard
{
  public:
    explicit XmlSecGuard(zip::Archive* zipArchive, Crypto& crypto)
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
    }

    ~XmlSecGuard()
    {
        if (!_good)
        {
            return;
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

    [[nodiscard]] bool isGood() const { return _good; }

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

    [[nodiscard]] const std::string& getErrorString() const override;

    bool verify() override;

    bool verifyXAdES() override;

    [[nodiscard]] std::string getSubjectName() const override;

    [[nodiscard]] std::string getDate() const override;

    [[nodiscard]] std::string getMethod() const override;

    [[nodiscard]] std::string getType() const override;

    [[nodiscard]] std::set<std::string> getSignedStreams() const override;

  private:
    static std::string getObjectDate(xmlNode* objectNode);

    static std::string
    getSignaturePropertiesDate(xmlNode* signaturePropertiesNode);

    static std::string getSignaturePropertyDate(xmlNode* signaturePropertyNode);

    static std::string getDateContent(xmlNode* dateNode);

    static xmlNodePtr getObjectCertDigestNode(xmlNode* objectNode);

    [[nodiscard]] xmlNodePtr getCertDigestNode() const;

    [[nodiscard]] xmlNodePtr getX509CertificateNode() const;

    bool getCertificateBinary(std::vector<xmlChar>& certificate) const;

    static bool getDigestValue(xmlNodePtr certDigest,
                               std::vector<xmlChar>& value);

    static std::unique_ptr<xmlChar> getDigestAlgo(xmlNodePtr certDigest);

    static bool hash(const std::vector<xmlChar>& input,
                     std::unique_ptr<xmlChar> algo,
                     std::vector<unsigned char>& out);

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

    if (!_crypto.initializeSignatureContext(dsigCtx.get()))
    {
        _errorString = "signature context crypto init failed";
        return false;
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
    xmlSecSize certSize;
    if (xmlSecBase64Decode_ex(
            certificateContent.get(), (xmlSecByte*)certificateContent.get(),
            xmlStrlen(certificateContent.get()), &certSize) < 0)
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
                                  std::vector<xmlChar>& value)
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
    xmlSecSize expectedDigestSize;
    if (xmlSecBase64Decode_ex(digestValueNodeContent.get(),
                              (xmlSecByte*)digestValueNodeContent.get(),
                              xmlStrlen(digestValueNodeContent.get()),
                              &expectedDigestSize) < 0)
    {
        return false;
    }

    std::copy(digestValueNodeContent.get(),
              digestValueNodeContent.get() + expectedDigestSize,
              std::back_inserter(value));
    return true;
}

std::unique_ptr<xmlChar> XmlSignature::getDigestAlgo(xmlNodePtr certDigest)
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

bool XmlSignature::hash(const std::vector<xmlChar>& input,
                        std::unique_ptr<xmlChar> algo,
                        std::vector<unsigned char>& out)
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
    if (xmlSecTransformCtxBinaryExecute(transform.get(), input.data(),
                                        static_cast<xmlSecSize>(input.size())) <
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
        return {};
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
        return {};
    }

    std::unique_ptr<xmlChar> href(
        xmlGetProp(signatureMethodNode, xmlSecAttrAlgorithm));
    if (!href)
    {
        return {};
    }

    xmlSecTransformId transformId =
        xmlSecTransformIdListFindByHref(xmlSecTransformIdsGet(), href.get(),
                                        xmlSecTransformUsageSignatureMethod);
    if (transformId == xmlSecTransformIdUnknown)
    {
        return fromXmlChar(href.get());
    }

    return fromXmlChar(xmlSecTransformKlassGetName(transformId));
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
        if (uri.starts_with("#"))
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
        return "XAdES";
    }

    return "XML-DSig";
}

xmlNodePtr XmlSignature::getObjectCertDigestNode(xmlNode* objectNode)
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

    return {};
}

std::string XmlSignature::getObjectDate(xmlNode* objectNode)
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

    return {};
}

std::string XmlSignature::getDateContent(xmlNode* dateNode)
{
    std::unique_ptr<xmlChar> content(xmlNodeGetContent(dateNode));
    if (!content)
    {
        return {};
    }

    return fromXmlChar(content.get());
}

std::string
XmlSignature::getSignaturePropertiesDate(xmlNode* signaturePropertiesNode)
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

    return {};
}

std::string
XmlSignature::getSignaturePropertyDate(xmlNode* signaturePropertyNode)
{
    xmlNode* dateNode =
        xmlSecFindChild(signaturePropertyNode, dateNodeName, dateNsName);
    if (dateNode == nullptr)
    {
        return {};
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

    [[nodiscard]] const std::string& getErrorString() const override;

    void setTrustedDers(const std::vector<std::string>& trustedDers) override;

    void setInsecure(bool insecure) override;

    bool parseSignatures() override;

    std::vector<std::unique_ptr<Signature>>& getSignatures() override;

    [[nodiscard]] std::set<std::string> getStreams() const override;

  private:
    bool locateSignatures();

    std::vector<char> _zipContents;

    std::unique_ptr<zip::Source> _zipSource;

    std::unique_ptr<zip::Archive> _zipArchive;

    std::string _errorString;

    int64_t _signaturesZipIndex = 0;

    std::unique_ptr<XmlGuard> _xmlGuard;

    std::unique_ptr<Crypto> _crypto;

    std::unique_ptr<XmlSecGuard> _xmlSecGuard;

    std::unique_ptr<zip::File> _zipFile;

    std::vector<char> _signaturesBytes;

    std::unique_ptr<xmlDoc> _signaturesDoc;

    std::vector<std::unique_ptr<Signature>> _signatures;

    std::string _cryptoConfig;

    std::vector<std::string> _trustedDers;

    bool _insecure = false;
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

    try
    {
        _zipContents =
            std::vector<char>((std::istreambuf_iterator<char>(stream)),
                              std::istreambuf_iterator<char>());
        return openZipMemory(_zipContents.data(), _zipContents.size());
    }
    catch (std::ios_base::failure& failure)
    {
        _errorString = failure.what();
        return false;
    }
}

bool ZipVerifier::openZipMemory(const void* data, size_t size)
{
    std::unique_ptr<zip::Error> zipError = zip::Error::create();
    _zipSource = zip::Source::create(data, size, zipError.get());
    if (!_zipSource)
    {
        _errorString = zipError->getString();
        return false;
    }

    _zipArchive = zip::Archive::create(_zipSource.get(), zipError.get());
    if (!_zipArchive)
    {
        _errorString = zipError->getString();
        return false;
    }

    return true;
}

const std::string& ZipVerifier::getErrorString() const { return _errorString; }

void ZipVerifier::setTrustedDers(const std::vector<std::string>& trustedDers)
{
    _trustedDers = trustedDers;
}

void ZipVerifier::setInsecure(bool insecure) { _insecure = insecure; }

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

    _xmlSecGuard = std::make_unique<XmlSecGuard>(_zipArchive.get(), *_crypto);
    if (!_xmlSecGuard->isGood())
    {
        _errorString = "Failed to initialize libxmlsec";
        return false;
    }

    _zipFile = zip::File::create(_zipArchive.get(), _signaturesZipIndex);
    if (!_zipFile)
    {
        std::stringstream stream;
        stream << "Can't open file at index " << _signaturesZipIndex << ":"
               << _zipArchive->getErrorString();
        _errorString = stream.str();
        return false;
    }

    const int bufferSize = 8192;
    std::vector<char> readBuffer(bufferSize);
    int64_t readSize;
    while ((readSize = _zipFile->read(readBuffer.data(), readBuffer.size())) >
           0)
    {
        _signaturesBytes.insert(_signaturesBytes.end(), readBuffer.begin(),
                                readBuffer.begin() + readSize);
    }
    if (readSize == -1)
    {
        std::stringstream stream;
        stream << "Can't read file at index " << _signaturesZipIndex << ": "
               << _zipFile->getErrorString();
        _errorString = stream.str();
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
    int64_t numEntries = _zipArchive->getNumEntries();
    if (numEntries < 0)
    {
        return streams;
    }

    for (int64_t entry = 0; entry < numEntries; ++entry)
    {
        std::string name = _zipArchive->getName(entry);
        if (ends_with(name, "/"))
        {
            continue;
        }

        if (name == signaturesStreamName)
        {
            continue;
        }

        streams.insert(name);
    }
    return streams;
}

bool ZipVerifier::locateSignatures()
{
    _signaturesZipIndex = _zipArchive->locateName(signaturesStreamName);

    return _signaturesZipIndex >= 0;
}
} // namespace odfsig

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
