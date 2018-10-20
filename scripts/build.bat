REM Copyright 2018 Miklos Vajna. All rights reserved.
REM Use of this source code is governed by a BSD-style license that can be
REM found in the LICENSE file.

cmake -E remove_directory workdir
md workdir
cd workdir
cmake -DODFSIG_ENABLE_WERROR=ON ..
cmake --build . --config Debug
cmake --build . --config Debug --target check
