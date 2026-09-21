# Mã nguồn chương 2

Chương 2 — Máy tính và lập trình cơ bản

Các file dưới đây được **trích tự động từ bản thảo** (`manuscript/chapter-02.md`) bằng `node tools/extract-code.js`, nên luôn khớp với nội dung sách. Đừng sửa trực tiếp ở đây — hãy sửa trong bản thảo rồi chạy lại script.

## Các file

| File | Mô tả |
|---|---|
| `bases.c` | In một số ở hệ thập phân, hex, bát phân |
| `overflow.c` | Tràn số có dấu / không dấu |
| `float_trap.c` | Bẫy so sánh số thực bằng == |
| `char_is_number.c` | Ký tự là số nguyên nhỏ; đổi chữ hoa/thường, ký tự số |
| `sizes.c` | In sizeof các kiểu cơ bản |
| `endian.c` | Xác định endianness bằng cách in từng byte |
| `memory_regions.c` | Địa chỉ của biến ở các vùng data/bss/heap/stack |
| `add.c` | Hàm cộng đơn giản để xem assembly (`gcc -S`) |
| `bin_convert.c` | In số nguyên dưới dạng nhị phân 32 bit |
| `ascii_table.c` | In bảng ASCII in được |
| `demo_malloc.c` | malloc, realloc, free đúng cách |

## Biên dịch và chạy

```bash
make            # build tất cả
make asan       # build với AddressSanitizer + UBSan (Linux/macOS/WSL)
make clean
```

Hoặc thủ công, ví dụ:

```bash
gcc -std=c11 -Wall -Wextra -g -o bases bases.c -lm
```

Đọc lại chương: https://github.com/phamtuanchip/c_book/blob/main/manuscript/chapter-02.md
