#!/bin/bash -ex
# Copyright 2018 Miklos Vajna. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Baseline: Ubuntu Trusty 14.04.
CWD=$PWD

# Fix up libgtest-dev.
cd /usr/src/gtest
sudo cmake CMakeLists.txt
sudo make
sudo cp *.a /usr/lib
cd -

# xmlsec1 is too old to disable cert verification properly.
xmlsec_ver=1.2.26
wget http://www.aleksey.com/xmlsec/download/xmlsec1-$xmlsec_ver.tar.gz
tar xvf xmlsec1-$xmlsec_ver.tar.gz
cd xmlsec1-$xmlsec_ver
./configure --prefix=/usr --without-gnutls --without-gcrypt --without-openssl --disable-apps --disable-docs --disable-crypto-dl
make -j$(getconf _NPROCESSORS_ONLN)
sudo make install
cd -

# Setup a dummy Firefox profile.
mkdir -p ~/.mozilla/firefox
cd ~/.mozilla/firefox
echo -e "[Profile0]\nPath=odfsig.default\nDefault=1" > profiles.ini
mkdir odfsig.default
cd odfsig.default
echo odfsig > pwdfile.txt
certutil -N -f pwdfile.txt -d .
certutil -A -n "odfsig ca" -t "C,C,C" -i $CWD/core/qa/keys/ca-chain.cert.pem -d .
cd $CWD

mkdir workdir
cd workdir
CC=gcc-7 CXX=g++-7 cmake -DCMAKE_BUILD_TYPE=Debug -DODFSIG_ENABLE_WERROR=ON -DODFSIG_INTERNAL_LIBZIP=ON ..
make -j$(getconf _NPROCESSORS_ONLN)
make check

# vim:set shiftwidth=4 softtabstop=4 expandtab:
