# Copyright 2018 Miklos Vajna. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

enable_testing()

add_executable(odfsigtest
    testlib.cxx
    )
target_link_libraries(odfsigtest
    odfsigcore
    googletest
    )
add_test(NAME odfsig
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMAND odfsigtest
    )

set(CMAKE_CTEST_COMMAND ctest -V)
add_custom_target(check
    COMMAND ${CMAKE_CTEST_COMMAND} -C ${CMAKE_BUILD_TYPE}
    )
add_dependencies(check
    odfsigtest
    )

# vim:set shiftwidth=4 softtabstop=4 expandtab:
