# Chương 4 — Cú pháp & cấu trúc chương trình C

Mục tiêu chương:

- Nắm rõ cú pháp cơ bản: tokens, khai báo, biểu thức, câu lệnh điều khiển.
- Hiểu tiền xử lý (#include, #define), include guards và cách tổ chức header/source.
- Thực hành tách module, viết prototype, và so sánh macro vs inline function.

1. Tokens và cú pháp cơ bản

- Tokens gồm identifiers (tên biến/hàm), keywords (int, return, if...), literals (số, ký tự, chuỗi), operators và punctuators.
- Câu lệnh kết thúc bằng dấu chấm phẩy `;`. Block được bao bởi `{ ... }`.

2. Kiểu dữ liệu và khai báo

- Kiểu cơ bản: char, short, int, long, float, double, _Bool, void.
- Phân biệt signed/unsigned, và dùng các kiểu kích thước cố định (stdint.h): int8_t, int32_t, uint64_t.
- Khai báo và khởi tạo biến: int x = 0; const int MAX = 100;

3. Hàm, prototype và phạm vi

- Prototype: đặt ở header (.h) để các file khác biết về hàm.
  Ví dụ: int add(int a, int b);
- Definition: int add(int a, int b) { return a + b; }
- Phạm vi (scope): local (biến trong hàm), file-scope (static biến ở đầu file), global (không nên dùng quá nhiều).

4. Tiền xử lý

- #include <stdio.h> để lấy khai báo thư viện chuẩn.
- Include guard để tránh include lặp:

#ifndef MATH_UTILS_H
#define MATH_UTILS_H
/* declarations */
#endif

- #define cho macro; cẩn thận với side-effects.

Ví dụ macro nguy hiểm:
#define SQUARE(x) ((x)*(x))
// SQUARE(i++) sẽ có hành vi không mong muốn

5. Macro vs inline function

- Macro: expand text, không có type-checking, có thể gây side-effect.
- Inline function (static inline): an toàn hơn, có type-checking và tối ưu hoá tương đương macro khi compiler cho phép.

Ví dụ:
static inline int square_int(int x) { return x * x; }

6. Tổ chức project nhỏ: header + source

- Thư mục: src/, include/, tests/
- Ví dụ: math.h (declarations), math.c (implementation), main.c (sử dụng)
- Makefile: rules để compile từng module và link thành executable.

7. Bài tập

- Viết macro SQUARE(x) và minh hoạ lỗi khi dùng SQUARE(i++).
- Viết inline function square_int và so sánh với macro.
- Tách chương trình thành math.h/math.c/main.c và build bằng Makefile.

Ghi chú: kèm theo ví dụ minh họa trong /code/chapter-04, mỗi ví dụ có README hướng dẫn biên dịch.
