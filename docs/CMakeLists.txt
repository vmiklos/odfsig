# Copyright 2018 Miklos Vajna. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

if (A2X)
    add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/odfsig.1
        COMMAND ${A2X} -f manpage -D ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/odfsig.adoc
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/odfsig.adoc
        COMMENT "Building manpage"
        VERBATIM)

    add_custom_target(man ALL DEPENDS odfsig.1)
endif ()

# vim:set shiftwidth=4 softtabstop=4 expandtab:
