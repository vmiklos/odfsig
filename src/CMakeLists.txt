# Copyright 2018 Miklos Vajna. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

add_library(odfsigcore
    crypto.cxx
    lib.cxx
    main.cxx
    string.cxx
    )
target_include_directories(odfsigcore
    PUBLIC
    "${CMAKE_SOURCE_DIR}/include"
    "${CMAKE_BINARY_DIR}/include"
    )
target_link_libraries(odfsigcore
    libzip
    libxmlsec
    )

# vim:set shiftwidth=4 softtabstop=4 expandtab:
