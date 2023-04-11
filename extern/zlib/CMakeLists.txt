# Copyright 2018 Miklos Vajna
#
# SPDX-License-Identifier: MIT

set(ZLIB_VERSION 1.2.13)
set(ZLIB_SHA256SUM b3a24de97a8fdbc835b9833169501030b8977031bcb54b3b3ac13740f846ab30)

if (NOT ODFSIG_INTERNAL_ZLIB)
    find_package(ZLIB REQUIRED)
else ()
    include(ExternalProject)

    ExternalProject_Add(extern-zlib-project
        URL http://zlib.net/zlib-${ZLIB_VERSION}.tar.gz
        URL_HASH SHA256=${ZLIB_SHA256SUM}
        DOWNLOAD_DIR ${CMAKE_SOURCE_DIR}/extern/tarballs
        CONFIGURE_COMMAND ${CMAKE_COMMAND}
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
