# Copyright 2018 Miklos Vajna. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

include(FindGit)

set(ODFSIG_VERSION_MAJOR 4)
set(ODFSIG_VERSION_MINOR 0)
if (GIT_FOUND AND EXISTS "${CMAKE_SOURCE_DIR}/.git")
    execute_process(COMMAND ${GIT_EXECUTABLE} rev-parse HEAD
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE ODFSIG_VERSION_GIT
        OUTPUT_STRIP_TRAILING_WHITESPACE)
endif ()

configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/odfsig/version.hxx.in"
  "${CMAKE_CURRENT_BINARY_DIR}/odfsig/version.hxx"
  )

set(ODFSIG_ARCHIVE_NAME "odfsig-${ODFSIG_VERSION_MAJOR}.${ODFSIG_VERSION_MINOR}")
add_custom_target(dist
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/dist
    COMMAND git archive --prefix=${ODFSIG_ARCHIVE_NAME}/ -o ${CMAKE_BINARY_DIR}/dist/${ODFSIG_ARCHIVE_NAME}.tar.gz HEAD
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    )

set(CPACK_PACKAGE_VERSION ${ODFSIG_VERSION_MAJOR}.${ODFSIG_VERSION_MINOR})
set(CPACK_SYSTEM_NAME linux)
set(CPACK_GENERATOR TGZ)
include(CPack)

if (WIN32)
elseif (APPLE)
else ()
add_custom_target(pack
    COMMAND cpack
    COMMAND ${CMAKE_COMMAND} -E make_directory dist
    COMMAND ${CMAKE_COMMAND} -E copy odfsig-${CPACK_PACKAGE_VERSION}-${CPACK_SYSTEM_NAME}.tar.gz dist/
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
add_dependencies(pack dist)
endif()

# vim:set shiftwidth=4 softtabstop=4 expandtab:
