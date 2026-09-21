# Chương 7 — Mảng & chuỗi (Arrays & Strings)

Mục tiêu chương:

- Hiểu cách khai báo, truyền và xử lý mảng tĩnh và động.
- Xử lý chuỗi (C-strings) an toàn, tránh buffer overflow.
- Hiểu pointer arithmetic và mối quan hệ giữa mảng và con trỏ.

1. Mảng tĩnh và khai báo

- Khai báo: int a[10]; a[0]..a[9]
- Lưu ý: truy cập ngoài ranh giới là undefined behavior.
- size_t và sizeof để xác định kích thước.

2. Mảng động (heap)

- Sử dụng malloc/calloc/realloc và free.
- Kiểm tra NULL sau malloc.
- Ví dụ: int *arr = malloc(n * sizeof *arr);

3. Chuỗi (C-strings)

- Chuỗi là mảng char kết thúc bằng '\0'.
- Dùng fgets để đọc chuỗi an toàn:
  fgets(buf, sizeof buf, stdin);
- Hàm an toàn: strncpy, strncat, snprintf; nhưng hiểu giới hạn của từng hàm.

4. Pointer arithmetic

- p + i tương đương &p[i]; *(p + i) == p[i]
- Dùng pointer khi cần hiệu năng hoặc thao tác mảng chung.

5. Ví dụ thực hành

- reverse_string.c: đảo chuỗi nhập từ stdin bằng hoán vị cặp ký tự.
- dynamic_string.c: builder dùng realloc để nối chuỗi lớn.

6. Bài tập

- Viết safe_concat(dest, src, dest_size) đảm bảo không tràn.
- Viết dynamic vector int: push/pop/resize.

Ghi chú: các ví dụ nằm trong /code/chapter-07; kèm README và test inputs.
