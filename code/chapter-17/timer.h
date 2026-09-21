// timer.h
#ifndef TIMER_H
#define TIMER_H

#define _POSIX_C_SOURCE 200809L
#include <time.h>

static inline double now_seconds(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + (double)ts.tv_nsec * 1e-9;
}

#endif
