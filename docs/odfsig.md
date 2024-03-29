% odfsig(1)

# NAME

odfsig - Open Document Format (ODF) digital signatures tool

# SYNOPSIS

odfsig [options] <ODF-file>...

# DESCRIPTION

odfsig verifies the digital signatures in an ODF document.

The signing certificate validation uses the trusted certificates stored in the
following locations:

- The NSS Certificate database in the default Firefox profile.

- Additional certificate chains, provided using the `--trusted-der` option.

# OPTIONS

--help

: Display this manpage.

--insecure

: Disable certificate verification, only focus on digest mismatches.

--trusted-der <file>

: Load trusted (root) certificate (chain) from a DER file.

# EXIT STATUS

The exit status is 0 if all signatures of all files are valid, 1 if at least
one signature is invalid, and 2 if an error occurred.

# LICENSE

Use of this source code is governed by a BSD-style license that can be found in
the LICENSE file.

# REPORTING BUGS

Patches welcome via <https://github.com/vmiklos/odfsig>.

# SEE ALSO

**pdfsig(1)**
