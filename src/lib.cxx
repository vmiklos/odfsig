/*
 * Copyright 2018 Miklos Vajna. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <odfsig/lib.hxx>

#include <assert.h>

#include <fstream>
#include <iostream>
#include <sstream>

#include <cert.h>
#include <xmlsec/base64.h>
#include <xmlsec/io.h>
#include <xmlsec/nss/app.h>
#include <xmlsec/nss/crypto.h>
#include <xmlsec/xmldsig.h>
#include <xmlsec/xmltree.h>
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
template <> struct default_delete<xmlChar>
{
    void operator()(xmlChar* ptr) { xmlFree(ptr); }
};
template <> struct default_delete<CERTCertificate>
{
    void operator()(CERTCertificate* ptr) { CERT_DestroyCertificate(ptr); }
};
}

namespace
{
const xmlChar dateNodeName[] = "date";
const xmlChar dateNsName[] = "http://purl.org/dc/elements/1.1/";
const xmlChar xadesNsName[] = "http://uri.etsi.org/01903/v1.3.2#";
const char signaturesStreamName[] = "META-INF/documentsignatures.xml";

/// Checks if `big` begins with `small`.
bool starts_with(const std::string& big, const std::string& small)
{
    return big.compare(0, small.length(), small) == 0;
}

/// Checks if `big` ends with `small`.
bool ends_with(const std::string& big, const std::string& small)
{
    return big.compare(big.length() - small.length(), small.length(), small) ==
           0;
}

/// Converts from libxml char to normal char.
const char* fromXmlChar(const xmlChar* s)
{
    return reinterpret_cast<const char*>(s);
}

/**
 * Finds the default Firefox profile.
 *
 * Expected profiles.ini format is something like:
 *
 * [Profile0]
 * Path=...
 * Default=...
 */
std::string getFirefoxProfile(const std::string& cryptoConfig)
{
    std::stringstream ss;
    ss << cryptoConfig;
    ss << "/.mozilla/firefox/";
    const std::string firefoxPath = ss.str();

    std::ifstream profilesIni(firefoxPath + "profiles.ini");
    if (!profilesIni.good())
        return std::string();

    std::string profilePath;
    const std::string pathPrefix("Path=");
    std::string line;
    while (std::getline(profilesIni, line))
    {
        if (starts_with(line, pathPrefix))
            // Path= is expected to be before Default=.
            profilePath = line.substr(pathPrefix.size());
        else if (line == "Default=1")
            return firefoxPath + profilePath;
    }

    return std::string();
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
        return 0;

    return 1;
}

