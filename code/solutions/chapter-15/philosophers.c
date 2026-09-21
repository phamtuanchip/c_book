// philosophers.c
#define _POSIX_C_SOURCE 200809L
#include <pthread.h>
#include <stdio.h>

#define N 5
static pthread_mutex_t fork_lock[N];

static void *philosopher(void *arg) {
    int id = (int)(long)arg;
    int a = id, b = (id + 1) % N;
    if (a > b) { int t = a; a = b; b = t; }        // thứ tự khóa nhất quán => không có chu trình chờ
    for (int meal = 0; meal < 3; meal++) {
        pthread_mutex_lock(&fork_lock[a]);
        pthread_mutex_lock(&fork_lock[b]);
        printf("triet gia %d an (bua %d)\n", id, meal + 1);
        pthread_mutex_unlock(&fork_lock[b]);
        pthread_mutex_unlock(&fork_lock[a]);
    }
    return NULL;
}

int main(void) {
    pthread_t t[N];
    for (int i = 0; i < N; i++) pthread_mutex_init(&fork_lock[i], NULL);
    for (long i = 0; i < N; i++) pthread_create(&t[i], NULL, philosopher, (void *)i);
    for (int i = 0; i < N; i++) pthread_join(t[i], NULL);
    for (int i = 0; i < N; i++) pthread_mutex_destroy(&fork_lock[i]);
    return 0;
}
