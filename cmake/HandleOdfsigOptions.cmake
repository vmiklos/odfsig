# Copyright 2018 Miklos Vajna. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

if (ODFSIG_ENABLE_CCACHE)
    find_program(CCACHE_PROGRAM ccache)
    if (CCACHE_PROGRAM)
        set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
    endif ()
endif ()

set(CMAKE_CXX_STANDARD 17)
if (WIN32)
    # more or less equivalent of the below -Wall and -Wextra
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
    # would warn on e.g. getenv()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_CRT_SECURE_NO_DEPRECATE")
    # would warn on <codecvt> usage
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING")
    # possible loss of data, libzip vs libxml2 different int types
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -wd4244")
    if (ODFSIG_ENABLE_WERROR)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /WX")
    endif ()
else()
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
endif()
if (ODFSIG_ENABLE_GCOV)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage")
endif ()

option(ODFSIG_IWYU "Run include-what-you-use with the compiler." OFF)
if(ODFSIG_IWYU)
  find_program(IWYU_COMMAND NAMES include-what-you-use iwyu)
  if(NOT IWYU_COMMAND)
    message(FATAL_ERROR "ODFSIG_IWYU is ON but include-what-you-use is not found!")
  endif()
  set(CMAKE_CXX_INCLUDE_WHAT_YOU_USE
    "${IWYU_COMMAND};-Xiwyu;--mapping_file=${CMAKE_SOURCE_DIR}/util/iwyu/mapping.imp")
endif()

# vim:set shiftwidth=4 softtabstop=4 expandtab:
