# odfsig
Open Document Format (ODF) digital signatures tool

## Description

odfsig verifies the digital signatures in an ODF document. It is more or less
the cmdline equivalent of the LibreOffice signature verifier GUI.

## Dependencies

### Build-time

- [cmake](https://cmake.org/)
- [googletest](https://github.com/google/googletest)

### Runtime

- [xmlsec1-nss](https://www.aleksey.com/xmlsec/) (implies
  [libxml-2.0](http://xmlsoft.org/) and
  [nss](https://developer.mozilla.org/en-US/docs/Mozilla/Projects/NSS))
- [libzip](https://libzip.org/)

## License

Use of this source code is governed by a BSD-style license that can be found in
the LICENSE file.
