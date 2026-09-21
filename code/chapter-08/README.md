# Mã nguồn chương 8

Chương 8 — Con trỏ chi tiết (Pointers Deep Dive)

Các file dưới đây được **trích tự động từ bản thảo** (`manuscript/chapter-08.md`) bằng `node tools/extract-code.js`, nên luôn khớp với nội dung sách. Đừng sửa trực tiếp ở đây — hãy sửa trong bản thảo rồi chạy lại script.

## Các file

| File | Mô tả |
|---|---|
| `ptr_practice.c` | Đảo mảng và tìm chuỗi con bằng con trỏ |

## Biên dịch và chạy

```bash
make            # build tất cả
make asan       # build với AddressSanitizer + UBSan (Linux/macOS/WSL)
make clean
```

Hoặc thủ công, ví dụ:

```bash
gcc -std=c11 -Wall -Wextra -g -o ptr_practice ptr_practice.c -lm
```

Đọc lại chương: https://github.com/phamtuanchip/c_book/blob/main/manuscript/chapter-08.md
