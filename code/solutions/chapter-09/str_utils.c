// str_utils.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Trả về bản sao mới của s. NGƯỜI GỌI phải free(). NULL nếu s == NULL hoặc hết bộ nhớ. */
static char *str_dup(const char *s) {
    if (!s) return NULL;
    size_t n = strlen(s) + 1;                  // +1 cho '\0'
    char *p = malloc(n);
    if (!p) return NULL;
    memcpy(p, s, n);
    return p;
}

/* Trả về chuỗi mới a+b. NGƯỜI GỌI phải free(). Kiểm tra tràn size_t khi cộng độ dài. */
static char *str_concat(const char *a, const char *b) {
    if (!a || !b) return NULL;
    size_t la = strlen(a), lb = strlen(b);
    if (la > (size_t)-1 - lb - 1) return NULL;
    char *p = malloc(la + lb + 1);
    if (!p) return NULL;
    memcpy(p, a, la);
    memcpy(p + la, b, lb + 1);                 // chép cả '\0'
    return p;
}

int main(void) {
    char *a = str_dup("xin ");
    char *c = str_concat(a, "chao");
    if (c) puts(c);
    free(a);
    free(c);
    return 0;
}
