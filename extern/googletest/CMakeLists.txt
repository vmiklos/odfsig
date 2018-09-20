# Copyright 2018 Miklos Vajna. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

if (NOT ODFSIG_INTERNAL_LIBGTEST)
    find_package(GTest REQUIRED)
else ()
    include(ExternalProject)
    include(GNUInstallDirs)
    find_package(Threads REQUIRED)
    ExternalProject_Add(extern-googletest-project
        URL https://github.com/google/googletest/archive/release-1.8.1.tar.gz
        URL_HASH SHA256=9bf1fe5182a604b4135edc1a425ae356c9ad15e9b23f9f12a02e80184c3a249c
        DOWNLOAD_DIR ${CMAKE_SOURCE_DIR}/extern/tarballs
        BUILD_IN_SOURCE ON
        CONFIGURE_COMMAND ${CMAKE_COMMAND}
            -DCMAKE_INSTALL_PREFIX=${CMAKE_CURRENT_BINARY_DIR}/install
            -Dgtest_force_shared_crt=ON
        )

    if (WIN32)
        set(GTEST_LIBRARY ${CMAKE_CURRENT_BINARY_DIR}/install/lib/gtest.lib)
        set(GTEST_MAIN_LIBRARY ${CMAKE_CURRENT_BINARY_DIR}/install/lib/gtest_main.lib)
    else ()
        set(GTEST_LIBRARY ${CMAKE_CURRENT_BINARY_DIR}/install/${CMAKE_INSTALL_LIBDIR}/libgtest.a)
        set(GTEST_MAIN_LIBRARY ${CMAKE_CURRENT_BINARY_DIR}/install/${CMAKE_INSTALL_LIBDIR}/libgtest_main.a)
    endif ()

    add_library(gtest STATIC IMPORTED)
    set_property(TARGET gtest PROPERTY IMPORTED_LOCATION ${GTEST_LIBRARY})
    add_dependencies(gtest extern-googletest-project)
    add_library(gtest_main STATIC IMPORTED)
    set_property(TARGET gtest_main PROPERTY IMPORTED_LOCATION ${GTEST_MAIN_LIBRARY})
    add_dependencies(gtest_main extern-googletest-project)
    set(GTEST_INCLUDE_DIRS ${CMAKE_CURRENT_BINARY_DIR}/install/include)
    set(GTEST_BOTH_LIBRARIES gtest gtest_main ${CMAKE_THREAD_LIBS_INIT})
endif ()

add_library(googletest INTERFACE)
target_include_directories(googletest INTERFACE ${GTEST_INCLUDE_DIRS})
target_link_libraries(googletest INTERFACE ${GTEST_BOTH_LIBRARIES})

# vim:set shiftwidth=4 softtabstop=4 expandtab:
