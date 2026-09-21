# Chương 1 — Lời giải bài tập

> Hãy tự làm trước khi xem lời giải. Biên dịch mọi bài bằng `gcc -std=c11 -Wall -Wextra -g -o prog prog.c`.

## Bài 1: in tên, năm sinh, quê quán

```c
#include <stdio.h>

int main(void) {
    printf("Ten:\t\tNguyen Van A\n");
    printf("Nam sinh:\t2000\n");
    printf("Que quan:\tHa Noi\n");
    return 0;
}
```

`\t` đưa con trỏ tới vị trí tab kế tiếp nên các giá trị thẳng hàng; `\n` xuống dòng.

## Bài 2: đọc tên bằng `fgets` và chào

```c
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
```

`strcspn(name, "\r\n")` trả về vị trí của ký tự xuống dòng đầu tiên (hoặc độ dài chuỗi nếu không có), nên câu lệnh gán an toàn ở cả hai trường hợp.

## Bài 3: bốn phép toán, xử lý chia cho 0

```c
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <errno.h>

// Đọc một số nguyên từ một dòng; trả 0 nếu hợp lệ
static int read_int(const char *prompt, int *out) {
    char buf[64];
    printf("%s", prompt);
    if (fgets(buf, sizeof buf, stdin) == NULL) return -1;

    char *end;
    errno = 0;
    long v = strtol(buf, &end, 10);
    if (end == buf || (*end != '\n' && *end != '\0')) return -1;   // không phải số / có ký tự thừa
    if (errno == ERANGE || v < INT_MIN || v > INT_MAX) return -1;  // ngoài phạm vi int
    *out = (int)v;
    return 0;
}

int main(void) {
    int a, b;
    if (read_int("a = ", &a) != 0 || read_int("b = ", &b) != 0) {
        fprintf(stderr, "Du lieu khong hop le\n");
        return 1;
    }

    // Dùng long long để phép tính không tràn với a, b lớn
    printf("%d + %d = %lld\n", a, b, (long long)a + b);
    printf("%d - %d = %lld\n", a, b, (long long)a - b);
    printf("%d * %d = %lld\n", a, b, (long long)a * b);

    if (b == 0) {
        printf("Khong the chia cho 0\n");
    } else {
        printf("%d / %d = %d (phan nguyen)\n", a, b, a / b);
        printf("%d %% %d = %d (phan du)\n", a, b, a % b);
        printf("%d / %d = %.3f (so thuc)\n", a, b, (double)a / b);
    }
    return 0;
}
```

Điểm đáng chú ý: (1) dùng `fgets` + `strtol` thay cho `scanf` để bắt được đầu vào sai như `12abc`; (2) ép sang `long long` trước khi cộng/trừ/nhân để không bị tràn số `int` (tràn số có dấu là UB); (3) kiểm tra `b == 0` **trước** khi chia hoặc lấy dư (chia số nguyên cho 0 là UB); (4) `INT_MIN / -1` cũng tràn; nếu muốn xử lý triệt để, kiểm tra thêm trường hợp `a == INT_MIN && b == -1`.

## Bài 4: độ dài chuỗi bằng `strlen` và bằng vòng lặp

```c
#include <stdio.h>
#include <string.h>

static size_t my_strlen(const char *s) {
    size_t n = 0;
    while (s[n] != '\0') n++;
    return n;
}

int main(void) {
    char line[256];
    printf("Nhap mot dong: ");
    if (fgets(line, sizeof line, stdin) == NULL) return 1;
    line[strcspn(line, "\r\n")] = '\0';

    size_t a = strlen(line);
    size_t b = my_strlen(line);
    printf("strlen    = %zu\n", a);
    printf("my_strlen = %zu\n", b);
    printf("%s\n", a == b ? "Hai ket qua giong nhau" : "KHAC NHAU (co loi!)");
    return 0;
}
```

Với văn bản tiếng Việt có dấu (UTF-8), cả hai hàm đều đếm **byte**, không phải ký tự: `"Việt"` cho 6 chứ không phải 4 (xem chương 7).

## Bài 5: quan sát các thông báo lỗi

Thử ba chương trình sai dưới đây và đối chiếu với bảng ở mục 1.5.

**(a) Thiếu dấu `;`**

```c
#include <stdio.h>
int main(void) {
    printf("Hello\n")
    return 0;
}
```

gcc báo `error: expected ';' before 'return'`. Đây là lỗi **biên dịch (compiling)**, cụ thể là lỗi cú pháp. Chú ý dòng được báo là dòng **sau** dòng thiếu `;`. Sửa: thêm `;` sau `printf(...)`.

**(b) Thiếu `#include <stdio.h>`**

```c
int main(void) {
    printf("Hello\n");
    return 0;
}
```

gcc (với `-Wall`) báo `warning: implicit declaration of function 'printf'` (từ GCC 14 là **lỗi**). Đây là lỗi ở giai đoạn **biên dịch**, vì compiler chưa thấy khai báo của `printf` (header chưa được đưa vào ở bước tiền xử lý). Sửa: thêm `#include <stdio.h>`.

**(c) Gọi hàm không tồn tại**

```c
void khong_ton_tai(void);
int main(void) {
    khong_ton_tai();
    return 0;
}
```

Biên dịch **thành công** (prototype đã cho compiler biết chữ ký), nhưng **liên kết** thất bại: `undefined reference to 'khong_ton_tai'`. Đây là lỗi ở giai đoạn **liên kết (linking)**: linker không tìm được định nghĩa. Sửa: viết định nghĩa hàm, hoặc liên kết file/thư viện chứa nó.

Nếu bạn không viết prototype ở (c) và gọi thẳng `khong_ton_tai();`, C89 chấp nhận (khai báo ngầm) và bạn chỉ thấy lỗi khi liên kết; từ C99, compiler hiện đại cảnh báo/báo lỗi ngay ở bước biên dịch — vì vậy luôn bật `-Wall -Wextra`.
