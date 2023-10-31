/*
 * Copyright 2018 Miklos Vajna
 *
 * SPDX-License-Identifier: MIT
 */

#include <odfsig/crypto.hxx>

#include <algorithm>
#include <cstddef>
#include <fstream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include <cert.h>
#include <certt.h>
#include <libxml/xmlstring.h>
#include <prtypes.h>
#include <seccomon.h>
#include <xmlsec/keyinfo.h>
#include <xmlsec/keysdata.h>
#include <xmlsec/list.h>
#include <xmlsec/nss/app.h>
#include <xmlsec/nss/crypto.h>
#include <xmlsec/nss/x509.h>
#include <xmlsec/xmldsig.h>
#include <xmlsec/xmlsec.h>

namespace std
{
template <> struct default_delete<CERTCertificate>
{
    void operator()(CERTCertificate* ptr) { CERT_DestroyCertificate(ptr); }
};
} // namespace std

namespace
{
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
    std::stringstream stream;
    stream << cryptoConfig;
    stream << "/.mozilla/firefox/";
    const std::string firefoxPath = stream.str();

    std::ifstream profilesIni(firefoxPath + "profiles.ini");
    if (!profilesIni.good())
    {
        return {};
    }

    std::string profilePath;
    const std::string pathPrefix("Path=");
    std::string line;
    while (std::getline(profilesIni, line))
    {
        if (line.starts_with(pathPrefix))
        {
            // Path= is expected to be before Default=.
            profilePath = line.substr(pathPrefix.size());
        }
        else if (line == "Default=1")
        {
            return firefoxPath + profilePath;
        }
    }

    return {};
}
} // namespace

namespace odfsig
{
/// Crypto implementation using Network Security Services (NSS).
class NssCrypto : public Crypto
{
  public:
    bool initialize(const std::string& cryptoConfig) override;

    bool xmlSecInitialize() override;

    bool xmlSecShutdown() override;

    bool shutdown() override;

    bool initializeKeysManager(xmlSecKeysMngr* keysManager,
                               std::vector<std::string> trustedDers) override;

    bool initializeSignatureContext(_xmlSecDSigCtx* signatureContext) override;

    std::string getCertificateSubjectName(unsigned char* certificate,
                                          size_t size) override;
};

bool NssCrypto::initialize(const std::string& cryptoConfig)
{
    std::string firefoxProfile = getFirefoxProfile(cryptoConfig);
    const char* nssDb = nullptr;
    if (!firefoxProfile.empty())
    {
        nssDb = firefoxProfile.c_str();
    }

    return xmlSecNssAppInit(nssDb) >= 0;
}

bool NssCrypto::xmlSecInitialize() { return xmlSecNssInit() >= 0; }

bool NssCrypto::xmlSecShutdown() { return xmlSecNssShutdown() >= 0; }

bool NssCrypto::shutdown() { return xmlSecNssAppShutdown() >= 0; }

bool NssCrypto::initializeKeysManager(xmlSecKeysMngr* keysManager,
                                      std::vector<std::string> trustedDers)
{
    if (xmlSecNssAppDefaultKeysMngrInit(keysManager) < 0)
    {
        return false;
    }

    return std::all_of(trustedDers.begin(), trustedDers.end(),
                       [keysManager](const std::string& trustedDer)
                       {
                           return xmlSecNssAppKeysMngrCertLoad(
                                      keysManager, trustedDer.c_str(),
                                      xmlSecKeyDataFormatDer,
                                      xmlSecKeyDataTypeTrusted) >= 0;
                       });

    return true;
}

bool NssCrypto::initializeSignatureContext(
    struct _xmlSecDSigCtx* signatureContext)
{
    return xmlSecPtrListAdd(&(signatureContext->keyInfoReadCtx.enabledKeyData),
                            BAD_CAST xmlSecNssKeyDataX509GetKlass()) >= 0;
}

std::string NssCrypto::getCertificateSubjectName(unsigned char* certificate,
                                                 size_t size)
{
    SECItem certItem;
    certItem.data = certificate;
    certItem.len = size;

    std::unique_ptr<CERTCertificate> cert(CERT_NewTempCertificate(
        CERT_GetDefaultCertDB(), &certItem, nullptr, PR_FALSE, PR_TRUE));
    if (!cert || (cert->subjectName == nullptr))
    {
        return {};
    }

    return cert->subjectName;
}

std::unique_ptr<Crypto> Crypto::create()
{
    return std::unique_ptr<Crypto>(new NssCrypto());
}
} // namespace odfsig

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
