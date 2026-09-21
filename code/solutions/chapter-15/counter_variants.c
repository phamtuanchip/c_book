// counter_variants.c
#define _POSIX_C_SOURCE 200809L
#include <pthread.h>
#include <stdatomic.h>
#include <stdio.h>
#include <time.h>

#define N 1000000
#define THREADS 4

static long g_plain;                                       // KHÔNG an toàn: kết quả sai
static long g_locked;
static pthread_mutex_t g_lock = PTHREAD_MUTEX_INITIALIZER;
static atomic_long g_atomic;

static void *work_plain(void *a)  { (void)a; for (int i = 0; i < N; i++) g_plain++; return NULL; }
static void *work_mutex(void *a)  { (void)a; for (int i = 0; i < N; i++) { pthread_mutex_lock(&g_lock); g_locked++; pthread_mutex_unlock(&g_lock); } return NULL; }
static void *work_atomic(void *a) { (void)a; for (int i = 0; i < N; i++) atomic_fetch_add(&g_atomic, 1); return NULL; }

static double run(void *(*fn)(void *)) {
    pthread_t t[THREADS];
    struct timespec a, b;
    clock_gettime(CLOCK_MONOTONIC, &a);
    for (int i = 0; i < THREADS; i++) pthread_create(&t[i], NULL, fn, NULL);
    for (int i = 0; i < THREADS; i++) pthread_join(t[i], NULL);
    clock_gettime(CLOCK_MONOTONIC, &b);
    return (double)(b.tv_sec - a.tv_sec) + (double)(b.tv_nsec - a.tv_nsec) * 1e-9;
}

int main(void) {
    double t1 = run(work_plain), t2 = run(work_mutex), t3 = run(work_atomic);
    printf("khong khoa : %ld (mong doi %ld) %.3fs\n", g_plain, (long)N * THREADS, t1);
    printf("mutex      : %ld %.3fs\n", g_locked, t2);
    printf("atomic     : %ld %.3fs\n", (long)g_atomic, t3);
    return 0;
}
