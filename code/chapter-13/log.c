// log.c
#define _POSIX_C_SOURCE 200809L        // để có localtime_r
#include "log.h"
#include <stdarg.h>

LogLevel g_log_level = LOG_INFO;

void log_write(LogLevel lvl, const char *file, int line, const char *fmt, ...) {
    if (lvl < g_log_level) return;

    static const char *names[] = {"DEBUG", "INFO", "WARN", "ERROR"};
    time_t now = time(NULL);
    struct tm tmv;
#if defined(_WIN32)
    localtime_s(&tmv, &now);
#else
    localtime_r(&now, &tmv);
#endif
    char ts[32];
    strftime(ts, sizeof ts, "%Y-%m-%d %H:%M:%S", &tmv);

    fprintf(stderr, "%s [%s] %s:%d: ", ts, names[lvl], file, line);
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);          // v-phiên bản nhận va_list
    va_end(ap);
    fputc('\n', stderr);
}
