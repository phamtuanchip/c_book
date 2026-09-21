# Chapter 1 — Solutions (tóm tắt)

Bài tập 1: Hello, [Tên bạn]

Sử dụng fgets để đọc tên và printf để in:

```c
#include <stdio.h>
int main(void) {
    char name[100];
    if (fgets(name, sizeof(name), stdin)) {
        // xóa newline
        size_t i = 0; while (name[i] && name[i] != '\n') i++; if (name[i] == '\n') name[i] = '\0';
        printf("Hello, %s!\n", name);
    }
    return 0;
}
```

Bài tập 2: Hai số nguyên

Sử dụng fgets + strtol để an toàn; kiểm tra chia cho 0 trước khi in thương.

Bài tập 3: Độ dài chuỗi

Sử dụng strlen (string.h) hoặc đếm bằng vòng lặp.
