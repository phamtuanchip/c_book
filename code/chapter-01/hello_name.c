// hello_name.c
#include <stdio.h>
#include <string.h>   // cần cho strlen

int main(void) {
    char name[128];   // mảng 128 ký tự để chứa tên

    printf("Nhap ten: ");
    if (fgets(name, sizeof(name), stdin) != NULL) {
        size_t len = strlen(name);
        if (len > 0 && name[len - 1] == '\n') {
            name[len - 1] = '\0';   // xóa ký tự xuống dòng ở cuối
        }
        printf("Xin chao, %s!\n", name);
    }
    return 0;
}
