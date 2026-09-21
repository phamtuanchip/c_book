// restrict_demo.c
#include <stddef.h>

void add_norestrict(float *out, const float *a, const float *b, size_t n) {
    for (size_t i = 0; i < n; i++) out[i] = a[i] + b[i];
}

void add_restrict(float *restrict out, const float *restrict a, const float *restrict b, size_t n) {
    for (size_t i = 0; i < n; i++) out[i] = a[i] + b[i];
}
