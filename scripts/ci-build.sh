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

if [ -n "${GITHUB_JOB}" -a "$(uname -s)" == "Darwin" ]; then
    brew install ninja
    PYVERSION=$(python3 -c 'import platform; print(".".join(platform.python_version_tuple()[0:2]))')
    export PATH=/Library/Frameworks/Python.framework/Versions/$PYVERSION/bin:$PATH
    python3 -m pip install packaging
    python3 -m pip install 'gyp-next @ git+https://github.com/nodejs/gyp-next@v0.16.1'
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
elif [ "$GITHUB_JOB" == "macos-release" ]; then
    CI_ARGS="--werror"
elif [ "$GITHUB_JOB" == "macos-debug" ]; then
    CI_ARGS="--debug --werror"
fi

scripts/build.sh "$@" $CI_ARGS --internal-libs

if [ "$GITHUB_JOB" == "linux-gcc-release" -o "$GITHUB_JOB" == "macos-release" ]; then
    cd workdir
    make pack
fi

# vim:set shiftwidth=4 softtabstop=4 expandtab:
