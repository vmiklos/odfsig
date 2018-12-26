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
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
