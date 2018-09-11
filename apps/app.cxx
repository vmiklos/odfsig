/*
 * Copyright 2018 Miklos Vajna. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <iostream>
#include <vector>

#include <odfsig/lib.hxx>

int main(int argc, char* argv[])
{
    std::vector<const char*> args(argv, argv + argc);
    return odfsig::main(args.size(), args.data(), std::cerr);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
