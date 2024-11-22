#!/usr/bin/env python3
#
# Copyright 2018 Miklos Vajna
#
# SPDX-License-Identifier: MIT
#
# Script that checks if newer versions of externals are available.
#

import json
import os
import sys
import urllib.request

releaseMonitoring = {
    "googletest": ["googletest", "https://google.github.io/googletest/"],
    "libxml2": ["libxml2", "http://xmlsoft.org"],
    "libzip": ["libzip", "https://libzip.org"],
    "xmlsec": ["xmlsec1", "http://www.aleksey.com/xmlsec/download.html"],
    "zlib": ["zlib", "https://www.zlib.net"],
    "nss": ["nss", "https://ftp.mozilla.org/pub/security/nss/releases/"],
}


def getActualVersion(name):
    name, homepage = releaseMonitoring[name]
    url = "https://release-monitoring.org/api/v2/projects/?name=" + name
    try:
        sock = urllib.request.urlopen(url)
    except urllib.error.HTTPError as http_error:
        print("Failed to open URL %s: %s" % (url, http_error))
        sys.exit(1)
    buf = sock.read()
    sock.close()
    doc = json.loads(buf.decode("utf-8"))
    homepages = [i for i in doc["items"] if i["homepage"] == homepage]
    if not len(homepages):
        print("WARNING: '%s' contains no homepage '%s'" % (url, homepage))
        return
    project = homepages[0]
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
    if not actualVersion:
        return False

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
