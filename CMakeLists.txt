cmake_minimum_required(VERSION 2.6)
project(odfsig)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(ODFSIG_ENABLE_WERROR "Fail and stop if a warning is triggered." OFF)
option(ODFSIG_ENABLE_GCOV "Measure code coverage of tests." OFF)
option(ODFSIG_INTERNAL_LIBZIP "Use internal libzip." OFF)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wextra")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wnon-virtual-dtor")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wendif-labels")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wundef")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wunreachable-code")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wunused-macros")
if (NOT ODFSIG_INTERNAL_LIBZIP)
    # libzip has a zip_stat that is both a struct and a function name.
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wshadow")
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
include(ExternalProject)

# libzip external.
if (NOT ODFSIG_INTERNAL_LIBZIP)
    pkg_check_modules(LIBZIP REQUIRED libzip)
else ()
    pkg_check_modules(ZLIB REQUIRED zlib)
    ExternalProject_Add(externalzip
        URL https://libzip.org/download/libzip-1.5.1.tar.gz
        SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/external/libzip-source
        BUILD_IN_SOURCE ON
        CONFIGURE_COMMAND cmake
            -DBUILD_SHARED_LIBS=OFF
            -DENABLE_GNUTLS=OFF
            -DENABLE_OPENSSL=OFF
            -DENABLE_COMMONCRYPTO=OFF
            -DCMAKE_INSTALL_PREFIX=${CMAKE_CURRENT_BINARY_DIR}/external/libzip-install
        BUILD_COMMAND make
        INSTALL_COMMAND make install
        )
    add_library(zip STATIC IMPORTED)
    set_property(TARGET zip PROPERTY IMPORTED_LOCATION ${CMAKE_CURRENT_BINARY_DIR}/external/libzip-source/lib/libzip.a)
    add_dependencies(zip externalzip)
    # Header must be picked up from the install dir, as zipconf.h is generated.
    set(LIBZIP_INCLUDE_DIRS ${CMAKE_CURRENT_BINARY_DIR}/external/libzip-install/include ${ZLIB_INCLUDE_DIRS})
    # Library is easier to pick up from the source dir, as that location is
    # platform-independent.
    # bz2 may not provide a pkg-config file, so just hardcode its name.
    set(LIBZIP_LIBRARIES zip ${ZLIB_LIBRARIES} bz2)
endif ()

# xmlsec external.
pkg_check_modules(XMLSEC1NSS REQUIRED xmlsec1-nss)

# googletest external.
find_package(GTest REQUIRED)

# odfsig library.
add_library(odfsigcore core/source/odfsig.cxx)
target_include_directories(odfsigcore PRIVATE
    ${LIBZIP_INCLUDE_DIRS}
    ${XMLSEC1NSS_INCLUDE_DIRS}
    )
target_include_directories(odfsigcore PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/core/include")
target_link_libraries(odfsigcore
    ${LIBZIP_LIBRARIES}
    ${XMLSEC1NSS_LIBRARIES}
    )
target_compile_options(odfsigcore PRIVATE ${XMLSEC1NSS_CFLAGS_OTHER})

# odfsig tests.
add_executable(odfsigtest core/qa/odfsigtest.cxx)
target_link_libraries(odfsigtest odfsigcore GTest::GTest GTest::Main)
add_test(NAME odfsig
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND odfsigtest
    )

set(CMAKE_CTEST_COMMAND ctest -V)
add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND})
add_dependencies(check odfsigtest)

# odfsig cmdline app.
add_executable(odfsig app/source/main.cxx)
target_link_libraries(odfsig odfsigcore)

# vim:set shiftwidth=4 softtabstop=4 expandtab:
