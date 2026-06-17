/*
 * cl_time.h
 * Purpose: Monotonic time, duration math, and elapsed timer helpers.
 * POSIX target: POSIX.1-2008 compatible C99.
 * Date modified: 2026-06-17.
 */

#ifndef CL_TIME_H
#define CL_TIME_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

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

cl_duration cl_duration_from_ns(int64_t ns);
bool cl_duration_from_us(int64_t us, cl_duration *out);
bool cl_duration_from_ms(int64_t ms, cl_duration *out);
bool cl_duration_from_s(int64_t s, cl_duration *out);

int64_t cl_duration_as_ns(cl_duration duration);
double cl_duration_as_seconds(cl_duration duration);

int cl_duration_compare(cl_duration a, cl_duration b);
bool cl_duration_add(cl_duration a, cl_duration b, cl_duration *out);
bool cl_duration_sub(cl_duration a, cl_duration b, cl_duration *out);

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

#ifdef __cplusplus
}
#endif

#endif
