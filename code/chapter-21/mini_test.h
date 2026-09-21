// mini_test.h
#ifndef MINI_TEST_H
#define MINI_TEST_H
#include <math.h>
#include <stdio.h>
#include <string.h>

static int mt_checks = 0, mt_failed = 0, mt_current_failed = 0;

#define ASSERT_TRUE(cond) do {                                                   \
        mt_checks++;                                                             \
        if (!(cond)) {                                                           \
            mt_failed++; mt_current_failed = 1;                                  \
            fprintf(stderr, "  %s:%d: ASSERT_TRUE(%s) that bai\n", __FILE__, __LINE__, #cond); \
        }                                                                        \
    } while (0)

#define ASSERT_EQ_INT(actual, expected) do {                                     \
        long long a_ = (actual), e_ = (expected);                                \
        mt_checks++;                                                             \
        if (a_ != e_) {                                                          \
            mt_failed++; mt_current_failed = 1;                                  \
            fprintf(stderr, "  %s:%d: %s = %lld, mong doi %lld\n", __FILE__, __LINE__, #actual, a_, e_); \
        }                                                                        \
    } while (0)

#define ASSERT_EQ_STR(actual, expected) do {                                     \
        const char *a_ = (actual), *e_ = (expected);                             \
        mt_checks++;                                                             \
        if (a_ == NULL || e_ == NULL || strcmp(a_, e_) != 0) {                   \
            mt_failed++; mt_current_failed = 1;                                  \
            fprintf(stderr, "  %s:%d: %s = \"%s\", mong doi \"%s\"\n", __FILE__, __LINE__, #actual, a_ ? a_ : "(null)", e_ ? e_ : "(null)"); \
        }                                                                        \
    } while (0)

#define ASSERT_NEAR(actual, expected, eps) do {                                  \
        double a_ = (actual), e_ = (expected);                                   \
        mt_checks++;                                                             \
        if (fabs(a_ - e_) > (eps)) {                                             \
            mt_failed++; mt_current_failed = 1;                                  \
            fprintf(stderr, "  %s:%d: %s = %g, mong doi %g (+-%g)\n", __FILE__, __LINE__, #actual, a_, e_, (double)(eps)); \
        }                                                                        \
    } while (0)

// Chạy một test và in kết quả
#define RUN_TEST(fn) do {                                                        \
        mt_current_failed = 0;                                                   \
        fn();                                                                    \
        printf("[%s] %s\n", mt_current_failed ? "FAIL" : " OK ", #fn);           \
    } while (0)

#define TEST_MAIN_END() do {                                                     \
        printf("\n%d kiem tra, %d that bai\n", mt_checks, mt_failed);            \
        return mt_failed ? 1 : 0;                                                \
    } while (0)

#endif
