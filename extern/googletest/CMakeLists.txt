# Copyright 2018 Miklos Vajna
#
# SPDX-License-Identifier: MIT

set(GOOGLETEST_VERSION 1.13.0)
set(GOOGLETEST_SHA256SUM ad7fdba11ea011c1d925b3289cf4af2c66a352e18d4c7264392fead75e919363)

if (NOT ODFSIG_INTERNAL_LIBGTEST)
    find_package(GTest REQUIRED)
else ()
    include(ExternalProject)
    include(GNUInstallDirs)
    find_package(Threads REQUIRED)
    ExternalProject_Add(extern-googletest-project
        URL https://github.com/google/googletest/archive/refs/tags/v${GOOGLETEST_VERSION}.tar.gz
        URL_HASH SHA256=${GOOGLETEST_SHA256SUM}
        DOWNLOAD_DIR ${CMAKE_SOURCE_DIR}/extern/tarballs
        CONFIGURE_COMMAND ${CMAKE_COMMAND}
            -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
            -Dgtest_force_shared_crt=ON
            <SOURCE_DIR>
        )

    ExternalProject_Get_Property(extern-googletest-project INSTALL_DIR)
    add_library(gtest STATIC IMPORTED)
    add_library(gtest_main STATIC IMPORTED)
    if (WIN32)
        set_property(TARGET gtest PROPERTY IMPORTED_LOCATION ${INSTALL_DIR}/lib/gtest.lib)
        set_property(TARGET gtest_main PROPERTY IMPORTED_LOCATION ${INSTALL_DIR}/lib/gtest_main.lib)
    else ()
        set_property(TARGET gtest PROPERTY IMPORTED_LOCATION ${INSTALL_DIR}/${CMAKE_INSTALL_LIBDIR}/libgtest.a)
        set_property(TARGET gtest_main PROPERTY IMPORTED_LOCATION ${INSTALL_DIR}/${CMAKE_INSTALL_LIBDIR}/libgtest_main.a)
    endif ()

    add_dependencies(gtest extern-googletest-project)
    add_dependencies(gtest_main extern-googletest-project)
    set(GTEST_INCLUDE_DIRS ${INSTALL_DIR}/include)
    set(GTEST_BOTH_LIBRARIES gtest gtest_main ${CMAKE_THREAD_LIBS_INIT})
endif ()

add_library(googletest INTERFACE)
target_include_directories(googletest INTERFACE ${GTEST_INCLUDE_DIRS})
target_link_libraries(googletest INTERFACE ${GTEST_BOTH_LIBRARIES})

# vim:set shiftwidth=4 softtabstop=4 expandtab:
