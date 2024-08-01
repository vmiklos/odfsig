# Changelog

## 24.8

- Maintenance release with up to date dependencies
- the macOS port is now deprecated, CI & release binary will be removed in the next release

## 7.3

- Update bundled externals, including nss to fix
  <https://www.mozilla.org/en-US/security/advisories/mfsa2021-51/>.

## 7.2

- Fix a missing include
- Update bundled externals
- Use `IMPORTED_LOCATION_<CONFIG>` instead of patching for the zlib and googletest externals

## 7.1

- Link bundled nss statically, which provides a self-contained executable on Linux and macOS

## 7.0

- Update bundled googletest, libxml2, libxmlsec, libzip, nss and xmlsec to latest versions

## 6.0

- Don't crash on directory argument

## 5.0

- Can validate multiple files in one go

## 4.0

- Initial XAdES certificate hash verify support
- Automatic packaging on all platforms
- Fix a crash found with fuzzing

## 3.0

- Runs on macOS
- Don't silently ignore unrecognized arguments

## 2.0

- First release with multiple backends (MS CNG next to NSS)
- Runs on Windows

## 1.0

- Initial release
- Runs on Linux
- Can detect digest mismatches in signatures
- Can detect certificate validation problems
