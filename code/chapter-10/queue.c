// queue.c
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct {
    int    *buf;
    size_t  cap;
    size_t  head;      // vị trí phần tử sẽ lấy ra
    size_t  count;     // số phần tử hiện có (dùng count để phân biệt đầy/rỗng)
} Queue;

bool queue_init(Queue *q, size_t cap) {
    q->buf = malloc(cap * sizeof *q->buf);
    if (!q->buf) return false;
    q->cap = cap; q->head = 0; q->count = 0;
    return true;
}

void queue_destroy(Queue *q) { free(q->buf); q->buf = NULL; }

bool queue_push(Queue *q, int v) {
    if (q->count == q->cap) return false;                    // đầy
    size_t tail = (q->head + q->count) % q->cap;
    q->buf[tail] = v;
    q->count++;
    return true;
}

bool queue_pop(Queue *q, int *out) {
    if (q->count == 0) return false;                          // rỗng
    *out = q->buf[q->head];
    q->head = (q->head + 1) % q->cap;
    q->count--;
    return true;
}

int main(void) {
    Queue q;
    if (!queue_init(&q, 4)) return 1;

    for (int i = 1; i <= 6; i++) {
        if (!queue_push(&q, i)) printf("queue day, bo qua %d\n", i);
    }
    int v;
    while (queue_pop(&q, &v)) printf("%d ", v);               // 1 2 3 4
    printf("\n");

    queue_destroy(&q);
    return 0;
}
