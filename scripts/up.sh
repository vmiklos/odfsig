#!/bin/bash -ex
# Copyright 2018 Miklos Vajna
#
# SPDX-License-Identifier: MIT

# Wrapper around build.sh, providing good settings for development.
#
# Symlink both this script and autogen.input to the root of the source tree before using it.

#git pull -r
export CC=gcc-12
export CXX=g++-12
scripts/build.sh $(sed 's/#.*//' autogen.input) "$@"

# Exclude workdir automatically.
ctags --c++-kinds=+p --fields=+iaS --extra=+q -R --totals=yes $(git ls-files|grep /|sed 's|/.*||'|sort -u)

# vim:set shiftwidth=4 softtabstop=4 expandtab:
