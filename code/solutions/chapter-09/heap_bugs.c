// heap_bugs.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv) {
    int which = argc > 1 ? atoi(argv[1]) : 0;
    if (which == 1) {                       /* use-after-free */
        int *p = malloc(sizeof *p);
        *p = 1;
        free(p);
        printf("%d\n", *p);
    } else if (which == 2) {                /* double free */
        char *s = malloc(10);
        free(s);
        free(s);
    } else if (which == 3) {                /* heap overflow off-by-one: quên chỗ cho '\0' */
        char *s = malloc(5);
        strcpy(s, "hello");
        free(s);
    } else {
        puts("dung: ./heap_bugs 1|2|3");
    }
    return 0;
}
