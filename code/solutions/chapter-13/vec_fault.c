// vec_fault.c
#include <stdio.h>
#include <stdlib.h>

static int g_fail_at = -1;                                  // thất bại ở lần gọi thứ g_fail_at (-1: không)
static void *test_realloc(void *p, size_t n) {
    if (g_fail_at == 0) return NULL;
    if (g_fail_at > 0) g_fail_at--;
    return realloc(p, n);
}

typedef struct { int *data; size_t size, cap; } Vec;

static int vec_push(Vec *v, int x) {
    if (v->size == v->cap) {
        size_t nc = v->cap ? v->cap * 2 : 4;
        int *t = test_realloc(v->data, nc * sizeof *t);
        if (!t) return -1;                                  // v->data còn nguyên: không rò rỉ, không hỏng
        v->data = t; v->cap = nc;
    }
    v->data[v->size++] = x;
    return 0;
}

int main(void) {
    for (int k = 0; k < 6; k++) {                           // thử làm lỗi ở lần realloc thứ k
        Vec v = {0};
        g_fail_at = k;
        int failed = 0;
        for (int i = 0; i < 100; i++) if (vec_push(&v, i) != 0) { failed = 1; break; }
        printf("k=%d: %s, size=%zu\n", k, failed ? "lỗi được xử lý" : "không lỗi", v.size);
        free(v.data);                                        // dưới ASan: không được báo rò rỉ
    }
    return 0;
}
