// vec_test.c
#include <stdio.h>
#include <stdlib.h>
#include "mini_test.h"

typedef struct { int *data; size_t size, cap; } Vec;

/* --- mã cần thử (thường nằm ở vec.c) --- */
static int g_fail_realloc_at = -1;
static void *my_realloc(void *p, size_t n) {           // bản giả: thất bại ở lần thứ k
    if (g_fail_realloc_at == 0) return NULL;
    if (g_fail_realloc_at > 0) g_fail_realloc_at--;
    return realloc(p, n);
}

static int vec_push(Vec *v, int x) {
    if (v->size == v->cap) {
        size_t nc = v->cap ? v->cap * 2 : 4;
        int *t = my_realloc(v->data, nc * sizeof *t);
        if (!t) return -1;
        v->data = t; v->cap = nc;
    }
    v->data[v->size++] = x;
    return 0;
}
static int vec_pop(Vec *v, int *out) { if (!v->size) return -1; *out = v->data[--v->size]; return 0; }

/* --- test --- */
static void test_push_many(void) {
    Vec v = {0};
    for (int i = 0; i < 1000; i++) ASSERT_EQ_INT(vec_push(&v, i), 0);
    ASSERT_EQ_INT((long long)v.size, 1000);
    ASSERT_TRUE(v.cap >= 1000);
    free(v.data);
}

static void test_pop_empty(void) {
    Vec v = {0};
    int x;
    ASSERT_EQ_INT(vec_pop(&v, &x), -1);
}

static void test_realloc_failure_is_clean(void) {
    for (int k = 0; k < 4; k++) {
        Vec v = {0};
        g_fail_realloc_at = k;
        int failed = 0;
        for (int i = 0; i < 100; i++) if (vec_push(&v, i) != 0) { failed = 1; break; }
        ASSERT_TRUE(failed);                          // đã thất bại đúng như giả lập
        ASSERT_TRUE(v.size <= v.cap);                 // trạng thái vẫn nhất quán
        free(v.data);                                 // ASan: không rò rỉ, không double free
    }
    g_fail_realloc_at = -1;
}

int main(void) {
    RUN_TEST(test_push_many);
    RUN_TEST(test_pop_empty);
    RUN_TEST(test_realloc_failure_is_clean);
    TEST_MAIN_END();
}
