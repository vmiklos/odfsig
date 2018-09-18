/*
 * Copyright 2018 Miklos Vajna. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <odfsig/string.hxx>

namespace odfsig
{
bool starts_with(const std::string& big, const std::string& prefix)
{
    return big.compare(0, prefix.length(), prefix) == 0;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
