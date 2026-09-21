# Mã nguồn chương 13

Chương 13 — Xử lý lỗi & ngoại lệ trong C

Các file dưới đây được **trích tự động từ bản thảo** (`manuscript/chapter-13.md`) bằng `node tools/extract-code.js`, nên luôn khớp với nội dung sách. Đừng sửa trực tiếp ở đây — hãy sửa trong bản thảo rồi chạy lại script.

## Các file

| File | Mô tả |
|---|---|
| `log.h` | Logging: macro LOG_* và log_write |
| `log.c` | Cài đặt log_write |
| `fault.h` | Chèn lỗi cấp phát (fault injection) |
| `fault.c` | Cài đặt test_malloc |
| `sum_file.c` | Tính tổng số trong file, xử lý lỗi đầy đủ |

## Biên dịch và chạy

```bash
make            # build tất cả
make asan       # build với AddressSanitizer + UBSan (Linux/macOS/WSL)
make clean
```

Hoặc thủ công, ví dụ:

```bash
gcc -std=c11 -Wall -Wextra -g -o sum_file sum_file.c -lm
```

Đọc lại chương: https://github.com/phamtuanchip/c_book/blob/main/manuscript/chapter-13.md
