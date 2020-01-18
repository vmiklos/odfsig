/*
 * Copyright 2018 Miklos Vajna. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "zip.hxx"

#include <cassert>

#include <zip.h>

namespace odfsig::zip
{
/// Wrapper around libzip's zip_error_t.
class ZipError : public Error
{
  public:
    ZipError();

    ~ZipError() override;

    std::string getString() override;

    zip_error* get();

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

    zip_source_t* get();

  private:
    zip_source_t* _source;
};

ZipSource::ZipSource(const void* data, size_t size, Error* error)
    : _source(nullptr)
{
    auto* zipError = dynamic_cast<zip::ZipError*>(error);
    if (zipError == nullptr)
    {
        return;
    }

    _source = zip_source_buffer_create(data, size, 0, zipError->get());
}

ZipSource::~ZipSource()
{
    if (_source != nullptr)
    {
        zip_source_free(_source);
    }
}

zip_source_t* ZipSource::get() { return _source; }

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

    int64_t locateName(const char* name) override;

    std::string getErrorString() override;

    int64_t getNumEntries() override;

    std::string getName(int64_t index) override;

    zip_t* get();

  private:
    zip_t* _archive;
};

ZipArchive::ZipArchive(Source* source, Error* error) : _archive(nullptr)
{
    auto* zipSource = dynamic_cast<zip::ZipSource*>(source);
    if (zipSource == nullptr)
    {
        return;
    }

    auto* zipError = dynamic_cast<zip::ZipError*>(error);
    if (zipError == nullptr)
    {
        return;
    }

    const int openFlags = 0;
    _archive =
        zip_open_from_source(zipSource->get(), openFlags, zipError->get());
}

ZipArchive::~ZipArchive()
{
    if (_archive != nullptr)
    {
        zip_close(_archive);
    }
}

int64_t ZipArchive::locateName(const char* name)
{
    assert(_archive);

    return zip_name_locate(_archive, name, 0);
}

std::string ZipArchive::getErrorString()
{
    assert(_archive);

    return zip_strerror(_archive);
}

int64_t ZipArchive::getNumEntries()
{
    assert(_archive);

    return zip_get_num_entries(_archive, 0);
}

std::string ZipArchive::getName(int64_t index)
{
    assert(_archive);

    const char* name = zip_get_name(_archive, index, 0);
    if (name == nullptr)
    {
        return std::string();
    }

    return name;
}

zip_t* ZipArchive::get() { return _archive; }

std::unique_ptr<Archive> Archive::create(Source* source, Error* error)
{
    auto* zipSource = dynamic_cast<zip::ZipSource*>(source);
    auto archive = std::make_unique<ZipArchive>(source, error);
    if (archive->get() != nullptr)
    {
        zip_source_keep(zipSource->get());
        return archive;
    }

    return nullptr;
}

/// Wrapper around libzip's zip_file_t.
class ZipFile : public File
{
  public:
    ZipFile(Archive* archive, int64_t index);

    ~ZipFile() override;

    int64_t read(void* buffer, uint64_t length) override;

    std::string getErrorString() override;

    zip_file_t* get();

  private:
    zip_file_t* _file;
};

ZipFile::ZipFile(Archive* archive, int64_t index) : _file(nullptr)
{
    auto* zipArchive = dynamic_cast<zip::ZipArchive*>(archive);
    if (zipArchive == nullptr)
    {
        return;
    }

    _file = zip_fopen_index(zipArchive->get(), index, 0);
}

ZipFile::~ZipFile()
{
    if (_file != nullptr)
    {
        zip_fclose(_file);
    }
}

int64_t ZipFile::read(void* buffer, uint64_t length)
{
    assert(_file);

    return zip_fread(_file, buffer, length);
}

std::string ZipFile::getErrorString()
{
    assert(_file);

    return zip_file_strerror(_file);
}

zip_file_t* ZipFile::get() { return _file; }

std::unique_ptr<File> File::create(Archive* archive, int64_t index)
{
    auto file = std::make_unique<ZipFile>(archive, index);
    if (file->get() != nullptr)
    {
        return file;
    }

    return nullptr;
}
} // namespace odfsig::zip

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
