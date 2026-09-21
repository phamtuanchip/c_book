// vec_ext.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct { int *data; size_t size, cap; } Vec;

static void vec_init(Vec *v) { v->data = NULL; v->size = v->cap = 0; }
static void vec_destroy(Vec *v) { free(v->data); vec_init(v); }

static int vec_reserve(Vec *v, size_t need) {
    if (need <= v->cap) return 0;
    size_t nc = v->cap ? v->cap : 4;
    while (nc < need) nc *= 2;
    int *t = realloc(v->data, nc * sizeof *t);
    if (!t) return -1;
    v->data = t; v->cap = nc;
    return 0;
}

static int vec_push(Vec *v, int x) {
    if (vec_reserve(v, v->size + 1) != 0) return -1;
    v->data[v->size++] = x;
    return 0;
}

/* Trả 0 nếu lấy được, -1 nếu rỗng */
static int vec_pop(Vec *v, int *out) {
    if (v->size == 0) return -1;
    *out = v->data[--v->size];
    return 0;
}

static int vec_get(const Vec *v, size_t i, int *out) {
    if (i >= v->size) return -1;
    *out = v->data[i];
    return 0;
}

/* Chèn x vào vị trí index (0..size): dịch các phần tử sau sang phải */
static int vec_insert(Vec *v, size_t index, int x) {
    if (index > v->size) return -1;
    if (vec_reserve(v, v->size + 1) != 0) return -1;
    memmove(v->data + index + 1, v->data + index, (v->size - index) * sizeof *v->data);
    v->data[index] = x;
    v->size++;
    return 0;
}

static int vec_remove(Vec *v, size_t index) {
    if (index >= v->size) return -1;
    memmove(v->data + index, v->data + index + 1, (v->size - index - 1) * sizeof *v->data);
    v->size--;
    return 0;
}

int main(void) {
    Vec v;
    vec_init(&v);
    for (int i = 0; i < 5; i++) vec_push(&v, i * 10);        // 0 10 20 30 40
    vec_insert(&v, 2, 99);                                    // 0 10 99 20 30 40
    vec_remove(&v, 0);                                        // 10 99 20 30 40
    int x;
    vec_pop(&v, &x);                                          // x = 40
    for (size_t i = 0; i < v.size; i++) { vec_get(&v, i, &x); printf("%d ", x); }
    printf("\n");
    vec_destroy(&v);
    return 0;
}
