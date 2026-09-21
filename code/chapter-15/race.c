// race.c
#define _POSIX_C_SOURCE 200809L
#include <pthread.h>
#include <stdio.h>

static long counter = 0;               // biến chia sẻ

static void *inc(void *arg) {
    (void)arg;
    for (int i = 0; i < 1000000; i++) {
        counter++;                     // KHÔNG nguyên tử!
    }
    return NULL;
}

int main(void) {
    pthread_t a, b;
    pthread_create(&a, NULL, inc, NULL);
    pthread_create(&b, NULL, inc, NULL);
    pthread_join(a, NULL);
    pthread_join(b, NULL);
    printf("counter = %ld (mong doi 2000000)\n", counter);
    return 0;
}
