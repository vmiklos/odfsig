#pragma once
/*
 * Copyright 2018 Miklos Vajna
 *
 * SPDX-License-Identifier: MIT
 */

#include <memory>
#include <ostream>
#include <set>
#include <string>
#include <vector>

#include <cstddef>

namespace odfsig
{
/// Represents one specific signature in the document.
class Signature
{
  public:
    virtual ~Signature() = default;

    [[nodiscard]] virtual const std::string& getErrorString() const = 0;

    virtual bool verify() = 0;

    virtual bool verifyXAdES() = 0;

    [[nodiscard]] virtual std::string getSubjectName() const = 0;

    [[nodiscard]] virtual std::string getDate() const = 0;

    [[nodiscard]] virtual std::string getMethod() const = 0;

    [[nodiscard]] virtual std::string getType() const = 0;

    [[nodiscard]] virtual std::set<std::string> getSignedStreams() const = 0;
};

/// Verifies signatures of an ODF document.
class Verifier
{
  public:
    virtual ~Verifier() = default;

    /// Opens a file, wrapper around openZipMemory().
    virtual bool openZip(const std::string& path) = 0;

    /// Opens in-memory data.
    virtual bool openZipMemory(const void* data, size_t size) = 0;

    [[nodiscard]] virtual const std::string& getErrorString() const = 0;

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
    [[nodiscard]] virtual std::set<std::string> getStreams() const = 0;

    /**
     * cryptoConfig can be a path to a crypto DB, in which case no need to
     * trust DER CA chains manually.
     */
    static std::unique_ptr<Verifier> create(const std::string& cryptoConfig);
};

/// CLI wrapper around the C++ API.
int main(const std::vector<const char*>& args, std::ostream& ostream);
} // namespace odfsig

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
