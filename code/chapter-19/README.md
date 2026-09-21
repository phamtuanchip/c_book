# Mã nguồn chương 19

Chương 19 — Project: Trình biên dịch / Interpreter mini

Các file dưới đây được **trích tự động từ bản thảo** (`manuscript/chapter-19.md`) bằng `node tools/extract-code.js`, nên luôn khớp với nội dung sách. Đừng sửa trực tiếp ở đây — hãy sửa trong bản thảo rồi chạy lại script.

## Các file

| File | Mô tả |
|---|---|
| `calc.c` | Trình thông dịch mini: lexer → parser → AST → evaluator/VM (REPL) |

## Biên dịch và chạy

```bash
make            # build tất cả
make asan       # build với AddressSanitizer + UBSan (Linux/macOS/WSL)
make clean
```

Hoặc thủ công, ví dụ:

```bash
gcc -std=c11 -Wall -Wextra -g -o calc calc.c -lm
```

## Chạy thử

```bash
./calc            # REPL dùng evaluator duyệt cây
./calc --vm       # REPL dùng bytecode VM
> x = 1 + 2 * (3 - 4)
= -1
```

Đọc lại chương: https://github.com/phamtuanchip/c_book/blob/main/manuscript/chapter-19.md
