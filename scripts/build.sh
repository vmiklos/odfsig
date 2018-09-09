#!/bin/bash -ex
# Copyright 2018 Miklos Vajna. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

rm -rf workdir
mkdir workdir
cd workdir
cmake "$@" ..
make -j$(getconf _NPROCESSORS_ONLN)
make check

# vim:set shiftwidth=4 softtabstop=4 expandtab:
