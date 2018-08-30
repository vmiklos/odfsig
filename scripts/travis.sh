#!/bin/bash -ex
# Copyright 2018 Miklos Vajna. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Fix up libgtest-dev.
cd /usr/src/gtest
sudo cmake CMakeLists.txt
sudo make
sudo cp *.a /usr/lib
cd -

mkdir workdir
cd workdir
CC=gcc-7 CXX=g++-7 cmake -DCMAKE_BUILD_TYPE=Debug -DODFSIG_ENABLE_WERROR=ON ..
make -j$(getconf _NPROCESSORS_ONLN)
make check

# vim:set shiftwidth=4 softtabstop=4 expandtab:
