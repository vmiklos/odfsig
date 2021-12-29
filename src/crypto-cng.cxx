/*
 * Copyright 2018 Miklos Vajna. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <odfsig/crypto.hxx>

#include <codecvt>

#include <xmlsec/keysdata.h>
#include <xmlsec/mscng/app.h>
#include <xmlsec/mscng/crypto.h>
#include <xmlsec/xmlsec.h>

#include <odfsig/string.hxx>

namespace std
{
template <> struct default_delete<const CERT_CONTEXT>
{
    void operator()(const CERT_CONTEXT* ptr)
    {
        CertFreeCertificateContext(ptr);
    }
};
} // namespace std

namespace odfsig
{
/**
 * Crypto implementation using Microsoft Cryptography API: Next Generation
 * (CNG).
 */
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
                                          size_t size) override;
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
    {
        return false;
    }

    for (const auto& trustedDer : trustedDers)
    {
        if (xmlSecMSCngAppKeysMngrCertLoad(keysManager, trustedDer.c_str(),
                                           xmlSecKeyDataFormatDer,
                                           xmlSecKeyDataTypeTrusted) < 0)
        {
            return false;
        }
    }

    return true;
}

std::string CngCrypto::getCertificateSubjectName(unsigned char* certificate,
                                                 size_t size)
{
    std::unique_ptr<const CERT_CONTEXT> context(CertCreateCertificateContext(
        X509_ASN_ENCODING, certificate, static_cast<DWORD>(size)));
    if (context->pCertInfo == nullptr)
    {
        return std::string();
    }

    DWORD subjectSize = CertNameToStrW(
        X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, &context->pCertInfo->Subject,
        CERT_X500_NAME_STR | CERT_NAME_STR_REVERSE_FLAG, nullptr, 0);
    if (subjectSize <= 0)
    {
        return std::string();
    }

    std::vector<wchar_t> subject(subjectSize);
    subjectSize = CertNameToStrW(
        X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, &context->pCertInfo->Subject,
        CERT_X500_NAME_STR | CERT_NAME_STR_REVERSE_FLAG, subject.data(),
        static_cast<DWORD>(subject.size()));
    if (subjectSize <= 0)
    {
        return std::string();
    }

    std::wstring_convert<std::codecvt_utf8<wchar_t>> convert;
    return convert.to_bytes(subject.data());
}

std::unique_ptr<Crypto> Crypto::create()
{
    return std::unique_ptr<Crypto>(new CngCrypto());
}
} // namespace odfsig

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
