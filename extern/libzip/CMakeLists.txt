# Copyright 2019 Miklos Vajna. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

set(LIBZIP_VERSION 1.6.0)
set(LIBZIP_SHA256SUM 70c4bb351dec256fb727053a161faa14c6fe244988b12943800aafc5ccbf5aff)

if (NOT ODFSIG_INTERNAL_LIBZIP)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(LIBZIP REQUIRED libzip)
else ()
    include(ExternalProject)
    include(GNUInstallDirs)

    if (ODFSIG_INTERNAL_ZLIB)
        ExternalProject_Get_Property(extern-zlib-project INSTALL_DIR)
        set(ZLIB_ROOT -DZLIB_ROOT=${INSTALL_DIR})
    endif ()
    ExternalProject_Add(extern-libzip-project
        URL https://libzip.org/download/libzip-${LIBZIP_VERSION}.tar.gz
        URL_HASH SHA256=${LIBZIP_SHA256SUM}
        DOWNLOAD_DIR ${CMAKE_SOURCE_DIR}/extern/tarballs
        CONFIGURE_COMMAND ${CMAKE_COMMAND}
            -DBUILD_SHARED_LIBS=OFF
            -DENABLE_GNUTLS=OFF
            -DENABLE_OPENSSL=OFF
            -DENABLE_COMMONCRYPTO=OFF
            -DENABLE_WINDOWS_CRYPTO=OFF
            -DENABLE_BZIP2=OFF
            -DENABLE_LZMA=OFF
            ${ZLIB_ROOT}
            -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
            <SOURCE_DIR>
        DEPENDS extern-zlib-project
        )
    ExternalProject_Get_Property(extern-libzip-project INSTALL_DIR)

    if (WIN32)
        set(LIBZIP_LIBRARY ${INSTALL_DIR}/lib/zip.lib)
    else ()
        set(LIBZIP_LIBRARY ${INSTALL_DIR}/${CMAKE_INSTALL_LIBDIR}/libzip.a)
    endif ()

    add_library(zip STATIC IMPORTED)
    set_property(TARGET zip PROPERTY IMPORTED_LOCATION ${LIBZIP_LIBRARY})
    add_dependencies(zip extern-libzip-project)
    set(LIBZIP_INCLUDE_DIRS ${INSTALL_DIR}/include)
    set(LIBZIP_LIBRARIES zip)
endif ()

add_library(libzip INTERFACE)
target_include_directories(libzip INTERFACE ${LIBZIP_INCLUDE_DIRS})
target_link_libraries(libzip INTERFACE ${LIBZIP_LIBRARIES} zlib)

# vim:set shiftwidth=4 softtabstop=4 expandtab:
