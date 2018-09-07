# Copyright 2018 Miklos Vajna. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

cmake_minimum_required(VERSION 3.4.3)

project(odfsig)

set(CMAKE_MODULE_PATH
    ${CMAKE_MODULE_PATH}
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake"
    )

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(ODFSIG_ENABLE_WERROR "Fail and stop if a warning is triggered." OFF)
option(ODFSIG_ENABLE_CCACHE "Try to use ccache automatically" ON)
option(ODFSIG_ENABLE_GCOV "Measure code coverage of tests." OFF)
option(ODFSIG_INTERNAL_LIBZIP "Use internal libzip." OFF)
option(ODFSIG_INTERNAL_LIBGTEST "Use internal libgtest." OFF)

if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "No build type selected, default to Release")
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type (default Release)" FORCE)
endif()

include(HandleOdfsigOptions)

# xmlsec external.
find_package(PkgConfig REQUIRED)
pkg_check_modules(XMLSEC1NSS REQUIRED xmlsec1-nss)
add_library(libxmlsec INTERFACE)
target_include_directories(libxmlsec INTERFACE ${XMLSEC1NSS_INCLUDE_DIRS})
target_link_libraries(libxmlsec INTERFACE ${XMLSEC1NSS_LIBRARIES})
target_compile_options(libxmlsec INTERFACE ${XMLSEC1NSS_CFLAGS_OTHER})

add_subdirectory(src)
add_subdirectory(apps)
add_subdirectory(tests)
add_subdirectory(extern)

# vim:set shiftwidth=4 softtabstop=4 expandtab:
