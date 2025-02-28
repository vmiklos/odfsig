# Copyright 2018 Miklos Vajna
#
# SPDX-License-Identifier: MIT

set(LIBXML2_VERSION 2.13.6)
set(LIBXML2_SHA256SUM f453480307524968f7a04ec65e64f2a83a825973bcd260a2e7691be82ae70c96)

if (NOT ODFSIG_INTERNAL_LIBXML2)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(LIBXML2 REQUIRED libxml-2.0)
else ()
    include(ExternalProject)
    include(GNUInstallDirs)

    if (WIN32)
        set(CONFIGURE_COMMAND ${CMAKE_COMMAND} -E chdir win32
            cscript /e:javascript configure.js iconv=no sax1=yes)
        set(BUILD_COMMAND ${CMAKE_COMMAND} -E chdir win32
            nmake)
        set(INSTALL_COMMAND
            COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/bin/$<CONFIG>
            COMMAND ${CMAKE_COMMAND} -E copy win32/bin.msvc/libxml2.dll ${CMAKE_BINARY_DIR}/bin/$<CONFIG>/
            )
    else()
        set(CONFIGURE_COMMAND ./configure
            --prefix=<INSTALL_DIR>
            --libdir=<INSTALL_DIR>/lib
            --disable-ipv6
            --without-python
            --without-zlib
            --with-sax1
            --without-lzma
            --disable-shared)
        set(BUILD_COMMAND $(MAKE))
        set(INSTALL_COMMAND COMMAND $(MAKE) install)
    endif()

    ExternalProject_Add(extern-libxml2-project
        URL https://download.gnome.org/sources/libxml2/2.13/libxml2-${LIBXML2_VERSION}.tar.xz
        URL_HASH SHA256=${LIBXML2_SHA256SUM}
        DOWNLOAD_DIR ${CMAKE_SOURCE_DIR}/extern/tarballs
        BUILD_IN_SOURCE ON
        CONFIGURE_COMMAND ${CONFIGURE_COMMAND}
        BUILD_COMMAND ${BUILD_COMMAND}
        INSTALL_COMMAND ${INSTALL_COMMAND}
        )

    if (WIN32)
        ExternalProject_Get_Property(extern-libxml2-project SOURCE_DIR)
        set(LIBXML2_INCLUDE_DIRS ${SOURCE_DIR}/include/)
        set(LIBXML2_LIBRARY ${SOURCE_DIR}/win32/bin.msvc/libxml2.lib)

        install(PROGRAMS
            ${CMAKE_BINARY_DIR}/bin/$<CONFIG>/libxml2.dll
            DESTINATION ${CMAKE_INSTALL_BINDIR})
    else ()
        ExternalProject_Get_Property(extern-libxml2-project INSTALL_DIR)
        set(LIBXML2_INCLUDE_DIRS ${INSTALL_DIR}/include/libxml2)
        set(LIBXML2_LIBRARY ${INSTALL_DIR}/lib/libxml2.a)
    endif ()

    add_library(xml2 STATIC IMPORTED)
    set_property(TARGET xml2 PROPERTY IMPORTED_LOCATION ${LIBXML2_LIBRARY})
    add_dependencies(xml2 extern-libxml2-project)

    set(LIBXML2_LIBRARIES
        xml2
        ${ICONV}
        )
endif ()

add_library(libxml2 INTERFACE)
target_include_directories(libxml2 INTERFACE ${LIBXML2_INCLUDE_DIRS})
target_link_libraries(libxml2 INTERFACE ${LIBXML2_LIBRARIES})

# vim:set shiftwidth=4 softtabstop=4 expandtab:
