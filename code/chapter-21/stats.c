// stats.c
#include "stats.h"
int stats_compute(const int *data, size_t n, Stats *out) {
    if (!data || n == 0 || !out) return -1;
    long sum = 0;
    int mn = data[0], mx = data[0];
    for (size_t i = 0; i < n; i++) {
        sum += data[i];
        if (data[i] < mn) mn = data[i];
        if (data[i] > mx) mx = data[i];
    }
    out->count = n;
    out->mean  = (double)sum / (double)n;
    out->min   = mn;
    out->max   = mx;
    return 0;
}
