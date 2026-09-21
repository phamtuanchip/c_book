// parallel_sum.c
#define _POSIX_C_SOURCE 200809L
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define NTHREADS 4

typedef struct {
    const int *data;
    size_t start, end;
    long partial_sum;
} SumTask;

static void *sum_worker(void *arg) {
    SumTask *t = arg;
    long s = 0;
    for (size_t i = t->start; i < t->end; i++) s += t->data[i];
    t->partial_sum = s;
    return NULL;
}

int main(void) {
    const size_t N = 10000000;
    int *data = malloc(N * sizeof *data);
    if (!data) return 1;
    for (size_t i = 0; i < N; i++) data[i] = 1;

    pthread_t th[NTHREADS];
    SumTask   tasks[NTHREADS];
    size_t chunk = N / NTHREADS;

    for (int i = 0; i < NTHREADS; i++) {
        tasks[i].data  = data;
        tasks[i].start = i * chunk;
        tasks[i].end   = (i == NTHREADS - 1) ? N : (i + 1) * chunk;   // luồng cuối nhận phần dư
        tasks[i].partial_sum = 0;
        if (pthread_create(&th[i], NULL, sum_worker, &tasks[i]) != 0) { perror("create"); return 1; }
    }

    long total = 0;
    for (int i = 0; i < NTHREADS; i++) {
        pthread_join(th[i], NULL);              // join tạo "hàng rào bộ nhớ": kết quả luồng đã ghi là nhìn thấy được
        total += tasks[i].partial_sum;
    }
    printf("tong = %ld (mong doi %zu)\n", total, N);
    free(data);
    return 0;
}
