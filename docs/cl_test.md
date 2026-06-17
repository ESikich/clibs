<!--
cl_test.md
Purpose: Unit test helper documentation.
POSIX target: POSIX.1-2008 compatible C99.
Date modified: 2026-06-17.
-->

# cl_test

`cl_test` is a tiny header-only unit test helper for repository tests. It keeps
test programs explicit C99 while avoiding one-off `CHECK` macros and manual
suite runners in each test file.

## Interface

Test functions return `0` on success and non-zero on failure:

```c
static int test_example(void)
{
    CL_TEST_CHECK(1 + 1 == 2);
    return 0;
}
```

Suites declare an array of `cl_test_case` values and pass it to
`cl_test_run_all`:

```c
int main(void)
{
    static const cl_test_case tests[] = {
        CL_TEST_CASE(test_example)
    };

    return cl_test_run_all("example", tests, sizeof(tests) / sizeof(tests[0]));
}
```

`CL_TEST_CHECK` reports the source file, line, and failed expression before
returning from the current test. The runner reports a compact pass/fail summary
and returns a process status suitable for `make test`.

## Scope

The helper intentionally does not allocate memory, install signal handlers,
fork, or depend on non-POSIX APIs. It is meant for small deterministic unit
tests; broader integration tests can still use it as their process-level
runner.
