# Copyright 2018 Miklos Vajna. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

cmake_minimum_required(VERSION 3.4.3)
# Add path for custom modules
set(CMAKE_MODULE_PATH
    ${CMAKE_MODULE_PATH}
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake"
    )

project(odfsig)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(ODFSIG_ENABLE_WERROR "Fail and stop if a warning is triggered." OFF)
option(ODFSIG_ENABLE_GCOV "Measure code coverage of tests." OFF)
option(ODFSIG_INTERNAL_LIBZIP "Use internal libzip." OFF)
option(ODFSIG_INTERNAL_LIBGTEST "Use internal libgtest." OFF)

find_program(CCACHE_PROGRAM ccache)
if (CCACHE_PROGRAM)
    set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
endif ()

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wextra")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wnon-virtual-dtor")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wendif-labels")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wunreachable-code")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wunused-macros")
if (NOT ODFSIG_INTERNAL_LIBZIP)
    # libzip has a zip_stat that is both a struct and a function name.
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wshadow")
endif ()
if (NOT ODFSIG_INTERNAL_LIBGTEST)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wundef")
endif ()
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Woverloaded-virtual")
if (ODFSIG_ENABLE_WERROR)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror")
endif ()
if (ODFSIG_ENABLE_GCOV)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage")
endif ()

# Testing.
enable_testing()

# Externals.
find_package(PkgConfig REQUIRED)

# libzip external.
include(FindZip)

# xmlsec external.
pkg_check_modules(XMLSEC1NSS REQUIRED xmlsec1-nss)

# googletest external.
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

# odfsig library.
add_library(odfsigcore src/lib.cxx)
target_include_directories(odfsigcore PRIVATE
    ${LIBZIP_INCLUDE_DIRS}
    ${XMLSEC1NSS_INCLUDE_DIRS}
    )
target_include_directories(odfsigcore PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(odfsigcore
    ${LIBZIP_LIBRARIES}
    ${XMLSEC1NSS_LIBRARIES}
    )
target_compile_options(odfsigcore PRIVATE ${XMLSEC1NSS_CFLAGS_OTHER})

# odfsig tests.
add_executable(odfsigtest tests/testlib.cxx)
target_link_libraries(odfsigtest
    odfsigcore
    ${LIBGTEST_LIBRARIES}
    )
target_include_directories(odfsigtest PRIVATE
    ${LIBGTEST_INCLUDE_DIRS}
    )
add_test(NAME odfsig
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND odfsigtest
    )

set(CMAKE_CTEST_COMMAND ctest -V)
add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND})
add_dependencies(check odfsigtest)

# odfsig cmdline app.
add_executable(odfsig apps/app.cxx)
target_link_libraries(odfsig odfsigcore)

# vim:set shiftwidth=4 softtabstop=4 expandtab:
