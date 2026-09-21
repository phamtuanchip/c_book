# Mã nguồn chương 21

Chương 21 — Testing & CI cho C

Các file dưới đây được **trích tự động từ bản thảo** (`manuscript/chapter-21.md`) bằng `node tools/extract-code.js`, nên luôn khớp với nội dung sách. Đừng sửa trực tiếp ở đây — hãy sửa trong bản thảo rồi chạy lại script.

## Các file

| File | Mô tả |
|---|---|
| `stats.h` | Lõi thống kê thuần (dễ kiểm thử) |
| `stats.c` | Cài đặt stats_compute |
| `mini_test.h` | Khung kiểm thử: ASSERT_*, RUN_TEST |
| `test_stats.c` | Bộ kiểm thử cho stats |
| `test_stats_unity.c` | Bộ kiểm thử dùng Unity (cần tải Unity, xem README) |

## Biên dịch và chạy

```bash
make            # build tất cả
make asan       # build với AddressSanitizer + UBSan (Linux/macOS/WSL)
make clean
```

Hoặc thủ công, ví dụ:

```bash
gcc -std=c11 -Wall -Wextra -g -o test_stats test_stats.c -lm
```

`test_stats_unity.c` cần thư viện [Unity](https://github.com/ThrowTheSwitch/Unity): chép `unity.c`, `unity.h`, `unity_internals.h` vào thư mục này rồi `gcc test_stats_unity.c stats.c unity.c -o test_unity`.

Đọc lại chương: https://github.com/phamtuanchip/c_book/blob/main/manuscript/chapter-21.md
