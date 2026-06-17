/*
 * test_time.c
 * Purpose: Safety and behavior tests for cl_time.
 * POSIX target: POSIX.1-2008 compatible C99.
 * Date modified: 2026-06-17.
 */

#include "cl_test.h"
#include "cl_time.h"

#include <stdint.h>

static int test_duration_conversions(void)
{
    cl_duration duration;

    duration = cl_duration_from_ns(INT64_C(123));
    CL_TEST_CHECK(cl_duration_as_ns(duration) == INT64_C(123));

    CL_TEST_CHECK(cl_duration_from_us(INT64_C(2), &duration));
    CL_TEST_CHECK(cl_duration_as_ns(duration) == INT64_C(2000));

    CL_TEST_CHECK(cl_duration_from_ms(INT64_C(3), &duration));
    CL_TEST_CHECK(cl_duration_as_ns(duration) == INT64_C(3000000));

    CL_TEST_CHECK(cl_duration_from_s(INT64_C(4), &duration));
    CL_TEST_CHECK(cl_duration_as_ns(duration) == INT64_C(4000000000));
    CL_TEST_CHECK(cl_duration_as_seconds(duration) == 4.0);

    CL_TEST_CHECK(!cl_duration_from_s(INT64_MAX, &duration));
    CL_TEST_CHECK(!cl_duration_from_ms(INT64_MIN, &duration));
    return 0;
}

static int test_duration_math(void)
{
    cl_duration a = cl_duration_from_ns(INT64_C(10));
    cl_duration b = cl_duration_from_ns(INT64_C(3));
    cl_duration result;

    CL_TEST_CHECK(cl_duration_compare(a, b) > 0);
    CL_TEST_CHECK(cl_duration_compare(b, a) < 0);
    CL_TEST_CHECK(cl_duration_compare(a, cl_duration_from_ns(INT64_C(10))) == 0);

    CL_TEST_CHECK(cl_duration_add(a, b, &result));
    CL_TEST_CHECK(cl_duration_as_ns(result) == INT64_C(13));
    CL_TEST_CHECK(cl_duration_sub(a, b, &result));
    CL_TEST_CHECK(cl_duration_as_ns(result) == INT64_C(7));

    CL_TEST_CHECK(!cl_duration_add(
        cl_duration_from_ns(INT64_MAX),
        cl_duration_from_ns(INT64_C(1)),
        &result));
    CL_TEST_CHECK(!cl_duration_sub(
        cl_duration_from_ns(INT64_MIN),
        cl_duration_from_ns(INT64_C(1)),
        &result));
    return 0;
}

static int test_time_point_math(void)
{
    cl_time_point point;
    cl_time_point moved;
    cl_duration duration;

    point.ns = INT64_C(100);
    duration = cl_duration_from_ns(INT64_C(25));

    CL_TEST_CHECK(cl_time_point_add(point, duration, &moved));
    CL_TEST_CHECK(moved.ns == INT64_C(125));
    CL_TEST_CHECK(cl_time_point_sub(moved, duration, &point));
    CL_TEST_CHECK(point.ns == INT64_C(100));
    CL_TEST_CHECK(cl_time_point_duration_since(moved, point, &duration));
    CL_TEST_CHECK(cl_duration_as_ns(duration) == INT64_C(25));
    CL_TEST_CHECK(cl_time_point_compare(moved, point) > 0);

    point.ns = INT64_MAX;
    CL_TEST_CHECK(!cl_time_point_add(
        point,
        cl_duration_from_ns(INT64_C(1)),
        &moved));
    return 0;
}

static int test_monotonic_now_is_ordered(void)
{
    cl_time_point first;
    cl_time_point second;

    CL_TEST_CHECK(cl_time_monotonic_now(&first) == CL_TIME_OK);
    CL_TEST_CHECK(cl_time_monotonic_now(&second) == CL_TIME_OK);
    CL_TEST_CHECK(cl_time_point_compare(second, first) >= 0);
    CL_TEST_CHECK(cl_time_monotonic_now(NULL) == CL_TIME_ERROR);
    return 0;
}

static int test_timer_elapsed_is_nonnegative(void)
{
    cl_timer timer;
    cl_duration elapsed;

    CL_TEST_CHECK(cl_timer_start(&timer) == CL_TIME_OK);
    CL_TEST_CHECK(cl_timer_elapsed(&timer, &elapsed) == CL_TIME_OK);
    CL_TEST_CHECK(cl_duration_as_ns(elapsed) >= 0);
    CL_TEST_CHECK(cl_timer_start(NULL) == CL_TIME_ERROR);
    CL_TEST_CHECK(cl_timer_elapsed(NULL, &elapsed) == CL_TIME_ERROR);
    CL_TEST_CHECK(cl_timer_elapsed(&timer, NULL) == CL_TIME_ERROR);
    return 0;
}

int main(void)
{
    const cl_test_case cases[] = {
        CL_TEST_CASE(test_duration_conversions),
        CL_TEST_CASE(test_duration_math),
        CL_TEST_CASE(test_time_point_math),
        CL_TEST_CASE(test_monotonic_now_is_ordered),
        CL_TEST_CASE(test_timer_elapsed_is_nonnegative)
    };

    return cl_test_run_all("cl_time", cases, sizeof(cases) / sizeof(cases[0]));
}
