#!/bin/bash -ex
# Copyright 2018 Miklos Vajna. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Baseline: Ubuntu Xenial 16.04 and macOS 10.13.

if [ "$TRAVIS_OS_NAME" = "linux" ]; then
    export CC=gcc-9
    export CXX=g++-9
fi

scripts/build.sh "$@"

if [ "$TRAVIS_DEPLOY" = "y" ]; then
    cd workdir
    make pack
fi

# vim:set shiftwidth=4 softtabstop=4 expandtab:
