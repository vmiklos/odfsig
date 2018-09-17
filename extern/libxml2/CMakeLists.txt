# Copyright 2018 Miklos Vajna. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

if (ODFSIG_INTERNAL_LIBXML2)
    include(ExternalProject)
    include(GNUInstallDirs)

    if (WIN32)
        set(CONFIGURE_COMMAND cd win32 &&
            cscript /e:javascript configure.js iconv=no sax1=yes)
        set(BUILD_COMMAND cd win32 &&
            nmake)
        set(INSTALL_COMMAND echo NOP)
    else()
        set(CONFIGURE_COMMAND ./configure
            --prefix=${CMAKE_CURRENT_BINARY_DIR}/install
            --disable-ipv6
            --without-python
            --without-zlib
            --with-sax1
            --without-lzma
            --disable-shared)
        set(BUILD_COMMAND $(MAKE))
        set(INSTALL_COMMAND $(MAKE) install)
    endif()
    ExternalProject_Add(extern-libxml2-project
        URL http://xmlsoft.org/sources/libxml2-2.9.8.tar.gz
        URL_HASH SHA256=0b74e51595654f958148759cfef0993114ddccccbb6f31aee018f3558e8e2732
        DOWNLOAD_DIR ${CMAKE_SOURCE_DIR}/extern/tarballs
        BUILD_IN_SOURCE ON
        CONFIGURE_COMMAND ${CONFIGURE_COMMAND}
        BUILD_COMMAND ${BUILD_COMMAND}
        INSTALL_COMMAND ${INSTALL_COMMAND}
        )
    add_library(xml2 STATIC IMPORTED)
    set_property(TARGET xml2 PROPERTY IMPORTED_LOCATION ${CMAKE_CURRENT_BINARY_DIR}/install/${CMAKE_INSTALL_LIBDIR}/libxml2.a)
    add_dependencies(xml2 extern-libxml2-project)

    set(LIBXML2_INCLUDE_DIRS
        ${CMAKE_CURRENT_BINARY_DIR}/install/include/libxml2
        )
    set(LIBXML2_LIBRARIES
        xml2
        )
endif ()

add_library(libxml2 INTERFACE)
target_include_directories(libxml2 INTERFACE ${LIBXML2_INCLUDE_DIRS})
target_link_libraries(libxml2 INTERFACE ${LIBXML2_LIBRARIES})

# vim:set shiftwidth=4 softtabstop=4 expandtab:
