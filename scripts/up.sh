#!/bin/bash -ex
# Copyright 2018 Miklos Vajna
#
# SPDX-License-Identifier: MIT

# Wrapper around build.sh, providing good settings for development.
#
# Symlink both this script and autogen.input to the root of the source tree before using it.

# Dependencies in a opensuse/leap:15.5 container (both system and internal libs config works after
# this):
# zypper in ccache
# zypper in cmake
# zypper in ctags
# zypper in gcc12-c++
# zypper in git
# zypper in gtest
# zypper in libtool
# zypper in libxml2-devel
# zypper in libzip-devel
# zypper in mozilla-nss-devel
# zypper in python-base
# zypper in xmlsec1-nss-devel
# zypper in zlib-devel

#git pull -r
export CC=gcc-12
export CXX=g++-12
scripts/build.sh $(sed 's/#.*//' autogen.input) "$@"

# Exclude workdir automatically.
ctags --c++-kinds=+p --fields=+iaS --extra=+q -R --totals=yes $(git ls-files|grep /|sed 's|/.*||'|sort -u)

# vim:set shiftwidth=4 softtabstop=4 expandtab:
