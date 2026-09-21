// log_init.c
#include <stdlib.h>
#include <string.h>

typedef enum { LOG_DEBUG, LOG_INFO, LOG_WARN, LOG_ERROR } LogLevel;
LogLevel g_log_level_from_env(void) {
    const char *e = getenv("LOG_LEVEL");
    if (!e) return LOG_INFO;
    if (strcmp(e, "DEBUG") == 0) return LOG_DEBUG;
    if (strcmp(e, "WARN")  == 0) return LOG_WARN;
    if (strcmp(e, "ERROR") == 0) return LOG_ERROR;
    return LOG_INFO;
}
