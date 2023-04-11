# Copyright 2018 Miklos Vajna
#
# SPDX-License-Identifier: MIT

param (
    [string]$config = "Release"
)

cmake -E remove_directory workdir
if (-not $?) { throw "error $?" }
md workdir
if (-not $?) { throw "error $?" }
cd workdir
if (-not $?) { throw "error $?" }
cmake -DODFSIG_ENABLE_WERROR=ON ..
if (-not $?) { throw "error $?" }
cmake --build . --config $config
if (-not $?) { throw "error $?" }
cmake --build . --config $config --target check
if (-not $?) { throw "error $?" }
