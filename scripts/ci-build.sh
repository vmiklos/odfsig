#!/bin/bash -ex
# Copyright 2018 Miklos Vajna. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Baseline: Ubuntu 18.04 and macOS 10.13.

scripts/build.sh "$@" --internal-libs

# TODO
#if [ "$TRAVIS_DEPLOY" = "y" ]; then
#    cd workdir
#    make pack
#fi

# vim:set shiftwidth=4 softtabstop=4 expandtab:
