# Copyright 2018 Miklos Vajna. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

set(ZLIB_VERSION 1.2.12)
set(ZLIB_SHA256SUM 91844808532e5ce316b3c010929493c0244f3d37593afd6de04f71821d5136d9)

if (NOT ODFSIG_INTERNAL_ZLIB)
    find_package(ZLIB REQUIRED)
else ()
    include(ExternalProject)

    ExternalProject_Add(extern-zlib-project
        URL http://zlib.net/zlib-${ZLIB_VERSION}.tar.gz
        URL_HASH SHA256=${ZLIB_SHA256SUM}
        DOWNLOAD_DIR ${CMAKE_SOURCE_DIR}/extern/tarballs
        CONFIGURE_COMMAND ${CMAKE_COMMAND}
            -DCMAKE_DEBUG_POSTFIX=
            -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
            <SOURCE_DIR>
        )
    add_library(z STATIC IMPORTED)
    ExternalProject_Get_Property(extern-zlib-project INSTALL_DIR)

    if (WIN32)
        set_property(TARGET z PROPERTY IMPORTED_LOCATION ${INSTALL_DIR}/lib/zlibstatic.lib)
        set_property(TARGET z PROPERTY IMPORTED_LOCATION_DEBUG ${INSTALL_DIR}/lib/zlibstaticd.lib)
    else ()
        set_property(TARGET z PROPERTY IMPORTED_LOCATION ${INSTALL_DIR}/lib/libz.a)
    endif ()

    add_dependencies(z extern-zlib-project)
    set(ZLIB_INCLUDE_DIR ${INSTALL_DIR}/include)
endif ()

add_library(zlib INTERFACE)
target_include_directories(zlib INTERFACE ${ZLIB_INCLUDE_DIR})
target_link_libraries(zlib INTERFACE z)

# vim:set shiftwidth=4 softtabstop=4 expandtab:
