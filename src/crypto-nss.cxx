/*
 * Copyright 2018 Miklos Vajna. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <odfsig/crypto.hxx>

#include <fstream> // IWYU pragma: keep
#include <memory>
#include <sstream> // IWYU pragma: keep
#include <string>
#include <vector>

#include <cert.h>
#include <certt.h>
#include <prtypes.h>
#include <seccomon.h>
#include <xmlsec/keysdata.h>
#include <xmlsec/nss/app.h>
#include <xmlsec/nss/crypto.h>
#include <xmlsec/xmlsec.h>

#include <odfsig/string.hxx>

namespace std
{
template <> struct default_delete<CERTCertificate>
{
    void operator()(CERTCertificate* ptr) { CERT_DestroyCertificate(ptr); }
};
}

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
    std::stringstream ss;
    ss << cryptoConfig;
    ss << "/.mozilla/firefox/";
    const std::string firefoxPath = ss.str();

    std::ifstream profilesIni(firefoxPath + "profiles.ini");
    if (!profilesIni.good())
    {
        return std::string();
    }

    std::string profilePath;
    const std::string pathPrefix("Path=");
    std::string line;
    while (std::getline(profilesIni, line))
    {
        if (odfsig::starts_with(line, pathPrefix))
        {
            // Path= is expected to be before Default=.
            profilePath = line.substr(pathPrefix.size());
        }
        else if (line == "Default=1")
        {
            return firefoxPath + profilePath;
        }
    }

    return std::string();
}
}

namespace odfsig
{
class NssCrypto : public Crypto
{
  public:
    bool initialize(const std::string& cryptoConfig) override;

    bool xmlSecInitialize() override;

    bool xmlSecShutdown() override;

    bool shutdown() override;

    bool initializeKeysManager(xmlSecKeysMngr* keysManager,
                               std::vector<std::string> trustedDers) override;

    std::string getCertificateSubjectName(unsigned char* certificate,
                                          int size) override;
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

    for (const auto& trustedDer : trustedDers)
    {
        if (xmlSecNssAppKeysMngrCertLoad(keysManager, trustedDer.c_str(),
                                         xmlSecKeyDataFormatDer,
                                         xmlSecKeyDataTypeTrusted) < 0)
        {
            return false;
        }
    }

    return true;
}

std::string NssCrypto::getCertificateSubjectName(unsigned char* certificate,
                                                 int size)
{
    SECItem certItem;
    certItem.data = certificate;
    certItem.len = size;

    std::unique_ptr<CERTCertificate> cert(CERT_NewTempCertificate(
        CERT_GetDefaultCertDB(), &certItem, nullptr, PR_FALSE, PR_TRUE));
    if (!cert || !cert->subjectName)
    {
        return std::string();
    }

    return std::string(cert->subjectName);
}

std::unique_ptr<Crypto> Crypto::create()
{
    return std::unique_ptr<Crypto>(new NssCrypto());
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
