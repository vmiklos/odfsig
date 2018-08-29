cmake_minimum_required(VERSION 2.6)
project(odfsig)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(ODFSIG_ENABLE_WERROR "Fail and stop if a warning is triggered." OFF)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wextra")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wnon-virtual-dtor")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wendif-labels")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wundef")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wunreachable-code")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wunused-macros")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wshadow")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Woverloaded-virtual")
if (ODFSIG_ENABLE_WERROR)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror")
endif (ODFSIG_ENABLE_WERROR)

# Externals.
find_package(PkgConfig REQUIRED)

# libzip external.
pkg_check_modules(LIBZIP REQUIRED libzip)

# xmlsec external.
pkg_check_modules(LIBXML2 REQUIRED libxml-2.0)
pkg_check_modules(XMLSEC1NSS REQUIRED xmlsec1-nss)

add_executable(odfsig odfsig.cxx)
target_include_directories(odfsig PRIVATE
    ${LIBZIP_INCLUDE_DIRS}
    ${LIBXML2_INCLUDE_DIRS}
    ${XMLSEC1NSS_INCLUDE_DIRS}
    )
target_link_libraries(odfsig
    ${LIBZIP_LIBRARIES}
    ${LIBXML2_LIBRARIES}
    ${XMLSEC1NSS_LIBRARIES}
    )
target_compile_options(odfsig PRIVATE ${XMLSEC1NSS_CFLAGS_OTHER})

# vim:set shiftwidth=4 softtabstop=4 expandtab:
