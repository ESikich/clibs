<!--
docs/cl_time.md
Purpose: Time helper library documentation.
POSIX target: POSIX.1-2008 compatible C99.
Date modified: 2026-06-17.
-->

# cl_time

`cl_time` provides monotonic timestamps, checked duration math, and a simple
elapsed timer. Public values use signed 64-bit nanoseconds so callers do not need
to depend on `struct timespec`.

## Interface

```c
typedef enum cl_time_status {
    CL_TIME_OK = 0,
    CL_TIME_ERROR = 1,
    CL_TIME_OVERFLOW = 2
} cl_time_status;

typedef struct cl_duration {
    int64_t ns;
} cl_duration;

typedef struct cl_time_point {
    int64_t ns;
} cl_time_point;

typedef struct cl_timer {
    cl_time_point started_at;
} cl_timer;
```

Duration helpers:

```c
cl_duration cl_duration_from_ns(int64_t ns);
bool cl_duration_from_us(int64_t us, cl_duration *out);
bool cl_duration_from_ms(int64_t ms, cl_duration *out);
bool cl_duration_from_s(int64_t s, cl_duration *out);
int64_t cl_duration_as_ns(cl_duration duration);
double cl_duration_as_seconds(cl_duration duration);
int cl_duration_compare(cl_duration a, cl_duration b);
bool cl_duration_add(cl_duration a, cl_duration b, cl_duration *out);
bool cl_duration_sub(cl_duration a, cl_duration b, cl_duration *out);
```

Monotonic time and timers:

```c
cl_time_status cl_time_monotonic_now(cl_time_point *out);
bool cl_time_point_add(cl_time_point point, cl_duration duration, cl_time_point *out);
bool cl_time_point_sub(cl_time_point point, cl_duration duration, cl_time_point *out);
bool cl_time_point_duration_since(
    cl_time_point later,
    cl_time_point earlier,
    cl_duration *out);
int cl_time_point_compare(cl_time_point a, cl_time_point b);
cl_time_status cl_timer_start(cl_timer *timer);
cl_time_status cl_timer_elapsed(const cl_timer *timer, cl_duration *out);
```

## Contracts

All helpers that write through an output pointer require that pointer to be
non-null. Duration conversion and arithmetic helpers return `false` on overflow
or invalid output pointers. Time-reading helpers return `CL_TIME_ERROR` for
invalid output pointers or `clock_gettime` failures, and `CL_TIME_OVERFLOW` if a
system timestamp cannot fit in signed 64-bit nanoseconds.

`cl_time_monotonic_now` returns an opaque monotonic timestamp suitable for
elapsed-time measurement. It is not a wall-clock time and must not be serialized
as an absolute date.

`cl_timer_start` stores the current monotonic timestamp. `cl_timer_elapsed`
reads the clock again and returns the duration since that start point.

## Portability

The implementation uses POSIX.1-2008 `clock_gettime(CLOCK_MONOTONIC, ...)`.
The default project build defines `_POSIX_C_SOURCE=200809L`, which exposes that
API on POSIX-compatible systems.

The representation range is roughly +/-292 years at nanosecond precision.
Arithmetic is checked before writing results, so overflow is reported rather
than wrapped.
