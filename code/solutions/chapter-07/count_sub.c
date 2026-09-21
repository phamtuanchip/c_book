// count_sub.c
#include <stdio.h>
#include <string.h>

// overlap = 0: không chồng lấn ("aaaa" chứa "aa" 2 lần); overlap = 1: có chồng lấn (3 lần)
static int count_sub(const char *s, const char *sub, int overlap) {
    size_t m = strlen(sub);
    if (m == 0) return 0;
    int c = 0;
    for (const char *p = strstr(s, sub); p; p = strstr(overlap ? p + 1 : p + m, sub)) c++;
    return c;
}

int main(void) {
    printf("%d %d\n", count_sub("aaaa", "aa", 0), count_sub("aaaa", "aa", 1));   // 2 3
    return 0;
}
