// xmalloc_array.c
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* NULL nếu n * size tràn hoặc hết bộ nhớ */
static void *xmalloc_array(size_t n, size_t size) {
    if (size != 0 && n > SIZE_MAX / size) return NULL;
    return malloc(n * size);
}

static void *xmalloc_array_builtin(size_t n, size_t size) {       // gcc/clang
    size_t total;
    if (__builtin_mul_overflow(n, size, &total)) return NULL;
    return malloc(total);
}

int main(void) {
    printf("%p\n", xmalloc_array(SIZE_MAX / 2 + 1, 2));           // (nil)
    printf("%p\n", xmalloc_array_builtin(SIZE_MAX / 2 + 1, 2));   // (nil)
    void *p = xmalloc_array(1000, sizeof(int));
    printf("%s\n", p ? "ok" : "loi");
    free(p);
    return 0;
}
