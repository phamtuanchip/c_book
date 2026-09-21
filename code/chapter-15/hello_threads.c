// hello_threads.c
#define _POSIX_C_SOURCE 200809L
#include <pthread.h>
#include <stdio.h>

static void *worker(void *arg) {
    int id = *(int *)arg;
    printf("Xin chao tu luong %d\n", id);
    return NULL;
}

int main(void) {
    pthread_t threads[4];
    int ids[4];

    for (int i = 0; i < 4; i++) {
        ids[i] = i;                                   // mỗi luồng một biến riêng (xem cảnh báo bên dưới)
        int rc = pthread_create(&threads[i], NULL, worker, &ids[i]);
        if (rc != 0) { fprintf(stderr, "pthread_create: %d\n", rc); return 1; }
    }
    for (int i = 0; i < 4; i++) pthread_join(threads[i], NULL);

    printf("xong\n");
    return 0;
}
