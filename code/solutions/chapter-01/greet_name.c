// greet_name.c
#include <stdio.h>
#include <string.h>

int main(void) {
    char name[100];
    printf("Nhap ten: ");
    if (fgets(name, sizeof name, stdin) == NULL) {
        fprintf(stderr, "Khong doc duoc du lieu\n");
        return 1;
    }
    name[strcspn(name, "\r\n")] = '\0';     // bỏ ký tự xuống dòng (cả \r nếu file từ Windows)
    printf("Hello, %s!\n", name);
    return 0;
}
