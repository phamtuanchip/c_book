# Mã nguồn chương 10

Chương 10 — Struct, union, enum

Các file dưới đây được **trích tự động từ bản thảo** (`manuscript/chapter-10.md`) bằng `node tools/extract-code.js`, nên luôn khớp với nội dung sách. Đừng sửa trực tiếp ở đây — hãy sửa trong bản thảo rồi chạy lại script.

## Các file

| File | Mô tả |
|---|---|
| `layout.c` | sizeof/offsetof và padding của struct |
| `linked_list.c` | Danh sách liên kết đơn |
| `stack.c` | Ngăn xếp bằng mảng động |
| `queue.c` | Hàng đợi vòng (ring buffer) |
| `stack_adt.h` | Giao diện ADT stack (opaque pointer) |
| `stack_adt.c` | Cài đặt ADT stack |

## Biên dịch và chạy

```bash
make            # build tất cả
make asan       # build với AddressSanitizer + UBSan (Linux/macOS/WSL)
make clean
```

Hoặc thủ công, ví dụ:

```bash
gcc -std=c11 -Wall -Wextra -g -o layout layout.c -lm
```

Đọc lại chương: https://github.com/phamtuanchip/c_book/blob/main/manuscript/chapter-10.md
