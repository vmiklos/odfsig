# Copyright 2018 Miklos Vajna. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

include(FindGit)

set(ODFSIG_VERSION_MAJOR 0)
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

# vim:set shiftwidth=4 softtabstop=4 expandtab:
