// vec.c
#include <stdio.h>
#include <stdlib.h>

typedef struct {
    int    *data;
    size_t  size;      // số phần tử đang dùng
    size_t  cap;       // sức chứa hiện có
} Vec;

int vec_init(Vec *v) {
    v->data = NULL;
    v->size = 0;
    v->cap  = 0;
    return 0;
}

void vec_destroy(Vec *v) {
    free(v->data);
    v->data = NULL;
    v->size = v->cap = 0;
}

int vec_push(Vec *v, int value) {
    if (v->size == v->cap) {                        // hết chỗ -> tăng gấp đôi
        size_t new_cap = v->cap ? v->cap * 2 : 4;
        int *tmp = realloc(v->data, new_cap * sizeof *tmp);
        if (tmp == NULL) return -1;                 // v->data còn nguyên, không rò rỉ
        v->data = tmp;
        v->cap  = new_cap;
    }
    v->data[v->size++] = value;
    return 0;
}

int main(void) {
    Vec v;
    vec_init(&v);
    for (int i = 0; i < 1000; i++) {
        if (vec_push(&v, i * i) != 0) { vec_destroy(&v); return 1; }
    }
    printf("size = %zu, cap = %zu, last = %d\n", v.size, v.cap, v.data[v.size - 1]);
    vec_destroy(&v);
    return 0;
}