void* open(const char* uri)
{
    assert(zipArchive);

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
    explicit XmlSecGuard(zip_t* zipArchive, const std::string& cryptoConfig)
    {
        // Initialize nss.
        std::string firefoxProfile = getFirefoxProfile(cryptoConfig);
        const char* nssDb = nullptr;
        if (!firefoxProfile.empty())
            nssDb = firefoxProfile.c_str();
        _good = xmlSecNssAppInit(nssDb) >= 0;
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

    std::string getSubjectName() const override;

    std::string getDate() const override;

    std::string getMethod() const override;

    std::string getType() const override;

    virtual std::set<std::string> getSignedStreams() const override;

  private:
    std::string getObjectDate(xmlNode* objectNode) const;

    std::string
    getSignaturePropertiesDate(xmlNode* signaturePropertiesNode) const;

    std::string getSignaturePropertyDate(xmlNode* signaturePropertyNode) const;

    std::string getDateContent(xmlNode* dateNode) const;

    bool hasObjectCertDigest(xmlNode* objectNode) const;

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

    if (xmlSecDSigCtxVerify(dsigCtx.get(), _signatureNode) < 0)
    {
        _errorString = "DSig context verify failed";
        return false;
    }

    return dsigCtx->status == xmlSecDSigStatusSucceeded;
}

std::string XmlSignature::getSubjectName() const
{
    xmlNode* keyInfoNode =
        xmlSecFindChild(_signatureNode, xmlSecNodeKeyInfo, xmlSecDSigNs);
    xmlNode* x509Data =
        xmlSecFindChild(keyInfoNode, xmlSecNodeX509Data, xmlSecDSigNs);
    xmlNode* x509Certificate =
        xmlSecFindChild(x509Data, xmlSecNodeX509Certificate, xmlSecDSigNs);
    if (!x509Certificate)
        return std::string();

    std::unique_ptr<xmlChar> content(xmlNodeGetContent(x509Certificate));
    if (!content || xmlSecIsEmptyString(content.get()))
        return std::string();

    // Base64 decode in-place.
    int size = xmlSecBase64Decode(content.get(), (xmlSecByte*)content.get(),
                                  xmlStrlen(content.get()));
    if (size < 0)
        return std::string();

    SECItem certItem;
    certItem.data = content.get();
    certItem.len = size;
    std::unique_ptr<CERTCertificate> cert(CERT_NewTempCertificate(
        CERT_GetDefaultCertDB(), &certItem, NULL, PR_FALSE, PR_TRUE));
    if (!cert || !cert->subjectName)
        return std::string();

    return std::string(cert->subjectName);
}

std::string XmlSignature::getMethod() const
{
    xmlNode* signedInfoNode =
        xmlSecFindChild(_signatureNode, xmlSecNodeSignedInfo, xmlSecDSigNs);
    xmlNode* signatureMethodNode = xmlSecFindChild(
        signedInfoNode, xmlSecNodeSignatureMethod, xmlSecDSigNs);
    if (!signatureMethodNode)
        return std::string();

    std::unique_ptr<xmlChar> href(
        xmlGetProp(signatureMethodNode, xmlSecAttrAlgorithm));
    if (!href)
        return std::string();

    xmlSecTransformId id =
        xmlSecTransformIdListFindByHref(xmlSecTransformIdsGet(), href.get(),
                                        xmlSecTransformUsageSignatureMethod);
    if (id == xmlSecTransformIdUnknown)
        return fromXmlChar(href.get());

    return std::string(fromXmlChar(xmlSecTransformKlassGetName(id)));
}

std::set<std::string> XmlSignature::getSignedStreams() const
{
    xmlNode* signedInfoNode =
        xmlSecFindChild(_signatureNode, xmlSecNodeSignedInfo, xmlSecDSigNs);
    if (!signedInfoNode)
        return {};

    std::set<std::string> signedStreams;
    for (xmlNode* signedInfoChild = signedInfoNode->children; signedInfoChild;
         signedInfoChild = signedInfoChild->next)
    {
        if (!xmlSecCheckNodeName(signedInfoChild, xmlSecNodeReference,
                                 xmlSecDSigNs))
            continue;

        std::unique_ptr<xmlChar> uriProp(
            xmlGetProp(signedInfoChild, xmlSecAttrURI));
        if (!uriProp)
            continue;

        std::string uri(fromXmlChar(uriProp.get()));
        if (starts_with(uri, "#"))
            continue;

        signedStreams.insert(uri);
    }

    return signedStreams;
}

std::string XmlSignature::getType() const
{
    for (xmlNode* signatureChild = _signatureNode->children; signatureChild;
         signatureChild = signatureChild->next)
    {
        if (!xmlSecCheckNodeName(signatureChild, xmlSecNodeObject,
                                 xmlSecDSigNs))
            continue;

        bool hasCertDigest = hasObjectCertDigest(signatureChild);
        if (hasCertDigest)
            return std::string("XAdES");
    }

    return std::string("XML-DSig");
}

bool XmlSignature::hasObjectCertDigest(xmlNode* objectNode) const
{
    const xmlChar qualifyingPropertiesNodeName[] = "QualifyingProperties";
    const xmlChar signedPropertiesNodeName[] = "SignedProperties";
    const xmlChar signedSignaturePropertiesNodeName[] =
        "SignedSignatureProperties";
    const xmlChar signingCertificateNodeName[] = "SigningCertificate";
    const xmlChar certNodeName[] = "Cert";
    const xmlChar certDigestNodeName[] = "CertDigest";

    xmlNode* qualifyingPropertiesNode =
        xmlSecFindChild(objectNode, qualifyingPropertiesNodeName, xadesNsName);
    if (!qualifyingPropertiesNode)
        return false;

    xmlNode* signedPropertiesNode = xmlSecFindChild(
        qualifyingPropertiesNode, signedPropertiesNodeName, xadesNsName);
    if (!signedPropertiesNode)
        return false;

    xmlNode* signedSignaturePropertiesNode = xmlSecFindChild(
        signedPropertiesNode, signedSignaturePropertiesNodeName, xadesNsName);
    if (!signedSignaturePropertiesNode)
        return false;

    xmlNode* signingCertificateNode = xmlSecFindChild(
        signedSignaturePropertiesNode, signingCertificateNodeName, xadesNsName);
    if (!signingCertificateNode)
        return false;

    xmlNode* certNode =
        xmlSecFindChild(signingCertificateNode, certNodeName, xadesNsName);
    if (!certNode)
        return false;

    xmlNode* certDigestNode =
        xmlSecFindChild(certNode, certDigestNodeName, xadesNsName);
    if (!certDigestNode)
        return false;

    return true;
}

std::string XmlSignature::getDate() const
{
    for (xmlNode* signatureChild = _signatureNode->children; signatureChild;
         signatureChild = signatureChild->next)
    {
        if (!xmlSecCheckNodeName(signatureChild, xmlSecNodeObject,
                                 xmlSecDSigNs))
            continue;

        std::string date = getObjectDate(signatureChild);
        if (!date.empty())
            return date;
    }

    return std::string();
}

std::string XmlSignature::getObjectDate(xmlNode* objectNode) const
{
    for (xmlNode* objectChild = objectNode->children; objectChild;
         objectChild = objectChild->next)
    {
        if (!xmlSecCheckNodeName(objectChild, xmlSecNodeSignatureProperties,
                                 xmlSecDSigNs))
            continue;

        std::string date = getSignaturePropertiesDate(objectChild);
        if (!date.empty())
            return date;
    }

    return std::string();
}

std::string XmlSignature::getDateContent(xmlNode* dateNode) const
{
    std::unique_ptr<xmlChar> content(xmlNodeGetContent(dateNode));
    if (!content)
        return std::string();

    return std::string(fromXmlChar(content.get()));
}

std::string
XmlSignature::getSignaturePropertiesDate(xmlNode* signaturePropertiesNode) const
{
    const xmlChar signaturePropertyNodeName[] = "SignatureProperty";

    for (xmlNode* signaturePropertiesChild = signaturePropertiesNode->children;
         signaturePropertiesChild;
         signaturePropertiesChild = signaturePropertiesChild->next)
    {
        if (!xmlSecCheckNodeName(signaturePropertiesChild,
                                 signaturePropertyNodeName, xmlSecDSigNs))
            continue;

        std::string date = getSignaturePropertyDate(signaturePropertiesChild);
        if (!date.empty())
            return date;
    }

    return std::string();
}

std::string
XmlSignature::getSignaturePropertyDate(xmlNode* signaturePropertyNode) const
{
    xmlNode* dateNode =
        xmlSecFindChild(signaturePropertyNode, dateNodeName, dateNsName);
    if (!dateNode)
        return std::string();

    return getDateContent(dateNode);
}

/// Implementation of Verifier using libzip.
class ZipVerifier : public Verifier
{
  public:
    explicit ZipVerifier(const std::string& cryptoConfig);

    bool openZip(const std::string& path) override;

    const std::string& getErrorString() const override;

    bool parseSignatures() override;

    std::vector<std::unique_ptr<Signature>>& getSignatures() override;

    virtual std::set<std::string> getStreams() const override;

  private:
    bool locateSignatures();

    std::unique_ptr<zip_t> _zipArchive;
    std::string _errorString;
    zip_int64_t _signaturesZipIndex = 0;
    std::unique_ptr<XmlGuard> _xmlGuard;
    std::unique_ptr<XmlSecGuard> _xmlSecGuard;
    std::unique_ptr<zip_file_t> _zipFile;
    std::vector<char> _signaturesBytes;
    std::unique_ptr<xmlDoc> _signaturesDoc;
    std::vector<std::unique_ptr<Signature>> _signatures;
    std::string _cryptoConfig;
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
    const int openFlags = 0;
    int errorCode = 0;
    _zipArchive.reset(zip_open(path.c_str(), openFlags, &errorCode));
    if (!_zipArchive)
    {
        zip_error_t zipError;
        zip_error_init_with_code(&zipError, errorCode);
        _errorString = zip_error_strerror(&zipError);
        zip_error_fini(&zipError);
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

    _xmlSecGuard.reset(new XmlSecGuard(_zipArchive.get(), _cryptoConfig));
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

std::set<std::string> ZipVerifier::getStreams() const
{
    std::set<std::string> streams;
    zip_int64_t numEntries = zip_get_num_entries(_zipArchive.get(), 0);
    if (numEntries < 0)
        return streams;

    for (zip_int64_t entry = 0; entry < numEntries; ++entry)
    {
        const char* name = zip_get_name(_zipArchive.get(), entry, 0);
        if (!name)
            continue;

        std::string stream(name);
        if (ends_with(stream, "/"))
            continue;

        if (stream == signaturesStreamName)
            continue;

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
