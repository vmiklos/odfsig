cmake_minimum_required(VERSION 2.6)
project(odfsig)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wextra")

# Externals.
find_package(PkgConfig REQUIRED)

# libzip external.
pkg_check_modules(LIBZIP REQUIRED libzip)

# libxml2 external.
pkg_check_modules(LIBXML2 REQUIRED libxml-2.0)

add_executable(odfsig odfsig.cxx)
target_include_directories(odfsig PRIVATE
	${LIBZIP_INCLUDE_DIRS}
	${LIBXML2_INCLUDE_DIRS}
	)
target_link_libraries(odfsig
	${LIBZIP_LIBRARIES}
	${LIBXML2_LIBRARIES}
	)
