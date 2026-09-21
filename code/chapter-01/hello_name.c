#include <stdio.h>

int main(void) {
    char name[100];
    printf("Nhap ten cua ban: ");
    if (fgets(name, sizeof(name), stdin) == NULL) return 1;
    // remove newline if present
    size_t i = 0;
    while (name[i] != '\0') {
        if (name[i] == '\n') { name[i] = '\0'; break; }
        i++;
    }
    printf("Hello, %s!\n", name);
    return 0;
}
