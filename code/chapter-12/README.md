# Mã nguồn chương 12

Chương 12 — File I/O & thao tác hệ thống

Các file dưới đây được **trích tự động từ bản thảo** (`manuscript/chapter-12.md`) bằng `node tools/extract-code.js`, nên luôn khớp với nội dung sách. Đừng sửa trực tiếp ở đây — hãy sửa trong bản thảo rồi chạy lại script.

## Các file

| File | Mô tả |
|---|---|
| `write_text.c` | Ghi file văn bản |
| `print_lines.c` | Đọc file theo dòng, đánh số dòng |
| `copy_file.c` | Sao chép file nhị phân bằng fread/fwrite |
| `csv_parser.c` | Tách một dòng CSV có trường trong dấu nháy kép |
| `wc_lite.c` | Đếm dòng/từ/byte như wc |
| `mini_grep.c` | Tìm chuỗi trong file như grep |
| `merge_files.c` | Nối nhiều file vào một file |

## Biên dịch và chạy

```bash
make            # build tất cả
make asan       # build với AddressSanitizer + UBSan (Linux/macOS/WSL)
make clean
```

Hoặc thủ công, ví dụ:

```bash
gcc -std=c11 -Wall -Wextra -g -o write_text write_text.c -lm
```

Đọc lại chương: https://github.com/phamtuanchip/c_book/blob/main/manuscript/chapter-12.md
