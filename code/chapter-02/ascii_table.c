// ascii_table.c
#include <stdio.h>
#include <ctype.h>

int main(void) {
    printf("Ma  Ky tu\n");
    for (int c = 32; c < 127; c++) {       // 0..31 là ký tự điều khiển, không in được
        printf("%3d  %c\n", c, c);
    }
    return 0;
}
