# Chương 3 — Lời giải bài tập

## Bài 1: biên dịch `compat_test.c` với các chuẩn

| Lệnh | Kết quả điển hình | Giải thích |
|---|---|---|
| `gcc -std=c89 -pedantic -Wall compat_test.c` | cảnh báo: `ISO C90 does not support 'long long'`, `C++ style comments are not allowed in ISO C90` | `long long` và `//` chỉ có từ C99 |
| `gcc -std=c99 compat_test.c` | sạch; in `__STDC_VERSION__ = 199901` | |
| `gcc -std=c11 compat_test.c` | sạch; `201112` | |
| `gcc -std=c17 compat_test.c` | sạch; `201710` | C17 chỉ sửa lỗi kỹ thuật của C11 |

Nếu bỏ `-pedantic`, gcc ở chế độ `-std=c89` vẫn chấp nhận `long long` và `//` như phần mở rộng, nên **không có cảnh báo**. Vì vậy hãy kết hợp `-std=` với `-pedantic` khi muốn kiểm tra tính di động.

## Bài 2: `ub_demo.c` với `-O0` và `-O2`

- `-O0`: thường in `0` (`INT_MAX + 1` quay vòng thành số âm nên `x + 1 > x` sai).
- `-O2`: thường in `1`. Compiler suy luận "tràn số có dấu không xảy ra" nên thay biểu thức bằng hằng `1`.
- Thêm `-fsanitize=undefined`: chương trình báo `runtime error: signed integer overflow: 2147483647 + 1 cannot be represented in type 'int'`.

Kết luận: cùng mã nguồn, kết quả phụ thuộc mức tối ưu — dấu hiệu điển hình của UB.

## Bài 3: gây ra từng nguồn UB

Mỗi đoạn dưới đây chạy với `-fsanitize=address,undefined` sẽ bị báo lỗi.

```c
// ub_cases.c
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
    int which = argc > 1 ? atoi(argv[1]) : 0;
    volatile int zero = 0;                 // volatile: compiler không đoán trước được giá trị

    switch (which) {
    case 0: { volatile int x = INT_MAX; x = x + 1; printf("%d\n", x); break; }          /* tràn số có dấu */
    case 1: { int a[3] = {0}; volatile int i = 5; a[i] = 1; break; }                     /* ngoài mảng (stack) */
    case 2: { int *p = NULL; printf("%d\n", *p); break; }                                /* NULL dereference */
    case 3: { int *p = malloc(sizeof *p); free(p); *p = 1; break; }                      /* use-after-free */
    case 4: { int *p = malloc(sizeof *p); free(p); free(p); break; }                     /* double free */
    case 5: { int x; volatile int y = x; printf("%d\n", y); break; }                     /* chưa khởi tạo */
    case 6: { printf("%d\n", 1 / zero); break; }                                         /* chia cho 0 */
    case 7: { volatile int s = 32; printf("%d\n", 1 << s); break; }                      /* dịch bit quá rộng */
    case 8: { char *s = "abc"; s[0] = 'x'; break; }                                      /* ghi vào literal */
    default: puts("chon 0..8");
    }
    return 0;
}
```

Chạy `./ub_cases 3` để thấy `heap-use-after-free`, `./ub_cases 4` cho `attempting double-free`... Một số trường hợp (5, 8) tùy compiler có thể cần bật `-O0` hoặc dùng MSan (clang) để bắt.

## Bài 4: phân loại

| Biểu thức | Loại |
|---|---|
| (a) `sizeof(long)` | **implementation-defined** (4 hoặc 8 tùy nền tảng) |
| (b) `a[i] = i++;` | **undefined** (sửa `i` và đọc `i` không có sequence point) |
| (c) `int x = 5 / 0;` | **undefined** |
| (d) thứ tự đánh giá `f() + g()` | **unspecified** (compiler chọn thứ tự nhưng không cần ghi lại) |
| (e) `-7 / 2` | **xác định** từ C99: làm tròn về 0 nên `-3` |

## Bài 5: in tên hệ điều hành

```c
// os_name.c
#include <stdio.h>

int main(void) {
#if defined(_WIN32)
    puts("Windows");
#elif defined(__APPLE__)
    puts("macOS");
#elif defined(__linux__)
    puts("Linux");
#else
    puts("khong ro");
#endif
    return 0;
}
```

Các macro này do compiler định nghĩa sẵn (xem `gcc -dM -E - < /dev/null`). Hãy kiểm tra nền tảng cụ thể (`_WIN32`, `__APPLE__`, `__linux__`) trước rồi mới tới nhóm chung như `__unix__`.

## Bài 6: giới hạn kiểu

```c
// limits_demo.c
#include <limits.h>
#include <stdint.h>
#include <stdio.h>

int main(void) {
    printf("INT_MAX   = %d\n", INT_MAX);
    printf("INT_MIN   = %d\n", INT_MIN);
    printf("LONG_MAX  = %ld\n", LONG_MAX);
    printf("SIZE_MAX  = %zu\n", (size_t)SIZE_MAX);
    return 0;
}
```

`INT_MAX` = 2147483647 và `INT_MIN` = −2147483648 ở mọi nền tảng phổ biến (int 32 bit). `LONG_MAX` là 2147483647 trên Windows (long 32 bit) nhưng 9223372036854775807 trên Linux/macOS 64-bit (long 64 bit). `SIZE_MAX` là 2⁶⁴ − 1 trên hệ 64 bit và 2³² − 1 trên hệ 32 bit.
