# Mã nguồn chương 14

Chương 14 — Tiền xử lý & macro nâng cao

Các file dưới đây được **trích tự động từ bản thảo** (`manuscript/chapter-14.md`) bằng `node tools/extract-code.js`, nên luôn khớp với nội dung sách. Đừng sửa trực tiếp ở đây — hãy sửa trong bản thảo rồi chạy lại script.

## Các file

| File | Mô tả |
|---|---|
| `tiny_test.h` | Khung kiểm thử nhỏ dựa trên macro |
| `test_math.c` | Ví dụ dùng tiny_test.h |

## Biên dịch và chạy

```bash
make            # build tất cả
make asan       # build với AddressSanitizer + UBSan (Linux/macOS/WSL)
make clean
```

Hoặc thủ công, ví dụ:

```bash
gcc -std=c11 -Wall -Wextra -g -o test_math test_math.c -lm
```

Đọc lại chương: https://github.com/phamtuanchip/c_book/blob/main/manuscript/chapter-14.md
