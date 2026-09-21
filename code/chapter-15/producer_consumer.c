// producer_consumer.c
#define _POSIX_C_SOURCE 200809L
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define CAP 8
#define NPRODUCERS 2
#define NCONSUMERS 3
#define ITEMS_PER_PRODUCER 10

typedef struct {
    int buf[CAP];
    size_t head, count;
    int closed;                       // producer báo "không còn dữ liệu nữa"
    pthread_mutex_t lock;
    pthread_cond_t  not_full, not_empty;
} BoundedQueue;

static void bq_init(BoundedQueue *q) {
    q->head = q->count = 0;
    q->closed = 0;
    pthread_mutex_init(&q->lock, NULL);
    pthread_cond_init(&q->not_full, NULL);
    pthread_cond_init(&q->not_empty, NULL);
}

static void bq_destroy(BoundedQueue *q) {
    pthread_mutex_destroy(&q->lock);
    pthread_cond_destroy(&q->not_full);
    pthread_cond_destroy(&q->not_empty);
}

// Thêm phần tử; chờ nếu đầy
static void bq_put(BoundedQueue *q, int v) {
    pthread_mutex_lock(&q->lock);
    while (q->count == CAP)                          // đầy -> chờ
        pthread_cond_wait(&q->not_full, &q->lock);
    q->buf[(q->head + q->count) % CAP] = v;
    q->count++;
    pthread_cond_signal(&q->not_empty);              // báo: hết rỗng
    pthread_mutex_unlock(&q->lock);
}

// Lấy phần tử; chờ nếu rỗng. Trả 1 nếu lấy được, 0 nếu hàng đợi đã đóng và hết dữ liệu.
static int bq_get(BoundedQueue *q, int *out) {
    pthread_mutex_lock(&q->lock);
    while (q->count == 0 && !q->closed)              // rỗng và còn khả năng có thêm -> chờ
        pthread_cond_wait(&q->not_empty, &q->lock);
    if (q->count == 0) {                             // rỗng và đã đóng -> hết
        pthread_mutex_unlock(&q->lock);
        return 0;
    }
    *out = q->buf[q->head];
    q->head = (q->head + 1) % CAP;
    q->count--;
    pthread_cond_signal(&q->not_full);               // báo: hết đầy
    pthread_mutex_unlock(&q->lock);
    return 1;
}

// Báo đóng: đánh thức tất cả consumer đang chờ để chúng thoát
static void bq_close(BoundedQueue *q) {
    pthread_mutex_lock(&q->lock);
    q->closed = 1;
    pthread_cond_broadcast(&q->not_empty);
    pthread_mutex_unlock(&q->lock);
}

typedef struct { BoundedQueue *q; int id; } ProducerArg;
typedef struct { BoundedQueue *q; int id; long sum; } ConsumerArg;

static void *producer(void *arg) {
    ProducerArg *a = arg;
    for (int i = 0; i < ITEMS_PER_PRODUCER; i++) {
        int v = a->id * 1000 + i;
        bq_put(a->q, v);
    }
    return NULL;
}

static void *consumer(void *arg) {
    ConsumerArg *a = arg;
    int v;
    while (bq_get(a->q, &v)) {
        a->sum += v;                               // sum là riêng của luồng này -> không cần khóa
    }
    return NULL;
}

int main(void) {
    BoundedQueue q;
    bq_init(&q);

    pthread_t prod[NPRODUCERS], cons[NCONSUMERS];
    ProducerArg pa[NPRODUCERS];
    ConsumerArg ca[NCONSUMERS];

    for (int i = 0; i < NCONSUMERS; i++) {
        ca[i] = (ConsumerArg){ &q, i, 0 };
        pthread_create(&cons[i], NULL, consumer, &ca[i]);
    }
    for (int i = 0; i < NPRODUCERS; i++) {
        pa[i] = (ProducerArg){ &q, i + 1 };
        pthread_create(&prod[i], NULL, producer, &pa[i]);
    }

    for (int i = 0; i < NPRODUCERS; i++) pthread_join(prod[i], NULL);   // chờ producer xong
    bq_close(&q);                                                       // rồi mới đóng queue
    for (int i = 0; i < NCONSUMERS; i++) pthread_join(cons[i], NULL);

    long total = 0;
    for (int i = 0; i < NCONSUMERS; i++) total += ca[i].sum;

    long expected = 0;
    for (int p = 1; p <= NPRODUCERS; p++)
        for (int i = 0; i < ITEMS_PER_PRODUCER; i++) expected += p * 1000 + i;

    printf("tong nhan duoc = %ld, mong doi = %ld -> %s\n", total, expected, total == expected ? "DUNG" : "SAI");
    bq_destroy(&q);
    return 0;
}
