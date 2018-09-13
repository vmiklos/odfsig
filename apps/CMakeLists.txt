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

# vim:set shiftwidth=4 softtabstop=4 expandtab:
