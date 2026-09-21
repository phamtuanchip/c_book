#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define BUF_SIZE 10
int buffer[BUF_SIZE]; int count = 0; int in = 0, out = 0;
pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t not_empty = PTHREAD_COND_INITIALIZER;
pthread_cond_t not_full = PTHREAD_COND_INITIALIZER;

void* producer(void *arg) {
    int id = *(int*)arg;
    for (int i = 0; i < 20; ++i) {
        pthread_mutex_lock(&m);
        while (count == BUF_SIZE) pthread_cond_wait(&not_full, &m);
        buffer[in] = i + id*100;
        in = (in + 1) % BUF_SIZE; count++;
        pthread_cond_signal(&not_empty);
        pthread_mutex_unlock(&m);
        usleep(10000);
    }
    return NULL;
}

void* consumer(void *arg) {
    (void)arg;
    for (int i = 0; i < 40; ++i) {
        pthread_mutex_lock(&m);
        while (count == 0) pthread_cond_wait(&not_empty, &m);
        int v = buffer[out]; out = (out + 1) % BUF_SIZE; count--;
        pthread_cond_signal(&not_full);
        pthread_mutex_unlock(&m);
        printf("Consumed: %d\n", v);
        usleep(15000);
    }
    return NULL;
}

int main(void) {
    pthread_t p1, p2, c;
    int id1 = 1, id2 = 2;
    pthread_create(&p1, NULL, producer, &id1);
    pthread_create(&p2, NULL, producer, &id2);
    pthread_create(&c, NULL, consumer, NULL);
    pthread_join(p1, NULL); pthread_join(p2, NULL); pthread_join(c, NULL);
    return 0;
}
