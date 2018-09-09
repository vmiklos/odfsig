#!/bin/bash -ex
# Copyright 2018 Miklos Vajna. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

cmake_args=""

for arg in "$@"
do
    case "$arg" in
        --debug)
            cmake_args+=" -DCMAKE_BUILD_TYPE=Debug"
            ;;
        --werror)
            cmake_args+=" -DODFSIG_ENABLE_WERROR=ON"
            ;;
        --internal-libs)
            cmake_args+=" -DODFSIG_INTERNAL_LIBZIP=ON"
            cmake_args+=" -DODFSIG_INTERNAL_LIBGTEST=ON"
            cmake_args+=" -DODFSIG_INTERNAL_XMLSEC=ON"
            ;;
        *)
            cmake_args+=" $arg"
    esac
done

rm -rf workdir
mkdir workdir
cd workdir
cmake $cmake_args ..
make -j$(getconf _NPROCESSORS_ONLN)
make check

# vim:set shiftwidth=4 softtabstop=4 expandtab:
