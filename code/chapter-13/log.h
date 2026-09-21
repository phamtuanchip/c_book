// log.h
#ifndef LOG_H
#define LOG_H

#include <stdio.h>
#include <time.h>

typedef enum { LOG_DEBUG, LOG_INFO, LOG_WARN, LOG_ERROR } LogLevel;

extern LogLevel g_log_level;              // định nghĩa trong log.c; mức tối thiểu được ghi

void log_write(LogLevel lvl, const char *file, int line, const char *fmt, ...)
#if defined(__GNUC__)
    __attribute__((format(printf, 4, 5)))  // để compiler kiểm tra định dạng như printf
#endif
;

#define LOG_DEBUG_MSG(...) log_write(LOG_DEBUG, __FILE__, __LINE__, __VA_ARGS__)
#define LOG_INFO_MSG(...)  log_write(LOG_INFO,  __FILE__, __LINE__, __VA_ARGS__)
#define LOG_WARN_MSG(...)  log_write(LOG_WARN,  __FILE__, __LINE__, __VA_ARGS__)
#define LOG_ERR_MSG(...)   log_write(LOG_ERROR, __FILE__, __LINE__, __VA_ARGS__)

#endif
