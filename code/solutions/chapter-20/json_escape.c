// json_escape.c
#include <stdio.h>
#include <stddef.h>

/* Ghi bản thoát ký tự JSON của src vào dst (cap byte). Trả 0 nếu vừa, -1 nếu bị cắt. */
static int json_escape(const char *src, char *dst, size_t cap) {
    size_t w = 0;
    for (const unsigned char *p = (const unsigned char *)src; *p; p++) {
        char tmp[8];
        size_t n;
        switch (*p) {
            case '"':  n = (size_t)snprintf(tmp, sizeof tmp, "\\\""); break;
            case '\\': n = (size_t)snprintf(tmp, sizeof tmp, "\\\\"); break;
            case '\n': n = (size_t)snprintf(tmp, sizeof tmp, "\\n");  break;
            case '\r': n = (size_t)snprintf(tmp, sizeof tmp, "\\r");  break;
            case '\t': n = (size_t)snprintf(tmp, sizeof tmp, "\\t");  break;
            default:
                if (*p < 0x20) n = (size_t)snprintf(tmp, sizeof tmp, "\\u%04x", *p);   // ký tự điều khiển
                else { tmp[0] = (char)*p; tmp[1] = '\0'; n = 1; }                       // byte UTF-8 giữ nguyên
        }
        if (w + n + 1 > cap) { if (cap) dst[w] = '\0'; return -1; }
        for (size_t i = 0; i < n; i++) dst[w++] = tmp[i];
    }
    if (cap) dst[w] = '\0';
    return 0;
}

int main(void) {
    char out[128];
    json_escape("say \"hi\"\n\\", out, sizeof out);
    printf("{\"msg\":\"%s\"}\n", out);                 // {"msg":"say \"hi\"\n\\"}
    return 0;
}
