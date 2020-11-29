#!/bin/bash -ex
# Copyright 2018 Miklos Vajna. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Baseline: Ubuntu 18.04 and macOS 10.13.

if [ -n "${GITHUB_WORKFLOW}" -a "$(uname -s)" == "Linux" ]; then
    sudo apt-get install \
        clang-tidy \
        valgrind \

fi

if [ "$GITHUB_JOB" == "linux-gcc-release" ]; then
    CI_ARGS="--werror"
elif [ "$GITHUB_JOB" == "linux-clang-debug" ]; then
    CI_ARGS="--debug --werror --clang"
elif [ "$GITHUB_JOB" == "linux-clang-asan-ubsan" ]; then
    CI_ARGS="--debug --werror --asan-ubsan"
elif [ "$GITHUB_JOB" == "linux-valgrind" ]; then
    CI_ARGS="--debug --werror --valgrind"
elif [ "$GITHUB_JOB" == "linux-clang-tidy" ]; then
    CI_ARGS="--debug --werror --tidy"
fi

scripts/build.sh "$@" $CI_ARGS --internal-libs

# TODO
#if [ "$TRAVIS_DEPLOY" = "y" ]; then
#    cd workdir
#    make pack
#fi

# vim:set shiftwidth=4 softtabstop=4 expandtab:
