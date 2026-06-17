<!--
docs/portability.md
Purpose: Portability policy for clibs.
POSIX target: POSIX.1-2008 compatible C99.
Date modified: 2026-06-17.
-->

# Portability

The project baseline is POSIX.1-2008 compatible C99.

## ANSI C Note

"ANSI C" commonly refers to the original C89/C90 standard. This project targets
ISO C99 instead. Code should stay close to portable C, but strict ANSI C/C89
compatibility is not a project goal.

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

## POSIX File I/O

`cl_file` uses POSIX.1-2008 `open`, `fstat`, `read`, `write`, and `close` for
whole-file and streaming helpers. File reads and writes retry interrupted
transfer calls, and write permissions are created as `0666` filtered by the
process umask.

## Lexical Paths

`cl_path` handles POSIX-style slash-separated paths lexically in caller-owned
buffers. It intentionally avoids `realpath`, `basename`, and `dirname` because
those APIs either touch filesystem state or may mutate caller-provided buffers.

## Locale-Free ASCII

`cl_ascii` intentionally avoids `<ctype.h>` so classification and case
conversion are independent of the current locale and well-defined for negative
values and bytes above `0x7f`.

## Bit Operations

`cl_bitset` uses portable `size_t` word operations and explicit loops for
population counts and bit scans. It intentionally avoids compiler-specific
popcount and bit-scan intrinsics so behavior stays within the C99 baseline.
