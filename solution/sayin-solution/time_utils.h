#ifndef HW1_TIME_UTILS_
#define HW1_TIME_UTILS_

#include <time.h>
#include <stdlib.h>

// A small set of quick inline utility functions to work with time

static void add_ms_to_timespec(unsigned long msec, struct timespec *ts)
{
    lldiv_t d = lldiv((long long) msec, 1000);
    ts->tv_sec += d.quot;
    ts->tv_nsec += d.rem * 1000000;

    // tv_nsec may have overflowed, handle that
    d = lldiv((long long) ts->tv_nsec, 1000000000LL);
    ts->tv_sec += d.quot;
    ts->tv_nsec = d.rem;
}

static void calc_delayed_abstime(unsigned long sleep_msec, struct timespec *out)
{
    // Get the current time and 
    clock_gettime(CLOCK_REALTIME, out);
    add_ms_to_timespec(sleep_msec, out);
}

#endif
