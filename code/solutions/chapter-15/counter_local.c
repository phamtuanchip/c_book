// counter_local.c
#define _POSIX_C_SOURCE 200809L
#include <pthread.h>
#include <stdio.h>

#define N 10000000
static long g_total;
static pthread_mutex_t g_lock = PTHREAD_MUTEX_INITIALIZER;

static void *work(void *arg) {
    (void)arg;
    long local = 0;
    for (int i = 0; i < N; i++) local++;             // cộng vào biến CỤC BỘ: không tranh chấp
    pthread_mutex_lock(&g_lock);                     // chỉ khóa MỘT lần
    g_total += local;
    pthread_mutex_unlock(&g_lock);
    return NULL;
}

int main(void) {
    pthread_t t[4];
    for (int i = 0; i < 4; i++) pthread_create(&t[i], NULL, work, NULL);
    for (int i = 0; i < 4; i++) pthread_join(t[i], NULL);
    printf("%ld\n", g_total);                        // 40000000
    return 0;
}
