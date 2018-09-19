REM Copyright 2018 Miklos Vajna. All rights reserved.
REM Use of this source code is governed by a BSD-style license that can be
REM found in the LICENSE file.

cmake -E remove_directory workdir
md workdir
cd workdir
cmake -DCMAKE_BUILD_TYPE=Debug -DODFSIG_INTERNAL_LIBZIP=ON -DODFSIG_INTERNAL_LIBGTEST=ON -DODFSIG_INTERNAL_LIBXML2=ON -DODFSIG_INTERNAL_XMLSEC=ON -DODFSIG_INTERNAL_ZLIB=ON ..
cmake --build .
