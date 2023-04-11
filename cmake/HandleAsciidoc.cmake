# Copyright 2018 Miklos Vajna
#
# SPDX-License-Identifier: MIT

find_program(A2X
    a2x
    )

if (A2X)
    message(STATUS "Found a2x, building documentation.")
else ()
    message(STATUS "Found no a2x, not building documentation.")
endif ()

# vim:set shiftwidth=4 softtabstop=4 expandtab:
