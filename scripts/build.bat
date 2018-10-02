REM Copyright 2018 Miklos Vajna. All rights reserved.
REM Use of this source code is governed by a BSD-style license that can be
REM found in the LICENSE file.

cmake -E remove_directory workdir
md workdir
cd workdir
cmake ..
cmake --build . --config Debug
REM disable due to the use-after-free bug in xmlsec 1.2.26's mscng backend
REM cmake --build . --config Release --target check
