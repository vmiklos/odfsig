#!/bin/bash -ex
# Copyright 2018 Miklos Vajna
#
# SPDX-License-Identifier: MIT

#git pull -r
scripts/build.sh --debug --werror "$@"

# Exclude workdir automatically.
ctags --c++-kinds=+p --fields=+iaS --extra=+q -R --totals=yes $(git ls-files|grep /|sed 's|/.*||'|sort -u)

# vim:set shiftwidth=4 softtabstop=4 expandtab:
