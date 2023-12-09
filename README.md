# odfsig

[![tests](https://github.com/vmiklos/odfsig/workflows/tests/badge.svg)](https://github.com/vmiklos/odfsig/actions")

Open Document Format (ODF) digital signatures tool: currently can verify already created signatures.

The latest version is v7.3, released on 2022-02-01.  See the
[release notes](https://github.com/vmiklos/odfsig/blob/master/NEWS.md).

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
