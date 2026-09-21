# Mã nguồn chương 5

Chương 5 — Điều khiển luồng (Control Flow)

Các file dưới đây được **trích tự động từ bản thảo** (`manuscript/chapter-05.md`) bằng `node tools/extract-code.js`, nên luôn khớp với nội dung sách. Đừng sửa trực tiếp ở đây — hãy sửa trong bản thảo rồi chạy lại script.

## Các file

| File | Mô tả |
|---|---|
| `classify.c` | Phân loại số âm/dương/0, chẵn/lẻ |
| `sum_input.c` | Cộng các số nhập đến hết đầu vào |
| `prime_check.c` | Kiểm tra số nguyên tố |
| `menu.c` | Menu tương tác dùng switch và do-while |

## Biên dịch và chạy

```bash
make            # build tất cả
make asan       # build với AddressSanitizer + UBSan (Linux/macOS/WSL)
make clean
```

Hoặc thủ công, ví dụ:

```bash
gcc -std=c11 -Wall -Wextra -g -o classify classify.c -lm
```

Đọc lại chương: https://github.com/phamtuanchip/c_book/blob/main/manuscript/chapter-05.md
