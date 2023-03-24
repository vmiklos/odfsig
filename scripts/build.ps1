# Copyright 2018 Miklos Vajna. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

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
