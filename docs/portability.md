<!--
docs/portability.md
Purpose: Portability policy for clibs.
POSIX target: POSIX.1-2008 compatible C99.
Date modified: 2026-06-17.
-->

# Portability

The project baseline is POSIX.1-2008 compatible C99.

## Build Defaults

The default `Makefile` uses:

```make
CFLAGS = -std=c99 -Wall -Wextra -Wpedantic -Wconversion -Wshadow -O2
CPPFLAGS = -D_POSIX_C_SOURCE=200809L -Iinclude
```

## Rules

- Prefer ISO C99 and POSIX.1-2008 APIs.
- Keep feature-test macros visible in build files or at the top of implementation
  files before system headers.
- Document any platform-specific API before adding it.
- Provide a POSIX fallback for non-POSIX code before using it in a core library.
- Keep examples and tests within the same portability target as the library code.
