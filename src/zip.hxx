#pragma once
/*
 * Copyright 2018 Miklos Vajna. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <memory>
#include <string>

// FIXME get rid of this
struct zip_error;

namespace odfsig
{
namespace zip
{

class Error
{
  public:
    virtual ~Error() = default;

    virtual std::string getString() = 0;

    // FIXME get rid of this
    virtual zip_error* get() = 0;

    static std::unique_ptr<Error> create();
};
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
