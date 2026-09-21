// test_stats_unity.c
#include "unity.h"
#include "stats.h"

void setUp(void)    { /* chạy TRƯỚC mỗi test */ }
void tearDown(void) { /* chạy SAU mỗi test */ }

void test_mean_of_five(void) {
    int d[] = {1, 2, 3, 4, 5};
    Stats s;
    TEST_ASSERT_EQUAL_INT(0, stats_compute(d, 5, &s));
    TEST_ASSERT_EQUAL_DOUBLE(3.0, s.mean);
    TEST_ASSERT_EQUAL_INT(1, (int)s.min);
}

void test_empty_input_fails(void) {
    Stats s;
    TEST_ASSERT_EQUAL_INT(-1, stats_compute(NULL, 0, &s));
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_mean_of_five);
    RUN_TEST(test_empty_input_fails);
    return UNITY_END();
}
