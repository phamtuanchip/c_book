// counter.c
static int counter = 0;                 // chỉ dùng được trong counter.c
static void log_msg(const char *m) {    // hàm nội bộ
    (void)m;   /* ... ghi log ... */
}

int counter_next(void) {                // hàm công khai (external linkage)
    log_msg("tang");
    return ++counter;
}
