#ifndef _HAD_ODFSIG_STRING_H
#define _HAD_ODFSIG_STRING_H
/*
 * Copyright 2018 Miklos Vajna. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <string>

namespace odfsig
{
/// Checks if `big` begins with `prefix`.
bool starts_with(const std::string& big, const std::string& prefix);

/// Replaces `from` with `to` in `str`.
void replace_all(std::string& str, const std::string& from,
                 const std::string& to);
}
#endif /* _HAD_ODFSIG_STRING_H */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
