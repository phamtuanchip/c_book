# Mã nguồn chương 15

Chương 15 — Đa luồng & đồng bộ (POSIX threads)

Các file dưới đây được **trích tự động từ bản thảo** (`manuscript/chapter-15.md`) bằng `node tools/extract-code.js`, nên luôn khớp với nội dung sách. Đừng sửa trực tiếp ở đây — hãy sửa trong bản thảo rồi chạy lại script.

## Các file

| File | Mô tả |
|---|---|
| `hello_threads.c` | Tạo và chờ nhiều luồng |
| `parallel_sum.c` | Cộng mảng lớn bằng nhiều luồng |
| `race.c` | Race condition và cách sửa bằng mutex |
| `producer_consumer.c` | Producer–consumer với hàng đợi giới hạn |

## Biên dịch và chạy

```bash
make            # build tất cả
make asan       # build với AddressSanitizer + UBSan (Linux/macOS/WSL)
make clean
```

Hoặc thủ công, ví dụ:

```bash
gcc -std=c11 -Wall -Wextra -g -o hello_threads hello_threads.c -lm -pthread
```

> **Lưu ý:** mã dùng POSIX (pthread, socket). Chạy trên Linux, macOS hoặc **WSL**; không biên dịch trực tiếp bằng MinGW/MSVC nếu chưa chuyển sang Winsock (xem chương 16).

Đọc lại chương: https://github.com/phamtuanchip/c_book/blob/main/manuscript/chapter-15.md
