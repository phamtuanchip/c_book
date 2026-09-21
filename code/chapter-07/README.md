# Mã nguồn chương 7

Chương 7 — Mảng & chuỗi (Arrays & Strings)

Các file dưới đây được **trích tự động từ bản thảo** (`manuscript/chapter-07.md`) bằng `node tools/extract-code.js`, nên luôn khớp với nội dung sách. Đừng sửa trực tiếp ở đây — hãy sửa trong bản thảo rồi chạy lại script.

## Các file

| File | Mô tả |
|---|---|
| `text_stats.c` | Đếm ký tự, từ, tần suất chữ cái |

## Biên dịch và chạy

```bash
make            # build tất cả
make asan       # build với AddressSanitizer + UBSan (Linux/macOS/WSL)
make clean
```

Hoặc thủ công, ví dụ:

```bash
gcc -std=c11 -Wall -Wextra -g -o text_stats text_stats.c -lm
```

Đọc lại chương: https://github.com/phamtuanchip/c_book/blob/main/manuscript/chapter-07.md
