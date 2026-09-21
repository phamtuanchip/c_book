# Mã nguồn chương 3

Chương 3 — Lịch sử và triết lý của ngôn ngữ C

Các file dưới đây được **trích tự động từ bản thảo** (`manuscript/chapter-03.md`) bằng `node tools/extract-code.js`, nên luôn khớp với nội dung sách. Đừng sửa trực tiếp ở đây — hãy sửa trong bản thảo rồi chạy lại script.

## Các file

| File | Mô tả |
|---|---|
| `compat_test.c` | Thử biên dịch với các chuẩn C khác nhau |
| `ub_demo.c` | Undefined behavior thay đổi theo mức tối ưu |

## Biên dịch và chạy

```bash
make            # build tất cả
make asan       # build với AddressSanitizer + UBSan (Linux/macOS/WSL)
make clean
```

Hoặc thủ công, ví dụ:

```bash
gcc -std=c11 -Wall -Wextra -g -o compat_test compat_test.c -lm
```

Đọc lại chương: https://github.com/phamtuanchip/c_book/blob/main/manuscript/chapter-03.md
