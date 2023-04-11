#!/bin/bash -ex
# Copyright 2018 Miklos Vajna
#
# SPDX-License-Identifier: MIT

rm -rf workdir
mkdir workdir
cd workdir
cmake -DCMAKE_BUILD_TYPE=Debug -DODFSIG_ENABLE_WERROR=ON -DODFSIG_ENABLE_GCOV=ON ..
make -j$(getconf _NPROCESSORS_ONLN)
make check
cd -
lcov --directory workdir --capture --output-file odfsig.info
lcov --remove odfsig.info '/usr/*' --output-file odfsig.info
genhtml -o coverage odfsig.info
echo "Coverage report is now available at coverage/index.html."

# vim:set shiftwidth=4 softtabstop=4 expandtab:
