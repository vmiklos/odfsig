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
    python3 -m pip install gyp-next
    PYVERSION=$(python3 -c 'import platform; print(".".join(platform.python_version_tuple()[0:2]))')
    # Patch in https://github.com/nodejs/gyp-next/pull/83 for now.
    sed 's/if sys.platform == "zos":/if PY3 or sys.platform == "zos":/' \
        /Library/Frameworks/Python.framework/Versions/$PYVERSION/lib/python$PYVERSION/site-packages/gyp/input.py \
        > /Library/Frameworks/Python.framework/Versions/$PYVERSION/lib/python$PYVERSION/site-packages/gyp/input.py.new
    mv /Library/Frameworks/Python.framework/Versions/$PYVERSION/lib/python$PYVERSION/site-packages/gyp/input.py.new \
        /Library/Frameworks/Python.framework/Versions/$PYVERSION/lib/python$PYVERSION/site-packages/gyp/input.py
    export PATH=$PATH:/Library/Frameworks/Python.framework/Versions/$PYVERSION/bin
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
