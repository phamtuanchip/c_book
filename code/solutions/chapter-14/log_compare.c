// log_compare.c
#include <stdarg.h>
#include <stdio.h>

#define LOG(fmt, ...) fprintf(stderr, "[%s:%d %s] " fmt "\n", __FILE__, __LINE__, __func__, ##__VA_ARGS__)

static void log_fn(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    fprintf(stderr, "[?:? ?] ");           // hàm KHÔNG biết __LINE__ của nơi gọi
    vfprintf(stderr, fmt, ap);
    fputc('\n', stderr);
    va_end(ap);
}

int main(void) {
    LOG("khoi dong");                      // [log_compare.c:19 main] khoi dong
    LOG("x = %d", 42);
    log_fn("x = %d", 42);
    return 0;
}
