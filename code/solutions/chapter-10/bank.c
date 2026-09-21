// bank.c
#include <stdio.h>

typedef struct { int id, service; } Customer;

#define CAP 16
typedef struct { Customer buf[CAP]; int head, count; } Queue;

static int push(Queue *q, Customer c) {
    if (q->count == CAP) return -1;
    q->buf[(q->head + q->count++) % CAP] = c;
    return 0;
}
static int pop(Queue *q, Customer *c) {
    if (!q->count) return -1;
    *c = q->buf[q->head];
    q->head = (q->head + 1) % CAP;
    q->count--;
    return 0;
}

int main(void) {
    Queue q = {0};
    int service[] = {3, 1, 4, 2};
    for (int i = 0; i < 4; i++) push(&q, (Customer){ i + 1, service[i] });

    int clock = 0;
    Customer c;
    while (pop(&q, &c) == 0) {
        printf("t=%2d: bat dau phuc vu khach %d (%d phut)\n", clock, c.id, c.service);
        clock += c.service;
    }
    printf("t=%2d: xong\n", clock);                            // 10
    return 0;
}
