#pragma once
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

/// Replaces `from` with `replacement` in `str`.
void replace_all(std::string& str, const std::string& from,
                 const std::string& replacement);
} // namespace odfsig

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
