# Development notes

## Coding style

- Memory management: no manual delete. Use an `std::default_delete<>` template
  specialization when it comes to releasing resources with C library calls.

- No conditional compilation at a C preprocessor level (`#ifdef`). If something
  like that is needed, create interfaces, create multiple implementations of
  that interface and implement a factory function multiple times. Use the build
  system to ensure that only one of the factory implementations are compiled.

- Error handling: no exceptions. Return errors when dealing with user input, or
  use `assert()` to find bugs in the code.

- Naming: type names (classes, etc.) should start with an upper-case letter
  (e.g. ZipVerifier). Other names should be camel case, and start with a lower
  case letter (e.g. openZip()). Class members are prefixed with an underscore
  (`_`).

- Whitespace formatting: install the git hook in `git-hooks/` to let
  `clang-format` handle formatting for you.

## Checklist before release

Ideally CI checks everything before a commit hits master, but here are a few
things which are not part of CI:

- using system libraries: this works on latest stable openSUSE (Leap 15.3
  currently):

```
scripts/build.sh
```

NOTE: the lack of `--internal-libs` is the point of this exercise

- version check of libraries:

```
scripts/extern.py
```

- fuzzing:

```
workdir/bin/odfsigfuzz -max_len=16384 tests/data/
```

NOTE: This requires a `--fuzz` build.
