# Mã nguồn chương 4

Chương 4 — Cú pháp & cấu trúc chương trình C

Các file dưới đây được **trích tự động từ bản thảo** (`manuscript/chapter-04.md`) bằng `node tools/extract-code.js`, nên luôn khớp với nội dung sách. Đừng sửa trực tiếp ở đây — hãy sửa trong bản thảo rồi chạy lại script.

## Các file

| File | Mô tả |
|---|---|
| `read_int.c` | Đọc số nguyên an toàn bằng fgets + strtol |
| `include/math_utils.h` | Header của module toán |
| `src/math_utils.c` | Cài đặt module toán |
| `src/main.c` | Chương trình chính |

## Biên dịch và chạy

```bash
make            # build tất cả
make asan       # build với AddressSanitizer + UBSan (Linux/macOS/WSL)
make clean
```

Đọc lại chương: https://github.com/phamtuanchip/c_book/blob/main/manuscript/chapter-04.md
