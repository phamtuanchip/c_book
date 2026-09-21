// greeting.c
#include <stdio.h>
#include <stdlib.h>

// Trả về chuỗi cấp phát bằng malloc; NGƯỜI GỌI phải free. NULL nếu hết bộ nhớ hoặc name == NULL.
static char *make_greeting(const char *name) {
    if (!name) return NULL;
    int n = snprintf(NULL, 0, "Xin chao %s", name);        // đo độ dài cần thiết
    if (n < 0) return NULL;
    char *out = malloc((size_t)n + 1);
    if (!out) return NULL;
    snprintf(out, (size_t)n + 1, "Xin chao %s", name);
    return out;
}

int main(void) {
    char *g = make_greeting("An");
    if (g) { puts(g); free(g); }
    return 0;
}
