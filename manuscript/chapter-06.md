# Chương 6 — Hàm & phạm vi biến (Functions & Scope)

Mục tiêu chương:

- Hiểu cách thiết kế hàm, prototype, và cách tổ chức mã theo modules (.h/.c).
- Khác biệt giữa pass-by-value và việc truyền con trỏ để thay đổi giá trị bên ngoài.
- Hiểu storage class: auto, static, extern; và best practices khi dùng biến toàn cục.

1. Prototype và definition

- Prototype (khai báo) giúp compiler biết kiểu trả về và tham số trước khi gọi hàm.
- Đặt prototype trong header (.h), definition trong .c.

Ví dụ:
// math.h
int add(int a, int b);

// math.c
int add(int a, int b) { return a + b; }

2. Truyền tham số: giá trị vs địa chỉ

- Mặc định: giá trị được truyền theo giá trị (copy). Thay đổi bên trong hàm không ảnh hưởng biến ở caller.
- Truyền con trỏ: hàm nhận địa chỉ để sửa giá trị ban đầu.

Ví dụ swap bằng con trỏ:
void swap(int *a, int *b) { int t = *a; *a = *b; *b = t; }

3. Storage class và scope

- auto: mặc định cho biến cục bộ.
- static: nếu áp dụng cho biến file-scope, biến chỉ nhìn thấy trong translation unit; nếu cho biến cục bộ, biến giữ giá trị giữa các lần gọi.
- extern: khai báo biến toàn cục được định nghĩa ở file khác.

4. Function pointers

- Khai báo: int (*cmp)(const void*, const void*);
- Dùng để truyền callback cho các hàm như qsort.

5. Tách module (header + source)

- Quy tắc: header cho API công khai; .c cho implementation nội bộ.
- Ví dụ cấu trúc: include/math.h, src/math.c, examples/main.c
- Makefile: compile .c -> .o và link thành exe.

6. Ví dụ thực hành

- Viết thư viện vector_int: create, push, pop, free; test memory leak.
- Viết swap bằng con trỏ và so sánh với macro swap.
- Dùng qsort với comparator function pointer.

7. Bài tập

1) Tạo module math với add/sub/mul/div (xử lý chia cho 0). 2) Viết vector_int (push/pop). 3) Viết comparator và sort array bằng qsort.

Ghi chú: đưa các ví dụ trong /code/chapter-06 kèm Makefile và test cases.
