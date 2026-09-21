# Chương 10 — Struct, union, enum

Mục tiêu chương:

- Nắm vững cách khai báo và sử dụng struct, union, enum; hiểu alignment, padding và ảnh hưởng tới sizeof.
- Biết cách dùng struct để xây dựng cấu trúc dữ liệu (linked list, stack, queue) và quản lý memory cho chúng.

1. Struct cơ bản

- Khai báo: struct Point { int x; int y; };
- Typedef để đơn giản: typedef struct Point Point;
- Truy cập: p.x, p.y; với pointer: p->x

2. Padding & alignment

- Vì alignment, compiler có thể chèn padding giữa trường để thỏa điều kiện alignment.
- Dùng sizeof và offsetof để đo; sắp xếp trường (small->large) có thể giảm padding.

Ví dụ: trước/sau tối ưu layout

3. Union và khi dùng

- union U { int i; float f; char s[4]; } — các trường chia sẻ cùng vùng nhớ.
- Dùng union cho variant hoặc overlay binary data; phải cẩn thận về field hiện tại hợp lệ.

4. Enum và bitfield

- enum Color { RED, GREEN, BLUE };
- Bitfield: struct Flags { unsigned a:1; unsigned b:1; }; chú ý portability giữa compiler.

5. Ứng dụng: cấu trúc dữ liệu

- Linked list: struct Node { int val; struct Node *next; };
- Quy tắc: viết các hàm create_node, insert, delete, free_list; luôn free cả danh sách.

6. Ví dụ thực hành

- /code/chapter-10/linked_list.c: insert/delete/find/iterate
- /code/chapter-10/stack.c: stack bằng dynamic array
- /code/chapter-10/queue.c: circular buffer implementation

Bài tập

1) Implement doubly linked list with splice.
2) Show size reduction by reordering fields in a struct and explain.

Ghi chú: kèm diagram memory và test cases.
