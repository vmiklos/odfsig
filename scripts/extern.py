#!/usr/bin/env python3
#
# Copyright 2018 Miklos Vajna. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#
# Script that checks if newer versions of externals are available.
#

import json
import os
import sys
import urllib.request

releaseMonitoring = {
    "googletest": ["googletest", "https://github.com/google/googletest"],
    "libxml2": ["libxml2", "http://xmlsoft.org/"],
    "libzip": ["libzip", "http://www.nih.at/libzip/"],
    "xmlsec": ["xmlsec1", "http://www.aleksey.com/xmlsec/download.html"],
    "zlib": ["zlib", "http://www.zlib.net/"],
}


def getActualVersion(name):
    name, homepage = releaseMonitoring[name]
    url = "https://release-monitoring.org/api/v2/projects/?name=" + name
    sock = urllib.request.urlopen(url)
    buf = sock.read()
    sock.close()
    doc = json.loads(buf.decode("utf-8"))
    project = [i for i in doc["items"] if i["homepage"] == homepage][0]
    return project["versions"][0]


def getExpectedVersion(name, cmakeLists):
    sock = open(cmakeLists)
    for line in sock.readlines():
        if not line.startswith("set(" + name.upper() + "_VERSION "):
            continue

        line = line.strip()
        # 'set(XMLSEC_VERSION 1.2.27)' -> '1.2.27'
        return line.split(' ')[1][:-1]


def checkVersion(name, cmakeLists):
    expectedVersion = getExpectedVersion(name, cmakeLists)
    sys.stdout.write("checking for " + name + "-" + expectedVersion + "...")
    sys.stdout.flush()

    actualVersion = getActualVersion(name)
    if actualVersion == expectedVersion:
        print(" up to date")
        return True
    else:
        print(" outdated, " + actualVersion + " is available")
        return False


def main():
    ret = True

    for extern in os.listdir("extern"):
        directory = os.path.join("extern", extern)
        cmakeLists = os.path.join(directory, "CMakeLists.txt")
        if not os.path.exists(cmakeLists):
            continue

        ret &= checkVersion(extern, cmakeLists)

    return ret


if __name__ == '__main__':
    if not main():
        sys.exit(1)
    else:
        sys.exit(0)

# vim:set filetype=python shiftwidth=4 softtabstop=4 expandtab:
