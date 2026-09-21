#include <stdio.h>
#include <stdlib.h>

typedef struct StackNode { int v; struct StackNode *next; } StackNode;

void push(StackNode **top, int v) {
    StackNode *n = malloc(sizeof(StackNode)); if (!n) return; n->v = v; n->next = *top; *top = n;
}

int pop(StackNode **top, int *out) {
    if (!*top) return 0;
    StackNode *n = *top; *out = n->v; *top = n->next; free(n); return 1;
}

void free_stack(StackNode *top) { while (top) { StackNode *t = top; top = top->next; free(t); } }

int main(void) {
    StackNode *s = NULL; int x;
    push(&s, 10); push(&s, 20); push(&s, 30);
    while (pop(&s, &x)) printf("pop: %d\n", x);
    free_stack(s);
    return 0;
}
