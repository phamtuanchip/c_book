// array_stats.c
#include <stddef.h>
#include <stdio.h>

static double average(const double *a, size_t n) {
    double s = 0;
    for (size_t i = 0; i < n; i++) s += a[i];
    return n ? s / (double)n : 0.0;
}

static void min_max(const double *a, size_t n, double *min, double *max) {   // tiền điều kiện: n >= 1
    *min = *max = a[0];
    for (size_t i = 1; i < n; i++) {
        if (a[i] < *min) *min = a[i];
        if (a[i] > *max) *max = a[i];
    }
}

static size_t count_above(const double *a, size_t n, double threshold) {
    size_t c = 0;
    for (size_t i = 0; i < n; i++) if (a[i] > threshold) c++;
    return c;
}

int main(void) {
    double a[] = {3.5, 1.0, 4.0, 1.5, 9.0};
    size_t n = sizeof a / sizeof a[0];
    double lo, hi, avg = average(a, n);
    min_max(a, n, &lo, &hi);
    printf("tb = %.2f, min = %.1f, max = %.1f, > tb: %zu\n", avg, lo, hi, count_above(a, n, avg));   // 3.80 1.0 9.0 2
    return 0;
}
