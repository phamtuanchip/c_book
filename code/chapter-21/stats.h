// stats.h
#include <stddef.h>
typedef struct { size_t count; double mean, min, max; } Stats;

// Trả 0 nếu tính được, -1 nếu n == 0 hoặc data == NULL.
int stats_compute(const int *data, size_t n, Stats *out);
