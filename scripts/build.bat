REM Copyright 2018 Miklos Vajna. All rights reserved.
REM Use of this source code is governed by a BSD-style license that can be
REM found in the LICENSE file.

cmake -E remove_directory workdir
md workdir
cd workdir
cmake -DODFSIG_INTERNAL_LIBS=ON ..
cmake --build . --config Release
REM disable due to the use-after-free bug in xmlsec 1.2.26's mscng backend
REM cmake --build . --config Release --target check
