/*
 * Copyright 2018 Miklos Vajna
 *
 * SPDX-License-Identifier: MIT
 */

#include <iostream>
#include <vector>

#include <odfsig/lib.hxx>

int main(int argc, char** argv)
{
    std::vector<const char*> args(argv, argv + argc);
    return odfsig::main(args, std::cerr);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
