# Mã nguồn chương 9

Chương 9 — Quản lý bộ nhớ và lỗi phổ biến

Các file dưới đây được **trích tự động từ bản thảo** (`manuscript/chapter-09.md`) bằng `node tools/extract-code.js`, nên luôn khớp với nội dung sách. Đừng sửa trực tiếp ở đây — hãy sửa trong bản thảo rồi chạy lại script.

## Các file

| File | Mô tả |
|---|---|
| `leak_demo.c` | Rò rỉ bộ nhớ có chủ đích (dùng Valgrind/ASan để bắt) |
| `use_after_free.c` | Use-after-free có chủ đích (ASan) |
| `vec.c` | Mảng động (Vec) với init/push/destroy |
| `tree.c` | Cây nhị phân tìm kiếm, giải phóng đúng thứ tự |

## Biên dịch và chạy

```bash
make            # build tất cả
make asan       # build với AddressSanitizer + UBSan (Linux/macOS/WSL)
make clean
```

Hoặc thủ công, ví dụ:

```bash
gcc -std=c11 -Wall -Wextra -g -o leak_demo leak_demo.c -lm
```

Đọc lại chương: https://github.com/phamtuanchip/c_book/blob/main/manuscript/chapter-09.md
