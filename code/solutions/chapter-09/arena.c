// arena.c
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct { unsigned char *base; size_t cap, used; } Arena;

static int arena_init(Arena *a, size_t cap) {
    a->base = malloc(cap);
    a->cap = cap;
    a->used = 0;
    return a->base ? 0 : -1;
}

/* Cấp n byte, căn hàng 16; NULL nếu hết chỗ. Không có free từng khối. */
static void *arena_alloc(Arena *a, size_t n) {
    size_t start = (a->used + 15u) & ~(size_t)15u;
    if (start > a->cap || n > a->cap - start) return NULL;
    a->used = start + n;
    return a->base + start;
}

static void arena_free_all(Arena *a) { free(a->base); a->base = NULL; a->cap = a->used = 0; }

int main(void) {
    Arena a;
    if (arena_init(&a, 1024) != 0) return 1;
    int *xs = arena_alloc(&a, 10 * sizeof *xs);
    char *name = arena_alloc(&a, 32);
    if (!xs || !name) return 1;
    xs[0] = 42;
    snprintf(name, 32, "arena");
    printf("%d %s used=%zu\n", xs[0], name, a.used);
    arena_free_all(&a);                            // trả tất cả một lần
    return 0;
}
