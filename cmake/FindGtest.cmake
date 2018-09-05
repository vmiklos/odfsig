# Copyright 2018 Miklos Vajna. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

include(ExternalProject)
include(GNUInstallDirs)

if (NOT ODFSIG_INTERNAL_LIBGTEST)
    find_package(GTest REQUIRED)
    set(LIBGTEST_LIBRARIES GTest::GTest GTest::Main)
else ()
    ExternalProject_Add(externalgtest
        URL https://github.com/google/googletest/archive/release-1.8.1.tar.gz
        SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/external/libgtest-source
        BUILD_IN_SOURCE ON
        CONFIGURE_COMMAND cmake
            -DCMAKE_INSTALL_PREFIX=${CMAKE_CURRENT_BINARY_DIR}/external/libgtest-install
        BUILD_COMMAND make
        INSTALL_COMMAND make install
        )
    add_library(GTest::GTest STATIC IMPORTED)
    set_property(TARGET GTest::GTest PROPERTY IMPORTED_LOCATION ${CMAKE_CURRENT_BINARY_DIR}/external/libgtest-install/${CMAKE_INSTALL_LIBDIR}/libgtest.a)
    add_dependencies(GTest::GTest externalgtest)
    add_library(GTest::Main STATIC IMPORTED)
    set_property(TARGET GTest::Main PROPERTY IMPORTED_LOCATION ${CMAKE_CURRENT_BINARY_DIR}/external/libgtest-install/${CMAKE_INSTALL_LIBDIR}/libgtest_main.a)
    add_dependencies(GTest::Main externalgtest)
    set(LIBGTEST_INCLUDE_DIRS ${CMAKE_CURRENT_BINARY_DIR}/external/libgtest-install/include)
    set(LIBGTEST_LIBRARIES GTest::GTest GTest::Main pthread)
endif ()

# vim:set shiftwidth=4 softtabstop=4 expandtab:
