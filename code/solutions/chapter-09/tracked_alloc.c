// tracked_alloc.c
#include <stdio.h>
#include <stdlib.h>

static size_t g_allocs, g_frees, g_bytes;

static void report(void) {
    printf("cap phat: %zu, giai phong: %zu, tong %zu byte%s\n",
           g_allocs, g_frees, g_bytes, g_allocs != g_frees ? "  <-- CO RO RI!" : "");
}

static void *tracked_malloc(size_t n) {
    static int registered;
    if (!registered) { atexit(report); registered = 1; }       // in báo cáo khi thoát
    void *p = malloc(n);
    if (p) { g_allocs++; g_bytes += n; }
    return p;
}

static void tracked_free(void *p) {
    if (p) g_frees++;
    free(p);
}

int main(void) {
    int *a = tracked_malloc(40);
    int *b = tracked_malloc(80);
    tracked_free(a);
    (void)b;                                    // cố ý không free b để thấy cảnh báo
    return 0;
}
