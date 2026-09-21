# Chương 19 — Lời giải bài tập dự án

Mã hoàn chỉnh của giai đoạn 1–4 nằm ở `code/chapter-19/calc.c` (ghép tự động từ các mục 19.3–19.7). Dưới đây là lời giải và hướng dẫn cho từng bước mở rộng.

## Bước 1: lexer và bộ kiểm thử

Bảng trường hợp cho `dump_tokens` (đầu vào → dãy token mong đợi):

| Đầu vào | Kết quả |
|---|---|
| `` (rỗng) | `EOF` |
| `   ` | `EOF` |
| `.5` | `NUM(0.5) EOF` |
| `3.` | `NUM(3) EOF` (strtod nhận `3.`) |
| `1e3` | `NUM(1000) EOF` |
| `x=1` | `IDENT(x) ASSIGN NUM(1) EOF` |
| `2*(3+4)` | `NUM(2) STAR LPAREN NUM(3) PLUS NUM(4) RPAREN EOF` |
| `a_b1` | `IDENT(a_b1) EOF` |
| tên 40 ký tự | `ERROR` (vượt quá 31) |
| `@` | `ERROR` |
| `1 # ghi chu` | `NUM(1) EOF` |

Thêm số hex và chuỗi: trong nhánh "số", nếu gặp `0x`/`0X` thì dùng `strtol(..., 16)`; với chuỗi thêm `T_STRING`, đọc từ `"` đến `"` tiếp theo (kiểm tra chuỗi chưa đóng → `T_ERROR`).

## Bước 2: ghép `calc.c`, ASan, chế độ `:ast`

Biên dịch `gcc -std=c11 -Wall -Wextra -g -fsanitize=address,undefined -o calc calc.c` và kiểm thử với `printf '1+2*3\n(1+2\n-(-3)\n' | ./calc`. LeakSanitizer sạch nghĩa là `node_free` được gọi trên mọi đường (thành công, lỗi cú pháp, lỗi chạy). Chế độ `:ast`: trong `main`, nếu dòng bắt đầu bằng `:ast ` thì parse phần còn lại và gọi `ast_print` thay vì `eval`:

```c
if (strncmp(line, ":ast ", 5) == 0) {
    Node *t = parse_line(line + 5, err, sizeof err, &err_pos);
    if (t) { ast_print(t); printf("\n"); node_free(t); }
    else printf("loi: %s\n", err);
    continue;
}
```

## Bước 3: báo lỗi có dấu `^`

```c
// caret.c
#include <stdio.h>

/* In dòng nguồn rồi một dòng có dấu ^ dưới cột pos (0-based). */
void show_error(const char *line, size_t pos, const char *msg) {
    printf("%s\n", line);
    for (size_t i = 0; i < pos; i++) putchar(line[i] == '\t' ? '\t' : ' ');   // giữ tab để thẳng cột
    printf("^ %s\n", msg);
}

int main(void) {
    show_error("x = 2 +", 7, "can mot bieu thuc");
    return 0;
}
```

Phân biệt loại lỗi bằng tiền tố: `loi tu vung:` (token `T_ERROR`), `loi cu phap:` (parser), `loi thuc thi:` (eval/VM).

## Bước 4: mở rộng biểu thức

- `%`: thêm `T_PERCENT`, xử lý cùng mức với `*` `/` trong `parse_term`; `eval` dùng `fmod`.
- `^` (kết hợp **phải**): thêm tầng `power = unary [ '^' power ]` nằm giữa `term` và `unary` và **đệ quy sang phải** (không dùng vòng `while`): `2^3^2` = `2^(3^2)` = 512.
- So sánh: tầng `comparison = expr [ ( '<' | '>' | '==' ) expr ]` trả `1.0`/`0.0`.
- Hằng và hàm: khi gặp IDENT trong `parse_primary`, nếu token kế là `(` thì dựng `N_CALL` (tên + mảng đối số) và tra bảng `{ "sqrt", sqrt, 1 }, { "min", min2, 2 }`; kiểm tra số đối số lúc **phân tích** để báo lỗi sớm.

## Bước 5: `if`/`while` và file chương trình

Thêm từ khóa trong lexer (đối chiếu IDENT với bảng `if`, `else`, `while`), AST `N_IF/N_WHILE/N_BLOCK`, và `eval` trả giá trị của câu lệnh cuối. Đọc chương trình từ file: `main` mở `argv[1]` và đọc **cả file** vào bộ đệm (mục 12.5), rồi parse liên tiếp các câu lệnh (`statement ';'`) — cần lexer nhớ số dòng để báo lỗi. Giới hạn số vòng lặp (ví dụ 10 triệu bước) để chương trình vô hạn không treo REPL.

## Bước 6: hàm

Cần thêm: (1) `N_CALL` với môi trường mới cho mỗi lần gọi (mảng `Env` theo độ sâu, giới hạn 64 tầng để tránh tràn stack C); (2) bảng hàm `name → (params, body)`; (3) `return` thực hiện bằng cờ trạng thái trong `eval` (ví dụ enum `{ NORMAL, RETURNING }`) thay vì `longjmp`. Kiểm thử: `def fib(n) { if n < 2 { return n; } return fib(n-1) + fib(n-2); }`, `fib(20)` = 6765, và `gcd(48, 18)` = 6.

## Bước 7: VM và so sánh

`./calc --vm` đã chạy được (`vm_eval`). Kiểm thử vi sai: viết script chạy cùng danh sách biểu thức qua hai chế độ và `diff` đầu ra:

```bash
./calc     < tests/exprs.txt > out_tree.txt
./calc --vm < tests/exprs.txt > out_vm.txt
diff out_tree.txt out_vm.txt && echo "GIONG NHAU"
```

Đo tốc độ với vòng lặp 10 triệu lần (sau khi thêm `while`): VM bytecode thường nhanh hơn duyệt cây 2–5 lần; `perf report` cho thấy thời gian nằm ở vòng `switch (in->op)` (VM) so với các lời gọi `eval` đệ quy và `env_find` (cây).

## Bước 8: constant folding

Sau khi dựng AST, gọi hàm `fold(Node *n)` hậu tố: với nút `N_BINOP` mà cả hai con đều là `N_NUM`, thay nút bằng `N_NUM` có giá trị đã tính (giải phóng hai con) — trừ chia cho 0 (để lỗi xuất hiện lúc chạy). `2 * 3 + x` thành `6 + x`. Với `--dump-bytecode`, in mỗi `Instr` dưới dạng `PUSH 6`, `LOAD 0`, `ADD`.
