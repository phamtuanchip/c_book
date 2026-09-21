# Mã nguồn chương 16

Chương 16 — Mạng cơ bản (sockets)

Các file dưới đây được **trích tự động từ bản thảo** (`manuscript/chapter-16.md`) bằng `node tools/extract-code.js`, nên luôn khớp với nội dung sách. Đừng sửa trực tiếp ở đây — hãy sửa trong bản thảo rồi chạy lại script.

## Các file

| File | Mô tả |
|---|---|
| `echo_server.c` | TCP echo server (tuần tự) |
| `echo_client.c` | TCP client đọc từ bàn phím |
| `http_get.c` | Gửi HTTP GET thô bằng getaddrinfo |

## Biên dịch và chạy

```bash
make            # build tất cả
make asan       # build với AddressSanitizer + UBSan (Linux/macOS/WSL)
make clean
```

Hoặc thủ công, ví dụ:

```bash
gcc -std=c11 -Wall -Wextra -g -o echo_server echo_server.c -lm -pthread
```

> **Lưu ý:** mã dùng POSIX (pthread, socket). Chạy trên Linux, macOS hoặc **WSL**; không biên dịch trực tiếp bằng MinGW/MSVC nếu chưa chuyển sang Winsock (xem chương 16).

Đọc lại chương: https://github.com/phamtuanchip/c_book/blob/main/manuscript/chapter-16.md
