# Copyright 2018 Miklos Vajna
#
# SPDX-License-Identifier: MIT

# odfsig cmdline app.
add_executable(odfsig
    app.cxx
    )
target_link_libraries(odfsig
    odfsigcore
    )

install(TARGETS odfsig
    DESTINATION bin
    )

if (ODFSIG_FUZZ)
    add_executable(odfsigfuzz
        fuzz.cxx
        )
    target_link_libraries(odfsigfuzz
        odfsigcore
        -fsanitize=fuzzer
        )
endif ()

# vim:set shiftwidth=4 softtabstop=4 expandtab:
