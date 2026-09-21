// times_table.c
#include <stdio.h>

int main(void) {
    for (int i = 1; i <= 9; i++) {
        for (int j = 1; j <= 10; j++) printf("%d x %2d = %3d\n", i, j, i * j);
        printf("\n");
    }
    return 0;
}
