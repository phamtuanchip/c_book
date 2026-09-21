// stack.c
#include <stdio.h>
#include <stdlib.h>

typedef struct {
    int    *data;
    size_t  size, cap;
} Stack;

int stack_init(Stack *s) {
    s->data = NULL; s->size = 0; s->cap = 0;
    return 0;
}

void stack_destroy(Stack *s) {
    free(s->data);
    s->data = NULL; s->size = s->cap = 0;
}

int stack_push(Stack *s, int v) {
    if (s->size == s->cap) {
        size_t nc = s->cap ? s->cap * 2 : 8;
        int *t = realloc(s->data, nc * sizeof *t);
        if (!t) return -1;
        s->data = t; s->cap = nc;
    }
    s->data[s->size++] = v;
    return 0;
}

// Trả về 0 nếu thành công, -1 nếu stack rỗng
int stack_pop(Stack *s, int *out) {
    if (s->size == 0) return -1;
    *out = s->data[--s->size];
    return 0;
}

int main(void) {
    Stack s;
    stack_init(&s);
    for (int i = 1; i <= 5; i++) stack_push(&s, i * 10);

    int v;
    while (stack_pop(&s, &v) == 0) printf("%d ", v);    // 50 40 30 20 10
    printf("\n");
    stack_destroy(&s);
    return 0;
}
