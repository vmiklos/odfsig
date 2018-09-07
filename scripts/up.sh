#!/bin/bash -ex
# Copyright 2018 Miklos Vajna. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

#git pull -r
rm -rf workdir
mkdir workdir
cd workdir
cmake -DCMAKE_BUILD_TYPE=Debug -DODFSIG_ENABLE_WERROR=ON "$@" ..
make -j$(getconf _NPROCESSORS_ONLN)
make check
# Exclude workdir automatically
cd ..
ctags --c++-kinds=+p --fields=+iaS --extra=+q -R --totals=yes $(git ls-files|grep /|sed 's|/.*||'|sort -u)

# vim:set shiftwidth=4 softtabstop=4 expandtab:
