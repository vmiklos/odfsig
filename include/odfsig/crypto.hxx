#pragma once
/*
 * Copyright 2018 Miklos Vajna. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <memory>
#include <string>
#include <vector>

struct _xmlSecKeysMngr;

namespace odfsig
{
/// Represents a crypto implementation.
class Crypto
{
  public:
    virtual ~Crypto() = default;

    /// Initializes the crypto itself.
    virtual bool initialize(const std::string& cryptoConfig) = 0;

    /// Initializes the crypto backend of xmlsec.
    virtual bool xmlSecInitialize() = 0;

    /// Shuts down the crypto backend of xmlsec.
    virtual bool xmlSecShutdown() = 0;

    /// Shuts down the crypto itself.
    virtual bool shutdown() = 0;

    /// Performs the crypto init of a keys manager.
    virtual bool
    initializeKeysManager(_xmlSecKeysMngr* keysManager,
                          std::vector<std::string> trustedDers) = 0;

    /// Extracts the subject name of an X509 certificate.
    virtual std::string getCertificateSubjectName(unsigned char* certificate,
                                                  int size) = 0;

    static std::unique_ptr<Crypto> create();
};
} // namespace odfsig

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
