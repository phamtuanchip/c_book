// ttl_test.c
#include <stdio.h>
#include <time.h>
#include "mini_test.h"

typedef struct { time_t (*now)(void); } Clock;
typedef struct { int value; time_t expires; int valid; } Entry;

static void entry_set(Entry *e, int v, time_t ttl, const Clock *c) { e->value = v; e->expires = c->now() + ttl; e->valid = 1; }
static int  entry_get(const Entry *e, const Clock *c, int *out) {
    if (!e->valid || c->now() >= e->expires) return -1;         // hết hạn khi now >= expires
    *out = e->value;
    return 0;
}

static time_t g_now;
static time_t fake_now(void) { return g_now; }

static void test_ttl(void) {
    Clock c = { fake_now };
    Entry e = {0};
    int v;
    g_now = 1000;
    entry_set(&e, 42, 60, &c);
    g_now = 1059; ASSERT_EQ_INT(entry_get(&e, &c, &v), 0);      // còn hạn
    g_now = 1060; ASSERT_EQ_INT(entry_get(&e, &c, &v), -1);     // đúng lúc hết hạn
    g_now = 5000; ASSERT_EQ_INT(entry_get(&e, &c, &v), -1);
}

int main(void) { RUN_TEST(test_ttl); TEST_MAIN_END(); }
