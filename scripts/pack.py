#!/usr/bin/env python3
#
# Copyright 2018 Miklos Vajna. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#
# Script that creates a Windows binary archive, bundling all libraries which
# would be installed by Visual Studio only.
#

import os
import platform
import shutil
import zipfile


def main():
    instdir = "odfsig"
    if os.path.exists(instdir):
        shutil.rmtree(instdir)
    os.mkdir(instdir)
    bins = [
        "libxml2.dll",
        "libxmlsec.dll",
        "libxmlsec-mscng.dll",
        "odfsig.exe",
    ]
    for bin in bins:
        shutil.copyfile("workdir/bin/Release/" + bin, instdir + "/" + bin)
    sys32dlls = [
        "msvcp140.dll",
        "vcruntime140.dll",
        "api-ms-win-core-file-l1-2-0.dll",
        "api-ms-win-core-file-l2-1-0.dll",
        "api-ms-win-core-localization-l1-2-0.dll",
        "api-ms-win-core-processthreads-l1-1-1.dll",
        "api-ms-win-core-synch-l1-2-0.dll",
        "api-ms-win-core-timezone-l1-1-0.dll",
        "api-ms-win-crt-convert-l1-1-0.dll",
        "api-ms-win-crt-environment-l1-1-0.dll",
        "api-ms-win-crt-filesystem-l1-1-0.dll",
        "api-ms-win-crt-heap-l1-1-0.dll",
        "api-ms-win-crt-locale-l1-1-0.dll",
        "api-ms-win-crt-math-l1-1-0.dll",
        "api-ms-win-crt-multibyte-l1-1-0.dll",
        "api-ms-win-crt-runtime-l1-1-0.dll",
        "api-ms-win-crt-stdio-l1-1-0.dll",
        "api-ms-win-crt-string-l1-1-0.dll",
        "api-ms-win-crt-time-l1-1-0.dll",
        "api-ms-win-crt-utility-l1-1-0.dll",
        "ucrtbase.dll",
    ]
    for dll in sys32dlls:
        shutil.copyfile("c:/windows/syswow64/" + dll, instdir + "/" + dll)

    with zipfile.ZipFile(instdir + ".zip", "w") as zip:
        for root, dirs, files in os.walk("odfsig"):
            for name in files:
                if not name == "odfsig":
                    zip.write(os.path.join(root, name))

def mainDarwin():
    instdir = "odfsig"
    if os.path.exists(instdir):
        shutil.rmtree(instdir)
    os.mkdir(instdir)
    os.mkdir(instdir + "/out")
    bins = [
        "libfreebl3.dylib",
        "libnss3.dylib",
        "libnssckbi.dylib",
        "libnssdbm3.dylib",
        "libnssutil3.dylib",
        "libsmime3.dylib",
        "libsoftokn3.dylib",
        "libssl3.dylib",
        "odfsig",
    ]
    for bin in bins:
        shutil.copyfile("workdir/bin/" + bin, instdir + "/" + bin)
    nsprdylibs = [
        "libnspr4.dylib",
        "libplc4.dylib",
        "libplds4.dylib",
    ]
    for dylib in nsprdylibs:
        shutil.copyfile("workdir/bin/out/" + dylib, instdir + "/out/" + dylib)

    with zipfile.ZipFile(instdir + ".zip", "w") as zip:
        for root, dirs, files in os.walk("odfsig"):
            for name in files:
                zip.write(os.path.join(root, name))

if __name__ == '__main__':
    if platform.system() == "Darwin":
        mainDarwin()
    else:
        main()

# vim:set filetype=python shiftwidth=4 softtabstop=4 expandtab:
