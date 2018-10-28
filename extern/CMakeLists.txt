# Copyright 2018 Miklos Vajna. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# 'git apply' is more cross-platform than 'patch -p1 -i'.
set(PATCH_EXECUTABLE git --work-tree=. --git-dir=.git apply --ignore-whitespace --whitespace=nowarn --verbose)

add_subdirectory(googletest)
add_subdirectory(zlib)
add_subdirectory(libzip)
add_subdirectory(libxml2)
if (NOT WIN32)
    add_subdirectory(nss)
endif ()
add_subdirectory(xmlsec)

# vim:set shiftwidth=4 softtabstop=4 expandtab:
