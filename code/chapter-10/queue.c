#include <stdio.h>
#include <stdlib.h>

typedef struct QNode { int v; struct QNode *next; } QNode;

typedef struct Queue { QNode *head, *tail; } Queue;

void q_init(Queue *q) { q->head = q->tail = NULL; }
void q_push(Queue *q, int v) {
    QNode *n = malloc(sizeof(QNode)); if (!n) return; n->v = v; n->next = NULL;
    if (!q->tail) q->head = q->tail = n; else { q->tail->next = n; q->tail = n; }
}

int q_pop(Queue *q, int *out) {
    if (!q->head) return 0;
    QNode *n = q->head; *out = n->v; q->head = n->next; if (!q->head) q->tail = NULL; free(n); return 1;
}

void q_free(Queue *q) { int tmp; while (q_pop(q,&tmp)); }

int main(void) {
    Queue q; q_init(&q);
    q_push(&q, 1); q_push(&q, 2); q_push(&q, 3);
    int v;
    while (q_pop(&q, &v)) printf("pop: %d\n", v);
    q_free(&q);
    return 0;
}
