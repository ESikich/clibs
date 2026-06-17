/*
 * cl_time.c
 * Purpose: Implement monotonic time, duration math, and timer helpers.
 * POSIX target: POSIX.1-2008 compatible C99.
 * Date modified: 2026-06-17.
 */

#include "cl_time.h"

#include <stdint.h>
#include <time.h>

static bool cl_i64_add(int64_t a, int64_t b, int64_t *out)
{
    if ((b > 0 && a > INT64_MAX - b) ||
        (b < 0 && a < INT64_MIN - b)) {
        return false;
    }

    *out = a + b;
    return true;
}

static bool cl_i64_sub(int64_t a, int64_t b, int64_t *out)
{
    if ((b < 0 && a > INT64_MAX + b) ||
        (b > 0 && a < INT64_MIN + b)) {
        return false;
    }

    *out = a - b;
    return true;
}

static bool cl_i64_mul_positive(int64_t value, int64_t factor, int64_t *out)
{
    if ((value > 0 && value > INT64_MAX / factor) ||
        (value < 0 && value < INT64_MIN / factor)) {
        return false;
    }

    *out = value * factor;
    return true;
}

static cl_time_status cl_timespec_to_time_point(
    const struct timespec *time,
    cl_time_point *out)
{
    int64_t seconds_ns;
    int64_t ns;

    if (!time || !out) {
        return CL_TIME_ERROR;
    }

    if (!cl_i64_mul_positive((int64_t)time->tv_sec, INT64_C(1000000000), &seconds_ns) ||
        !cl_i64_add(seconds_ns, (int64_t)time->tv_nsec, &ns)) {
        return CL_TIME_OVERFLOW;
    }

    out->ns = ns;
    return CL_TIME_OK;
}

cl_duration cl_duration_from_ns(int64_t ns)
{
    cl_duration duration;

    duration.ns = ns;
    return duration;
}

bool cl_duration_from_us(int64_t us, cl_duration *out)
{
    int64_t ns;

    if (!out || !cl_i64_mul_positive(us, INT64_C(1000), &ns)) {
        return false;
    }

    out->ns = ns;
    return true;
}

bool cl_duration_from_ms(int64_t ms, cl_duration *out)
{
    int64_t ns;

    if (!out || !cl_i64_mul_positive(ms, INT64_C(1000000), &ns)) {
        return false;
    }

    out->ns = ns;
    return true;
}

bool cl_duration_from_s(int64_t s, cl_duration *out)
{
    int64_t ns;

    if (!out || !cl_i64_mul_positive(s, INT64_C(1000000000), &ns)) {
        return false;
    }

    out->ns = ns;
    return true;
}

int64_t cl_duration_as_ns(cl_duration duration)
{
    return duration.ns;
}

double cl_duration_as_seconds(cl_duration duration)
{
    return (double)duration.ns / 1000000000.0;
}

int cl_duration_compare(cl_duration a, cl_duration b)
{
    if (a.ns < b.ns) {
        return -1;
    }
    if (a.ns > b.ns) {
        return 1;
    }
    return 0;
}

bool cl_duration_add(cl_duration a, cl_duration b, cl_duration *out)
{
    int64_t ns;

    if (!out || !cl_i64_add(a.ns, b.ns, &ns)) {
        return false;
    }

    out->ns = ns;
    return true;
}

bool cl_duration_sub(cl_duration a, cl_duration b, cl_duration *out)
{
    int64_t ns;

    if (!out || !cl_i64_sub(a.ns, b.ns, &ns)) {
        return false;
    }

    out->ns = ns;
    return true;
}

cl_time_status cl_time_monotonic_now(cl_time_point *out)
{
    struct timespec time;

    if (!out) {
        return CL_TIME_ERROR;
    }
    if (clock_gettime(CLOCK_MONOTONIC, &time) != 0) {
        return CL_TIME_ERROR;
    }

    return cl_timespec_to_time_point(&time, out);
}

bool cl_time_point_add(cl_time_point point, cl_duration duration, cl_time_point *out)
{
    int64_t ns;

    if (!out || !cl_i64_add(point.ns, duration.ns, &ns)) {
        return false;
    }

    out->ns = ns;
    return true;
}

bool cl_time_point_sub(cl_time_point point, cl_duration duration, cl_time_point *out)
{
    int64_t ns;

    if (!out || !cl_i64_sub(point.ns, duration.ns, &ns)) {
        return false;
    }

    out->ns = ns;
    return true;
}

bool cl_time_point_duration_since(
    cl_time_point later,
    cl_time_point earlier,
    cl_duration *out)
{
    int64_t ns;

    if (!out || !cl_i64_sub(later.ns, earlier.ns, &ns)) {
        return false;
    }

    out->ns = ns;
    return true;
}

int cl_time_point_compare(cl_time_point a, cl_time_point b)
{
    if (a.ns < b.ns) {
        return -1;
    }
    if (a.ns > b.ns) {
        return 1;
    }
    return 0;
}

cl_time_status cl_timer_start(cl_timer *timer)
{
    if (!timer) {
        return CL_TIME_ERROR;
    }

    return cl_time_monotonic_now(&timer->started_at);
}

cl_time_status cl_timer_elapsed(const cl_timer *timer, cl_duration *out)
{
    cl_time_point now;
    cl_time_status status;

    if (!timer || !out) {
        return CL_TIME_ERROR;
    }

    status = cl_time_monotonic_now(&now);
    if (status != CL_TIME_OK) {
        return status;
    }
    if (!cl_time_point_duration_since(now, timer->started_at, out)) {
        return CL_TIME_OVERFLOW;
    }

    return CL_TIME_OK;
}
