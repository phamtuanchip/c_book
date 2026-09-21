// hms.c
#include <stdio.h>

int main(void) {
    long total;
    if (scanf("%ld", &total) != 1 || total < 0) return 1;
    long h = total / 3600;
    long m = (total % 3600) / 60;
    long s = total % 60;
    printf("%02ld:%02ld:%02ld\n", h, m, s);        // 3725 -> 01:02:05
    return 0;
}
