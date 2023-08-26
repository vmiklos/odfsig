# Copyright 2018 Miklos Vajna
#
# SPDX-License-Identifier: MIT

set(ZLIB_VERSION 1.3)
set(ZLIB_SHA256SUM ff0ba4c292013dbc27530b3a81e1f9a813cd39de01ca5e0f8bf355702efa593e)

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
