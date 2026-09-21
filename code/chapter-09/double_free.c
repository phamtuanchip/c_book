#include <stdio.h>
#include <stdlib.h>

int main(void) {
    int *p = malloc(4 * sizeof(int));
    if (!p) return 1;
    free(p);
    /* double free */
    free(p);
    printf("After double free (may crash).\n");
    return 0;
}
