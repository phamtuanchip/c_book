// test_math.c
#include "tiny_test.h"

static int add(int a, int b) { return a + b; }

int main(void) {
    CHECK(add(2, 3) == 5);
    CHECK_EQ_INT(add(-1, 1), 0);
    CHECK_EQ_INT(add(2, 2), 5);        // cố tình sai để xem thông báo
    return TEST_SUMMARY();
}
