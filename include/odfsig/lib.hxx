#ifndef _HAD_ODFSIG__LIB_H
#define _HAD_ODFSIG__LIB_H
/*
 * Copyright 2018 Miklos Vajna. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <memory>
#include <ostream>
#include <set>
#include <string>
#include <vector>

namespace odfsig
{
/// Represents one specific signature in the document.
class Signature
{
  public:
    virtual ~Signature() = default;

    virtual const std::string& getErrorString() const = 0;

    virtual bool verify() = 0;

    virtual bool verifyXAdES() = 0;

    virtual std::string getSubjectName() const = 0;

    virtual std::string getDate() const = 0;

    virtual std::string getMethod() const = 0;

    virtual std::string getType() const = 0;

    virtual std::set<std::string> getSignedStreams() const = 0;
};

/// Verifies signatures of an ODF document.
class Verifier
{
  public:
    virtual ~Verifier() = default;

    virtual bool openZip(const std::string& path) = 0;

    virtual const std::string& getErrorString() const = 0;

    /**
     * List of file paths representing DER CA chains to trust, useful when the
     * crypto config is empty.
     */
    virtual void
    setTrustedDers(const std::vector<std::string>& trustedDers) = 0;

    /// Sets if the certificate should be validated.
    virtual void setInsecure(bool insecure) = 0;

    virtual bool parseSignatures() = 0;

    virtual std::vector<std::unique_ptr<Signature>>& getSignatures() = 0;

    /**
     * Returns all streams in an ODF document (except the signature stream
     * itself).
     */
    virtual std::set<std::string> getStreams() const = 0;

    /**
     * cryptoConfig can be a path to a crypto DB, in which case no need to
     * trust DER CA chains manually.
     */
    static std::unique_ptr<Verifier> create(const std::string& cryptoConfig);
};

/// CLI wrapper around the C++ API.
int main(const std::vector<const char*>& args, std::ostream& ostream);
}
#endif /* _HAD_ODFSIG_LIB_H */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
