// roundtrip_test.c
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include "mini_test.h"

static int parse_int(const char *s, int *out) {
    char *end;
    long v = strtol(s, &end, 10);
    if (end == s || *end != '\0' || v < INT_MIN || v > INT_MAX) return -1;
    *out = (int)v;
    return 0;
}

static void test_roundtrip(void) {
    srand(1234);                                                   // seed cố định: tái hiện được
    int samples[] = { 0, 1, -1, INT_MAX, INT_MIN };
    for (int iter = 0; iter < 2000; iter++) {
        int x = iter < 5 ? samples[iter] : (int)(((unsigned)rand() << 16) ^ (unsigned)rand());
        char buf[32];
        snprintf(buf, sizeof buf, "%d", x);
        int y = 0;
        ASSERT_EQ_INT(parse_int(buf, &y), 0);
        ASSERT_EQ_INT(y, x);
    }
}

int main(void) { RUN_TEST(test_roundtrip); TEST_MAIN_END(); }
