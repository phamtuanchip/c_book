# Mã nguồn chương 11

Chương 11 — Header, Makefile, build systems

Các file dưới đây được **trích tự động từ bản thảo** (`manuscript/chapter-11.md`) bằng `node tools/extract-code.js`, nên luôn khớp với nội dung sách. Đừng sửa trực tiếp ở đây — hãy sửa trong bản thảo rồi chạy lại script.

## Các file

| File | Mô tả |
|---|---|
| `include/geometry.h` | Header giao diện module hình học |
| `src/geometry.c` | Cài đặt module hình học |
| `src/main.c` | Chương trình chính |

## Biên dịch và chạy

```bash
make            # build tất cả
make asan       # build với AddressSanitizer + UBSan (Linux/macOS/WSL)
make clean
```

Đọc lại chương: https://github.com/phamtuanchip/c_book/blob/main/manuscript/chapter-11.md
