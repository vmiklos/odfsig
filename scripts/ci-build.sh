#!/bin/bash -ex
# Copyright 2018 Miklos Vajna
#
# SPDX-License-Identifier: MIT

# Baseline: see .github/workflows/tests.yml.

if [ -n "${GITHUB_JOB}" -a "$(uname -s)" == "Linux" ]; then
    sudo apt-get update
    sudo apt-get install \
        clang-tidy \
        gyp \
        iwyu libclang-common-11-dev \
        ninja-build \

fi

if [ "$GITHUB_JOB" == "linux-gcc-release" ]; then
    CI_ARGS="--werror"
elif [ "$GITHUB_JOB" == "linux-clang-debug" ]; then
    CI_ARGS="--debug --werror --clang"
elif [ "$GITHUB_JOB" == "linux-clang-asan-ubsan" ]; then
    CI_ARGS="--debug --werror --asan-ubsan"
elif [ "$GITHUB_JOB" == "linux-clang-tidy" ]; then
    CI_ARGS="--debug --werror --tidy"
elif [ "$GITHUB_JOB" == "linux-gcc-iwyu" ]; then
    CI_ARGS="--debug --werror --iwyu"
fi

scripts/build.sh "$@" $CI_ARGS --internal-libs

if [ "$GITHUB_JOB" == "linux-gcc-release" ]; then
    cd workdir
    make pack
fi

# vim:set shiftwidth=4 softtabstop=4 expandtab:
