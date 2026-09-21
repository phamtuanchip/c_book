#include <stdio.h>
#include <stdlib.h>

void leak_example(void) {
    int *p = malloc(10 * sizeof(int));
    if (!p) return;
    p[0] = 42;
    /* forgot to free(p) -> memory leak */
}

int main(void) {
    for (int i = 0; i < 1000; ++i) leak_example();
    printf("Leak demo finished (non-freed allocations).\n");
    return 0;
}
