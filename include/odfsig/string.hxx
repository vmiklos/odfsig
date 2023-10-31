#pragma once
/*
 * Copyright 2018 Miklos Vajna
 *
 * SPDX-License-Identifier: MIT
 */

#include <string>

namespace odfsig
{
/// Replaces `from` with `replacement` in `str`.
void replace_all(std::string& str, const std::string& from,
                 const std::string& replacement);
} // namespace odfsig

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
