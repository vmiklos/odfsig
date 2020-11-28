#!/bin/bash -ex
# Copyright 2018 Miklos Vajna. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Baseline: Ubuntu 18.04 and macOS 10.13.

if [ "$GITHUB_JOB" == "linux-gcc-release" ]; then
    CI_ARGS="--werror"
fi

scripts/build.sh "$@" $CI_ARGS --internal-libs

# TODO
#if [ "$TRAVIS_DEPLOY" = "y" ]; then
#    cd workdir
#    make pack
#fi

# vim:set shiftwidth=4 softtabstop=4 expandtab:
