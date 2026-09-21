// tiny_test.h
#ifndef TINY_TEST_H
#define TINY_TEST_H

#include <stdio.h>

static int tt_run = 0, tt_failed = 0;

#define CHECK(cond) do {                                                     \
        tt_run++;                                                            \
        if (!(cond)) {                                                       \
            tt_failed++;                                                     \
            fprintf(stderr, "%s:%d: CHECK(%s) that bai\n", __FILE__, __LINE__, #cond); \
        }                                                                    \
    } while (0)

#define CHECK_EQ_INT(actual, expected) do {                                  \
        long long _a = (actual), _e = (expected);                            \
        tt_run++;                                                            \
        if (_a != _e) {                                                      \
            tt_failed++;                                                     \
            fprintf(stderr, "%s:%d: %s = %lld, mong doi %lld\n",             \
                    __FILE__, __LINE__, #actual, _a, _e);                    \
        }                                                                    \
    } while (0)

#define TEST_SUMMARY() \
    (printf("%d kiem tra, %d that bai\n", tt_run, tt_failed), tt_failed ? 1 : 0)

#endif
