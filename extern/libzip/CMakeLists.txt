# Copyright 2018 Miklos Vajna. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

if (WIN32)
    set(ZLIB_ROOT_OPTION -DZLIB_ROOT=${CMAKE_CURRENT_BINARY_DIR}/../zlib/install)
    set(LIBZIP_LIBRARY ${CMAKE_CURRENT_BINARY_DIR}/install/lib/zip.lib)
else ()
    set(PATCH_COMMAND patch -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/add-an-option-to-disable-bzip2-support.patch)
    set(LIBZIP_LIBRARY ${CMAKE_CURRENT_BINARY_DIR}/install/${CMAKE_INSTALL_LIBDIR}/libzip.a)
endif()

if (NOT ODFSIG_INTERNAL_LIBZIP)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(LIBZIP REQUIRED libzip)
else ()
    include(ExternalProject)
    include(GNUInstallDirs)
    if (NOT ODFSIG_INTERNAL_LIBXML2)
        find_package(PkgConfig REQUIRED)
        pkg_check_modules(ZLIB REQUIRED zlib)
    else()
        set(ZLIB_INCLUDE_DIRS "")
        set(ZLIB_LIBRARIES zlib)
    endif()
    ExternalProject_Add(extern-libzip-project
        URL https://libzip.org/download/libzip-1.5.1.tar.gz
        URL_HASH SHA256=47eaa45faa448c72bd6906e5a096846c469a185f293cafd8456abb165841b3f2
        PATCH_COMMAND ${PATCH_COMMAND}
        DOWNLOAD_DIR ${CMAKE_SOURCE_DIR}/extern/tarballs
        BUILD_IN_SOURCE ON
        CONFIGURE_COMMAND ${CMAKE_COMMAND}
            -DBUILD_SHARED_LIBS=OFF
            -DENABLE_GNUTLS=OFF
            -DENABLE_OPENSSL=OFF
            -DENABLE_COMMONCRYPTO=OFF
            -DENABLE_BZIP2=OFF
            ${ZLIB_ROOT_OPTION}
            -DCMAKE_INSTALL_PREFIX=${CMAKE_CURRENT_BINARY_DIR}/install
        DEPENDS extern-zlib-project
        )
    add_library(zip STATIC IMPORTED)
    set_property(TARGET zip PROPERTY IMPORTED_LOCATION ${LIBZIP_LIBRARY})
    add_dependencies(zip extern-libzip-project)
    set(LIBZIP_INCLUDE_DIRS
        ${CMAKE_CURRENT_BINARY_DIR}/install/include
        ${ZLIB_INCLUDE_DIRS}
        )
    set(LIBZIP_LIBRARIES
        zip
        ${ZLIB_LIBRARIES})
endif ()

add_library(libzip INTERFACE)
target_include_directories(libzip INTERFACE ${LIBZIP_INCLUDE_DIRS})
target_link_libraries(libzip INTERFACE ${LIBZIP_LIBRARIES})

# vim:set shiftwidth=4 softtabstop=4 expandtab:
