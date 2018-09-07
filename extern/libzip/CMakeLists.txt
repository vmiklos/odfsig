# Copyright 2018 Miklos Vajna. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

find_package(PkgConfig REQUIRED)

if (NOT ODFSIG_INTERNAL_LIBZIP)
    pkg_check_modules(LIBZIP REQUIRED libzip)
else ()
    include(ExternalProject)
    include(GNUInstallDirs)
    pkg_check_modules(ZLIB REQUIRED zlib)
    ExternalProject_Add(extern-libzip-project
        URL https://libzip.org/download/libzip-1.5.1.tar.gz
        URL_HASH SHA256=47eaa45faa448c72bd6906e5a096846c469a185f293cafd8456abb165841b3f2
        DOWNLOAD_DIR ${CMAKE_SOURCE_DIR}/extern/tarballs
        BUILD_IN_SOURCE ON
        CONFIGURE_COMMAND ${CMAKE_COMMAND}
            -DBUILD_SHARED_LIBS=OFF
            -DENABLE_GNUTLS=OFF
            -DENABLE_OPENSSL=OFF
            -DENABLE_COMMONCRYPTO=OFF
            -DCMAKE_INSTALL_PREFIX=${CMAKE_CURRENT_BINARY_DIR}/install
        BUILD_COMMAND ${CMAKE_COMMAND} --build .
        INSTALL_COMMAND ${CMAKE_COMMAND} --build . --target install
        )
    add_library(zip STATIC IMPORTED)
    set_property(TARGET zip PROPERTY IMPORTED_LOCATION ${CMAKE_CURRENT_BINARY_DIR}/install/${CMAKE_INSTALL_LIBDIR}/libzip.a)
    add_dependencies(zip extern-libzip-project)
    set(LIBZIP_INCLUDE_DIRS ${CMAKE_CURRENT_BINARY_DIR}/install/include ${ZLIB_INCLUDE_DIRS})
    # bz2 may not provide a pkg-config file, so just hardcode its name.
    set(LIBZIP_LIBRARIES zip ${ZLIB_LIBRARIES} bz2)
endif ()

add_library(libzip INTERFACE)
target_include_directories(libzip INTERFACE ${LIBZIP_INCLUDE_DIRS})
target_link_libraries(libzip INTERFACE ${LIBZIP_LIBRARIES})

# vim:set shiftwidth=4 softtabstop=4 expandtab:
