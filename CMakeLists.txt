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
