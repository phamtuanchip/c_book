// ratelimit.c
#define _POSIX_C_SOURCE 200809L
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>

#define SLOTS 1024

typedef struct { uint32_t ip; double tokens; double last; } Bucket;
static Bucket g_buckets[SLOTS];
static pthread_mutex_t g_rl_lock = PTHREAD_MUTEX_INITIALIZER;

static double now_s(void) { struct timespec t; clock_gettime(CLOCK_MONOTONIC, &t); return (double)t.tv_sec + (double)t.tv_nsec * 1e-9; }

/* Trả 1 nếu cho phép, 0 nếu vượt hạn mức (trả 429). Hạn mức: `rate` yêu cầu/giây, tối đa `burst`. */
static int rate_allow(uint32_t ip, double rate, double burst) {
    int ok;
    pthread_mutex_lock(&g_rl_lock);                       // nhiều worker cùng gọi -> phải khóa
    Bucket *b = &g_buckets[ip % SLOTS];
    double t = now_s();
    if (b->ip != ip) { b->ip = ip; b->tokens = burst; b->last = t; }     // đụng độ/khởi tạo (đơn giản hóa)
    b->tokens += (t - b->last) * rate;
    if (b->tokens > burst) b->tokens = burst;
    b->last = t;
    ok = b->tokens >= 1.0;
    if (ok) b->tokens -= 1.0;
    pthread_mutex_unlock(&g_rl_lock);
    return ok;
}

int main(void) {
    int allowed = 0;
    for (int i = 0; i < 20; i++) allowed += rate_allow(0x7f000001u, 5.0, 10.0);
    printf("%d/20 duoc phep\n", allowed);                 // ~10 (burst) do 20 yêu cầu tới ngay lập tức
    return 0;
}
