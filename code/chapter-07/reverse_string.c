#include <stdio.h>
#include <string.h>

void reverse(char *s) {
    size_t i = 0, j = strlen(s);
    if (j == 0) return;
    if (s[j-1] == '\n') { s[--j] = '\0'; }
    if (j == 0) return;
    for (i = 0; i < j/2; ++i) {
        char t = s[i]; s[i] = s[j-1-i]; s[j-1-i] = t;
    }
}

int main(void) {
    char buf[256];
    printf("Nhap mot chuoi (toi da 255 ky tu): ");
    if (!fgets(buf, sizeof(buf), stdin)) return 1;
    reverse(buf);
    printf("Chuoi dao: %s\n", buf);
    return 0;
}
