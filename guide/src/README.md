# Introduction

odfsig is an Open Document Format (ODF) digital signatures tool: currently can verify already
created signatures.

The latest version is v25.2, released on 2025-02-02.  See the [release
notes](http://vmiklos.hu/odfsig/news.html).

## Description

odfsig verifies the digital signatures in an ODF document.

## Dependencies

### Build-time

- [cmake](https://cmake.org/)
- [googletest](https://github.com/google/googletest)

### Runtime

- [xmlsec1-nss or xmlsec1-mscng](https://www.aleksey.com/xmlsec/)
- [libzip](https://libzip.org/)

## Platforms

odfsig has been used on a variety of platforms:

- Linux
- Windows
- macOS

## Resources

- [ODF v1.3 digital signatures file spec](https://docs.oasis-open.org/office/OpenDocument/v1.3/os/part2-packages/OpenDocument-v1.3-os-part2-packages.html#__RefHeading__752871_826425813)

## License

Use of this source code is governed by a BSD-style license that can be found in
the LICENSE file.

## Download

From [GitHub](https://github.com/vmiklos/odfsig).
