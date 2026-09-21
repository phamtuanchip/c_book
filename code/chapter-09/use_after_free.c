#include <stdio.h>
#include <stdlib.h>

int *bad_function(void) {
    int *p = malloc(sizeof(int));
    if (!p) return NULL;
    *p = 123;
    free(p);
    return p; /* returning pointer to freed memory -> use-after-free */
}

int main(void) {
    int *q = bad_function();
    printf("Dereferencing freed pointer (undefined behavior): %d\n", q ? *q : -1);
    return 0;
}
