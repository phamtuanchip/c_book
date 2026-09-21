# Mã nguồn chương 18

Chương 18 — Bảo mật & an toàn bộ nhớ

Các file dưới đây được **trích tự động từ bản thảo** (`manuscript/chapter-18.md`) bằng `node tools/extract-code.js`, nên luôn khớp với nội dung sách. Đừng sửa trực tiếp ở đây — hãy sửa trong bản thảo rồi chạy lại script.

## Các file

| File | Mô tả |
|---|---|
| `vuln.c` | Tràn bộ đệm có chủ đích (chỉ để học, dùng với ASan) |
| `safe_parse.c` | Phân tích thông điệp có tiền tố độ dài an toàn |

## Biên dịch và chạy

```bash
make            # build tất cả
make asan       # build với AddressSanitizer + UBSan (Linux/macOS/WSL)
make clean
```

Hoặc thủ công, ví dụ:

```bash
gcc -std=c11 -Wall -Wextra -g -o safe_parse safe_parse.c -lm
```

`vuln.c` **có lỗi cố ý**. Chỉ biên dịch bằng `make vuln` (không nằm trong `make all`) và chạy với `-fsanitize=address` trên máy của bạn.

Đọc lại chương: https://github.com/phamtuanchip/c_book/blob/main/manuscript/chapter-18.md
