#pragma once
/*
 * Copyright 2018 Miklos Vajna. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <cstddef>
#include <cstdint>

#include <memory>
#include <string>

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

    static std::unique_ptr<Error> create();
};

/// Represents a ZIP source, what will become an archive later.
class Source
{
  public:
    virtual ~Source() = default;

    static std::unique_ptr<Source> create(const void* data, size_t size,
                                          Error* error);
};

/// Represents a ZIP archive, which consumes a source.
class Archive
{
  public:
    virtual ~Archive() = default;

    virtual int64_t locateName(const char* name) = 0;

    virtual std::string getErrorString() = 0;

    virtual int64_t getNumEntries() = 0;

    virtual std::string getName(int64_t index) = 0;

    /// Factory for this interface. If returns nullptr, error is set.
    static std::unique_ptr<Archive> create(Source* source, Error* error);
};

/// Represents one stream in the ZIP archive.
class File
{
  public:
    virtual ~File() = default;

    virtual int64_t read(void* buffer, uint64_t length) = 0;

    virtual std::string getErrorString() = 0;

    /// Factory for this interface. Returns nullptr, on failure.
    static std::unique_ptr<File> create(Archive* archive, int64_t index);
};
} // namespace zip
} // namespace odfsig

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
