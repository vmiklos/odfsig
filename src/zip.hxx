#pragma once
/*
 * Copyright 2018 Miklos Vajna. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <cstddef>

#include <memory>
#include <string>

// FIXME get rid of this
struct zip_error;
// FIXME get rid of this
struct zip_source;

namespace odfsig
{
namespace zip
{

/// Represents a ZIP error.
class Error
{
  public:
    virtual ~Error() = default;

    virtual std::string getString() = 0;

    // FIXME get rid of this
    virtual zip_error* get() = 0;

    static std::unique_ptr<Error> create();
};

/// Represents a ZIP source, what will become an archive later.
class Source
{
  public:
    virtual ~Source() = default;

    // FIXME get rid of this
    virtual zip_source* get() = 0;

    static std::unique_ptr<Source> create(const void* data, size_t size,
                                          Error* error);
};
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
