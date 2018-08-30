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
pkg_check_modules(XMLSEC1NSS REQUIRED xmlsec1-nss)

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
enable_testing()
find_package(GTest REQUIRED)
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
