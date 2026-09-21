// define_stack.c
#include <stdio.h>
#include <stdlib.h>

#define DEFINE_STACK(T)                                                     \
    typedef struct { T *data; size_t size, cap; } Stack_##T;                \
    static int Stack_##T##_push(Stack_##T *s, T v) {                        \
        if (s->size == s->cap) {                                            \
            size_t nc = s->cap ? s->cap * 2 : 4;                            \
            T *t = realloc(s->data, nc * sizeof *t);                        \
            if (!t) return -1;                                              \
            s->data = t; s->cap = nc;                                       \
        }                                                                   \
        s->data[s->size++] = v;                                             \
        return 0;                                                           \
    }                                                                       \
    static int Stack_##T##_pop(Stack_##T *s, T *out) {                      \
        if (s->size == 0) return -1;                                        \
        *out = s->data[--s->size];                                          \
        return 0;                                                           \
    }

DEFINE_STACK(int)
DEFINE_STACK(double)

int main(void) {
    Stack_int si = {0};
    Stack_double sd = {0};
    Stack_int_push(&si, 7);
    Stack_double_push(&sd, 2.5);
    int a; double b;
    Stack_int_pop(&si, &a);
    Stack_double_pop(&sd, &b);
    printf("%d %.1f\n", a, b);
    free(si.data); free(sd.data);
    return 0;
}
