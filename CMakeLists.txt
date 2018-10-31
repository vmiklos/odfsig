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
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

option(ODFSIG_ENABLE_WERROR "Fail and stop if a warning is triggered." OFF)
option(ODFSIG_ENABLE_CCACHE "Try to use ccache automatically" ON)
option(ODFSIG_ENABLE_GCOV "Measure code coverage of tests." OFF)
if (WIN32 OR APPLE)
    set(ODFSIG_INTERNAL_LIBS_DEFAULT ON)
else ()
    set(ODFSIG_INTERNAL_LIBS_DEFAULT OFF)
endif ()
option(ODFSIG_INTERNAL_LIBS "Use internal libraries." ${ODFSIG_INTERNAL_LIBS_DEFAULT})
option(ODFSIG_INTERNAL_LIBZIP "Use internal libzip." ${ODFSIG_INTERNAL_LIBS})
option(ODFSIG_INTERNAL_LIBGTEST "Use internal libgtest." ${ODFSIG_INTERNAL_LIBS})
option(ODFSIG_INTERNAL_XMLSEC "Use internal xmlsec." ${ODFSIG_INTERNAL_LIBS})
option(ODFSIG_INTERNAL_LIBXML2 "Use internal libxml2." ${ODFSIG_INTERNAL_LIBS})
if (NOT WIN32)
    option(ODFSIG_INTERNAL_NSS "Use internal nss." ${ODFSIG_INTERNAL_LIBS})
endif ()
option(ODFSIG_INTERNAL_ZLIB "Use internal zlib." ${ODFSIG_INTERNAL_LIBS})

if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "No build type selected, default to Release")
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type (default Release)" FORCE)
endif()

include(HandleAsciidoc)
include(HandleOdfsigOptions)

add_subdirectory(apps)
add_subdirectory(docs)
add_subdirectory(extern)
add_subdirectory(include)
add_subdirectory(src)
add_subdirectory(tests)

# vim:set shiftwidth=4 softtabstop=4 expandtab:
