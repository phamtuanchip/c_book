// pc_semaphore.c
#define _POSIX_C_SOURCE 200809L
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>

#define CAP 4
#define ITEMS 20

static int buf[CAP];
static int head, tail;
static sem_t empty_slots, full_slots;                       // đếm chỗ trống / phần tử có sẵn
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;    // bảo vệ head/tail

static void *producer(void *a) {
    (void)a;
    for (int i = 0; i < ITEMS; i++) {
        sem_wait(&empty_slots);                             // chờ có chỗ trống
        pthread_mutex_lock(&lock);
        buf[tail] = i; tail = (tail + 1) % CAP;
        pthread_mutex_unlock(&lock);
        sem_post(&full_slots);                              // báo có phần tử
    }
    return NULL;
}

static void *consumer(void *a) {
    long *sum = a;
    for (int i = 0; i < ITEMS; i++) {
        sem_wait(&full_slots);
        pthread_mutex_lock(&lock);
        int v = buf[head]; head = (head + 1) % CAP;
        pthread_mutex_unlock(&lock);
        sem_post(&empty_slots);
        *sum += v;
    }
    return NULL;
}

int main(void) {
    sem_init(&empty_slots, 0, CAP);
    sem_init(&full_slots, 0, 0);
    long sum = 0;
    pthread_t p, c;
    pthread_create(&p, NULL, producer, NULL);
    pthread_create(&c, NULL, consumer, &sum);
    pthread_join(p, NULL);
    pthread_join(c, NULL);
    printf("tong = %ld (mong doi %d)\n", sum, ITEMS * (ITEMS - 1) / 2);   // 190
    sem_destroy(&empty_slots);
    sem_destroy(&full_slots);
    return 0;
}
