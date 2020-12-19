#!/bin/bash -ex
# Copyright 2018 Miklos Vajna. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Baseline: Ubuntu 20.04 and macOS 10.15.

if [ -n "${GITHUB_JOB}" -a "$(uname -s)" == "Linux" ]; then
    sudo apt-get update
    sudo apt-get install \
        clang-tidy \
        gyp \
        ninja-build \
        valgrind \

fi

if [ -n "${GITHUB_JOB}" -a "$(uname -s)" == "Darwin" ]; then
    git clone https://chromium.googlesource.com/external/gyp.git
    cd gyp
    python setup.py build
    # See
    # <https://stackoverflow.com/questions/4495120/combine-user-with-prefix-error-with-setup-py-install>
    # on why '--prefix=' is necessary.
    python setup.py install --user --prefix=
    export PATH=$PATH:$HOME/Library/Python/2.7/bin
    cd -
    brew install ninja
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
