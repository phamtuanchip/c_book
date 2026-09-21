// memory_regions.c
#include <stdio.h>
#include <stdlib.h>

int global_init = 42;        // vùng data
int global_uninit;           // vùng BSS (tự động = 0)

int main(void) {
    int local = 7;                                   // stack
    int *dyn = malloc(sizeof(int));                  // heap
    if (dyn == NULL) return 1;
    *dyn = 99;

    printf("ma chuong trinh (main)  : %p\n", (void *)main);
    printf("global_init  (data)     : %p\n", (void *)&global_init);
    printf("global_uninit (bss)     : %p\n", (void *)&global_uninit);
    printf("dyn -> (heap)           : %p\n", (void *)dyn);
    printf("local (stack)           : %p\n", (void *)&local);

    free(dyn);
    return 0;
}
