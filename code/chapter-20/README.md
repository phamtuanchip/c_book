# Mã nguồn chương 20

Chương 20 — Project: Web server đơn giản

Các file dưới đây được **trích tự động từ bản thảo** (`manuscript/chapter-20.md`) bằng `node tools/extract-code.js`, nên luôn khớp với nội dung sách. Đừng sửa trực tiếp ở đây — hãy sửa trong bản thảo rồi chạy lại script.

## Các file

| File | Mô tả |
|---|---|
| `server.c` | Web server: thread pool, file tĩnh, JSON API, tắt êm |

## Biên dịch và chạy

```bash
make            # build tất cả
make asan       # build với AddressSanitizer + UBSan (Linux/macOS/WSL)
make clean
```

Hoặc thủ công, ví dụ:

```bash
gcc -std=c11 -Wall -Wextra -g -o server server.c -lm -pthread
```

> **Lưu ý:** mã dùng POSIX (pthread, socket). Chạy trên Linux, macOS hoặc **WSL**; không biên dịch trực tiếp bằng MinGW/MSVC nếu chưa chuyển sang Winsock (xem chương 16).

## Chạy thử

```bash
make && ./server 8080 4 ./www
curl -i http://127.0.0.1:8080/
curl -i http://127.0.0.1:8080/api/time
```

Đọc lại chương: https://github.com/phamtuanchip/c_book/blob/main/manuscript/chapter-20.md
