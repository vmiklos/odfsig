/*
 * Copyright 2018 Miklos Vajna
 *
 * SPDX-License-Identifier: MIT
 */

#include <odfsig/string.hxx>

#include <cstddef>

namespace odfsig
{
void replace_all(std::string& str, const std::string& from,
                 const std::string& replacement)
{
    size_t index = 0;

    while (true)
    {
        index = str.find(from, index);
        if (index == std::string::npos)
        {
            break;
        }

        str.replace(index, from.size(), replacement);

        index += replacement.size();
    }
}
} // namespace odfsig

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
