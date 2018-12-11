# Copyright 2018 Miklos Vajna. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

if (WIN32)
    set(CRYPTO cng)
    set(CRYPTO_LIBRARIES crypt32)
else ()
    set(CRYPTO nss)
    set(CRYPTO_LIBRARIES nss)
endif()

add_library(odfsigcore
    crypto-${CRYPTO}.cxx
    lib.cxx
    main.cxx
    string.cxx
    )
target_include_directories(odfsigcore
    PUBLIC
    "${CMAKE_SOURCE_DIR}/include"
    "${CMAKE_BINARY_DIR}/include"
    )
# In practice rpath is only needed for nss, everything else is linked
# statically.
if (ODFSIG_INTERNAL_NSS)
    set(ODFSIG_RPATH -Wl,-rpath,$ORIGIN)
endif ()
target_link_libraries(odfsigcore
    libzip
    libxmlsec
    libxml2
    ${CRYPTO_LIBRARIES}
    ${ODFSIG_RPATH}
    )

# vim:set shiftwidth=4 softtabstop=4 expandtab:
