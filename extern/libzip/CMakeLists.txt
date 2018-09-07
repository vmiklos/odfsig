# Copyright 2018 Miklos Vajna. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

find_package(PkgConfig REQUIRED)
include(ExternalProject)
include(GNUInstallDirs)

if (NOT ODFSIG_INTERNAL_LIBZIP)
    pkg_check_modules(LIBZIP REQUIRED libzip)
else ()
    pkg_check_modules(ZLIB REQUIRED zlib)
    ExternalProject_Add(externalzip
        URL https://libzip.org/download/libzip-1.5.1.tar.gz
        SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/external/libzip-source
        BUILD_IN_SOURCE ON
        CONFIGURE_COMMAND cmake
            -DBUILD_SHARED_LIBS=OFF
            -DENABLE_GNUTLS=OFF
            -DENABLE_OPENSSL=OFF
            -DENABLE_COMMONCRYPTO=OFF
            -DCMAKE_INSTALL_PREFIX=${CMAKE_CURRENT_BINARY_DIR}/external/libzip-install
        BUILD_COMMAND make
        INSTALL_COMMAND make install
        )
    add_library(zip STATIC IMPORTED)
    set_property(TARGET zip PROPERTY IMPORTED_LOCATION ${CMAKE_CURRENT_BINARY_DIR}/external/libzip-install/${CMAKE_INSTALL_LIBDIR}/libzip.a)
    add_dependencies(zip externalzip)
    set(LIBZIP_INCLUDE_DIRS ${CMAKE_CURRENT_BINARY_DIR}/external/libzip-install/include ${ZLIB_INCLUDE_DIRS})
    # bz2 may not provide a pkg-config file, so just hardcode its name.
    set(LIBZIP_LIBRARIES zip ${ZLIB_LIBRARIES} bz2)
endif ()

add_library(libzip INTERFACE)
target_include_directories(libzip INTERFACE ${LIBZIP_INCLUDE_DIRS})
target_link_libraries(libzip INTERFACE ${LIBZIP_LIBRARIES})

# vim:set shiftwidth=4 softtabstop=4 expandtab:
