# Mã nguồn chương 1

Chương 1 — Giới thiệu và chương trình C đầu tiên

Các file dưới đây được **trích tự động từ bản thảo** (`manuscript/chapter-01.md`) bằng `node tools/extract-code.js`, nên luôn khớp với nội dung sách. Đừng sửa trực tiếp ở đây — hãy sửa trong bản thảo rồi chạy lại script.

## Các file

| File | Mô tả |
|---|---|
| `hello.c` | Chương trình đầu tiên: in "Hello, World!" |
| `hello_name.c` | Đọc tên bằng fgets và chào |
| `arithmetic.c` | Bốn phép toán với hai số nguyên, xử lý chia cho 0 |

## Biên dịch và chạy

```bash
make            # build tất cả
make asan       # build với AddressSanitizer + UBSan (Linux/macOS/WSL)
make clean
```

Hoặc thủ công, ví dụ:

```bash
gcc -std=c11 -Wall -Wextra -g -o hello hello.c -lm
```

Đọc lại chương: https://github.com/phamtuanchip/c_book/blob/main/manuscript/chapter-01.md
