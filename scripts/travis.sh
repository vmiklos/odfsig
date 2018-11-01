#!/bin/bash -ex
# Copyright 2018 Miklos Vajna. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Baseline: Ubuntu Trusty 14.04 and macOS 10.12.

if [ "$TRAVIS_OS_NAME" = "linux" ]; then
    export CC=gcc-7
    export CXX=g++-7
fi

scripts/build.sh "$@"

# vim:set shiftwidth=4 softtabstop=4 expandtab:
