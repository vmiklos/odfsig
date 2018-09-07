#ifndef _HAD_ODFSIG__LIB_H
#define _HAD_ODFSIG__LIB_H
/*
 * Copyright 2018 Miklos Vajna. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <memory>
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

    virtual bool parseSignatures() = 0;

    virtual std::vector<std::unique_ptr<Signature>>& getSignatures() = 0;

    /**
     * Returns all streams in an ODF document (except the signature stream
     * itself).
     */
    virtual std::set<std::string> getStreams() const = 0;

    static std::unique_ptr<Verifier> create();
};
}
#endif /* _HAD_ODFSIG_LIB_H */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
