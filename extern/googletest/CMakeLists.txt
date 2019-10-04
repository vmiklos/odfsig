# Copyright 2018 Miklos Vajna. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

set(GOOGLETEST_VERSION 1.10.0)
set(GOOGLETEST_SHA256SUM 9dc9157a9a1551ec7a7e43daea9a694a0bb5fb8bec81235d8a1e6ef64c716dcb)

if (NOT ODFSIG_INTERNAL_LIBGTEST)
    find_package(GTest REQUIRED)
else ()
    include(ExternalProject)
    include(GNUInstallDirs)
    find_package(Threads REQUIRED)
    ExternalProject_Add(extern-googletest-project
        URL https://github.com/google/googletest/archive/release-${GOOGLETEST_VERSION}.tar.gz
        URL_HASH SHA256=${GOOGLETEST_SHA256SUM}
        PATCH_COMMAND ${PATCH_EXECUTABLE}
            ${CMAKE_CURRENT_SOURCE_DIR}/msvc-debug.patch
        DOWNLOAD_DIR ${CMAKE_SOURCE_DIR}/extern/tarballs
        CONFIGURE_COMMAND ${CMAKE_COMMAND}
            -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
            -Dgtest_force_shared_crt=ON
            <SOURCE_DIR>
        )

    ExternalProject_Get_Property(extern-googletest-project INSTALL_DIR)
    if (WIN32)
        set(GTEST_LIBRARY ${INSTALL_DIR}/lib/gtest.lib)
        set(GTEST_MAIN_LIBRARY ${INSTALL_DIR}/lib/gtest_main.lib)
    else ()
        set(GTEST_LIBRARY ${INSTALL_DIR}/${CMAKE_INSTALL_LIBDIR}/libgtest.a)
        set(GTEST_MAIN_LIBRARY ${INSTALL_DIR}/${CMAKE_INSTALL_LIBDIR}/libgtest_main.a)
    endif ()

    add_library(gtest STATIC IMPORTED)
    set_property(TARGET gtest PROPERTY IMPORTED_LOCATION ${GTEST_LIBRARY})
    add_dependencies(gtest extern-googletest-project)
    add_library(gtest_main STATIC IMPORTED)
    set_property(TARGET gtest_main PROPERTY IMPORTED_LOCATION ${GTEST_MAIN_LIBRARY})
    add_dependencies(gtest_main extern-googletest-project)
    set(GTEST_INCLUDE_DIRS ${INSTALL_DIR}/include)
    set(GTEST_BOTH_LIBRARIES gtest gtest_main ${CMAKE_THREAD_LIBS_INIT})
endif ()

add_library(googletest INTERFACE)
target_include_directories(googletest INTERFACE ${GTEST_INCLUDE_DIRS})
target_link_libraries(googletest INTERFACE ${GTEST_BOTH_LIBRARIES})

# vim:set shiftwidth=4 softtabstop=4 expandtab:
