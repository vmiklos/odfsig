# Copyright 2018 Miklos Vajna
#
# SPDX-License-Identifier: MIT

find_program(PANDOC
    pandoc
    )

if (PANDOC)
    message(STATUS "Found pandoc, building documentation.")
else ()
    message(STATUS "Found no pandoc, not building documentation.")
endif ()

# vim:set shiftwidth=4 softtabstop=4 expandtab:
