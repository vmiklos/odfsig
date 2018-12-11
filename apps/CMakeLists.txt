# Copyright 2018 Miklos Vajna. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

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
