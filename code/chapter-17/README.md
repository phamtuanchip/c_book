# Mã nguồn chương 17

Chương 17 — Tối ưu hóa & profiling

Các file dưới đây được **trích tự động từ bản thảo** (`manuscript/chapter-17.md`) bằng `node tools/extract-code.js`, nên luôn khớp với nội dung sách. Đừng sửa trực tiếp ở đây — hãy sửa trong bản thảo rồi chạy lại script.

## Các file

| File | Mô tả |
|---|---|
| `timer.h` | Đồng hồ đo thời gian (clock_gettime) |
| `profile_demo.c` | Chương trình có điểm nghẽn O(n²) để profile |
| `cache_demo.c` | Duyệt ma trận theo hàng và theo cột |
| `sort_bench.c` | So sánh qsort, quicksort, mergesort |

## Biên dịch và chạy

```bash
make            # build tất cả
make asan       # build với AddressSanitizer + UBSan (Linux/macOS/WSL)
make clean
```

Hoặc thủ công, ví dụ:

```bash
gcc -std=c11 -Wall -Wextra -g -o profile_demo profile_demo.c -lm
```

Đọc lại chương: https://github.com/phamtuanchip/c_book/blob/main/manuscript/chapter-17.md
