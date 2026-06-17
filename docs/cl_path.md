<!--
docs/cl_path.md
Purpose: Lexical POSIX path helper documentation.
POSIX target: POSIX.1-2008 compatible C99.
Date modified: 2026-06-17.
-->

# cl_path

`cl_path` provides small lexical helpers for POSIX-style slash-separated paths.
The helpers do not query the filesystem and do not resolve symbolic links.

## Example

```c
char path[128];
cl_path_view name;

if (cl_path_join("/tmp/project", "../out.txt", path, sizeof(path), NULL)) {
    name = cl_path_basename(path);
    (void)name;
}
```

## Contracts

- `cl_path_normalize` collapses repeated slashes, removes `.` components, and
  handles `..` lexically.
- Absolute paths stay absolute. Leading `..` components above root are discarded
  for absolute paths and preserved for relative paths.
- Empty normalized input becomes `"."`.
- `cl_path_join` treats an absolute child path as replacing the base path. A
  relative child path is appended to the base with one slash before
  normalization.
- Output buffers must have room for the resulting bytes plus a NUL terminator.
  On success, `*written` receives the byte count excluding the terminator.
- `cl_path_basename` and `cl_path_dirname` return non-owning views. For null or
  empty input, they return `"."`. For root input, they return `"/"`.

## Safety Properties

- All owning storage remains with the caller. The helpers do not allocate.
- Size checks include the NUL terminator before writing output bytes.
- `basename` and `dirname` views point either into the caller-provided path or to
  static string literals.

## Portability

The implementation targets POSIX.1-2008 compatible C99 and performs lexical
slash-separated path handling only. It intentionally avoids `realpath`,
`basename`, and `dirname` because those APIs either touch filesystem state or
may mutate caller-provided buffers.
