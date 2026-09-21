#ifndef MIN_H
#define MIN_H

/* Inline min functions for different integer types */
static inline int min_int(int a, int b) { return a < b ? a : b; }
static inline long min_long(long a, long b) { return a < b ? a : b; }

#endif /* MIN_H */
