// layout.c
#include <stdio.h>
#include <stddef.h>     // offsetof

struct A { char c; int i; char d; };
struct B { int i; char c; char d; };

int main(void) {
    printf("sizeof(struct A) = %zu\n", sizeof(struct A));    // 12
    printf("  offset c = %zu, i = %zu, d = %zu\n",
           offsetof(struct A, c), offsetof(struct A, i), offsetof(struct A, d));  // 0, 4, 8
    printf("sizeof(struct B) = %zu\n", sizeof(struct B));    // 8
    printf("  offset i = %zu, c = %zu, d = %zu\n",
           offsetof(struct B, i), offsetof(struct B, c), offsetof(struct B, d));  // 0, 4, 5
    return 0;
}
