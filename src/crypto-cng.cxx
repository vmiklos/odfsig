/*
 * Copyright 2018 Miklos Vajna. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <odfsig/crypto.hxx>

#include <xmlsec/keysdata.h>
#include <xmlsec/mscng/app.h>
#include <xmlsec/mscng/crypto.h>
#include <xmlsec/xmlsec.h>

#include <odfsig/string.hxx>

namespace odfsig
{
class CngCrypto : public Crypto
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

bool CngCrypto::initialize(const std::string& cryptoConfig)
{
    return xmlSecMSCngAppInit(cryptoConfig.c_str()) >= 0;
}

bool CngCrypto::xmlSecInitialize() { return xmlSecMSCngInit() >= 0; }

bool CngCrypto::xmlSecShutdown() { return xmlSecMSCngShutdown() >= 0; }

bool CngCrypto::shutdown() { return xmlSecMSCngAppShutdown() >= 0; }

bool CngCrypto::initializeKeysManager(xmlSecKeysMngr* keysManager,
                                      std::vector<std::string> trustedDers)
{
    if (xmlSecMSCngAppDefaultKeysMngrInit(keysManager) < 0)
        return false;

    for (const auto& trustedDer : trustedDers)
    {
        if (xmlSecMSCngAppKeysMngrCertLoad(keysManager, trustedDer.c_str(),
                                           xmlSecKeyDataFormatDer,
                                           xmlSecKeyDataTypeTrusted) < 0)
            return false;
    }

    return true;
}

std::string CngCrypto::getCertificateSubjectName(unsigned char* certificate,
                                                 int size)
{
    // TODO
    return std::string();
}

std::unique_ptr<Crypto> Crypto::create()
{
    return std::unique_ptr<Crypto>(new CngCrypto());
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
