#!/bin/bash -ex
# Copyright 2018 Miklos Vajna. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Baseline: Ubuntu Trusty 14.04.
CWD=$PWD

# Setup a dummy Firefox profile.
mkdir -p ~/.mozilla/firefox
cd ~/.mozilla/firefox
echo -e "[Profile0]\nPath=odfsig.default\nDefault=1" > profiles.ini
mkdir odfsig.default
cd odfsig.default
echo odfsig > pwdfile.txt
certutil -N -f pwdfile.txt -d .
certutil -A -n "odfsig ca" -t "C,C,C" -i $CWD/tests/keys/ca-chain.cert.pem -d .
cd $CWD

mkdir workdir
cd workdir
CC=gcc-7 CXX=g++-7 cmake \
    -DCMAKE_BUILD_TYPE=Debug \
    -DODFSIG_ENABLE_WERROR=ON \
    -DODFSIG_INTERNAL_LIBZIP=ON \
    -DODFSIG_INTERNAL_LIBGTEST=ON \
    -DODFSIG_INTERNAL_XMLSEC=ON \
        ..
make -j$(getconf _NPROCESSORS_ONLN)
make check

# vim:set shiftwidth=4 softtabstop=4 expandtab:
