cmake_minimum_required(VERSION 2.6)
project(odfsig)

# libzip external.
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBZIP REQUIRED libzip)

add_executable(odfsig odfsig.cxx)
target_include_directories(odfsig PRIVATE ${LIBZIP_INCLUDE_DIRS})
target_link_libraries(odfsig ${LIBZIP_LIBRARIES})
