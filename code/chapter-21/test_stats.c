// test_stats.c
#include "mini_test.h"
#include "stats.h"

static void test_compute_basic(void) {
    int d[] = {1, 2, 3, 4, 5};
    Stats s;
    ASSERT_EQ_INT(stats_compute(d, 5, &s), 0);
    ASSERT_NEAR(s.mean, 3.0, 1e-9);
    ASSERT_EQ_INT((long long)s.min, 1);
    ASSERT_EQ_INT((long long)s.max, 5);
}

static void test_compute_single(void) {
    int d[] = {42};
    Stats s;
    ASSERT_EQ_INT(stats_compute(d, 1, &s), 0);
    ASSERT_NEAR(s.mean, 42.0, 1e-9);
}

static void test_compute_negative(void) {
    int d[] = {-5, -1, -10};
    Stats s;
    ASSERT_EQ_INT(stats_compute(d, 3, &s), 0);
    ASSERT_EQ_INT((long long)s.min, -10);
    ASSERT_EQ_INT((long long)s.max, -1);
}

static void test_compute_rejects_empty(void) {
    Stats s;
    ASSERT_EQ_INT(stats_compute(NULL, 0, &s), -1);
    int d[] = {1};
    ASSERT_EQ_INT(stats_compute(d, 0, &s), -1);    // n == 0
}

int main(void) {
    RUN_TEST(test_compute_basic);
    RUN_TEST(test_compute_single);
    RUN_TEST(test_compute_negative);
    RUN_TEST(test_compute_rejects_empty);
    TEST_MAIN_END();
}
