/*
 * Copyright 2018 Miklos Vajna. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "zip.hxx"

#include <zip.h>

namespace odfsig
{
namespace zip
{
/// Wrapper around libzip's zip_error_t.
class ZipError : public Error
{
  public:
    ZipError();

    ~ZipError() override;

    std::string getString() override;

    zip_error* get() override;

  private:
    zip_error_t _error;
};

ZipError::ZipError() { zip_error_init(&_error); }

ZipError::~ZipError() { zip_error_fini(&_error); }

std::string ZipError::getString() { return zip_error_strerror(&_error); }

zip_error* ZipError::get() { return &_error; }

std::unique_ptr<Error> Error::create() { return std::make_unique<ZipError>(); }

/// Wrapper around libzip's zip_source_t.
class ZipSource : public Source
{
  public:
    ZipSource(const void* data, size_t size, Error* error);

    ~ZipSource() override;

    zip_source* get() override;

  private:
    zip_source_t* _source;
};

ZipSource::ZipSource(const void* data, size_t size, Error* error)
    : _source(nullptr)
{
    auto zipError = dynamic_cast<zip::Error*>(error);
    if (zipError == nullptr)
    {
        return;
    }

    _source = zip_source_buffer_create(data, size, 0, error->get());
}

ZipSource::~ZipSource()
{
    if (_source != nullptr)
    {
        zip_source_free(_source);
    }
}

zip_source* ZipSource::get() { return _source; }

std::unique_ptr<Source> Source::create(const void* data, size_t size,
                                       Error* error)
{
    return std::make_unique<ZipSource>(data, size, error);
}

/// Wrapper around libzip's zip_t.
class ZipArchive : public Archive
{
  public:
    ZipArchive(Source* source, Error* error);

    ~ZipArchive() override;

    zip_t* get() override;

  private:
    zip_t* _archive;
};

ZipArchive::ZipArchive(Source* source, Error* error) : _archive(nullptr)
{
    auto zipSource = dynamic_cast<zip::Source*>(source);
    if (zipSource == nullptr)
    {
        return;
    }

    auto zipError = dynamic_cast<zip::Error*>(error);
    if (zipError == nullptr)
    {
        return;
    }

    const int openFlags = 0;
    _archive = zip_open_from_source(zipSource->get(), openFlags, error->get());
}

ZipArchive::~ZipArchive()
{
    if (_archive != nullptr)
    {
        zip_close(_archive);
    }
}

zip_t* ZipArchive::get() { return _archive; }

std::unique_ptr<Archive> Archive::create(Source* source, Error* error)
{
    auto archive = std::make_unique<ZipArchive>(source, error);
    if (archive->get() != nullptr)
    {
        return archive;
    }

    return nullptr;
}
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
