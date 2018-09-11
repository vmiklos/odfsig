#!/bin/bash -ex
# Copyright 2018 Miklos Vajna. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Baseline: Ubuntu Trusty 14.04.

CC=gcc-7 CXX=g++-7 scripts/build.sh "$@"

# vim:set shiftwidth=4 softtabstop=4 expandtab:
