# Chương 8 — Con trỏ chi tiết (Pointers Deep Dive)

Mục tiêu

- Hiểu con trỏ: khai báo, dereference (*), address-of (&), pointer arithmetic.
- Biết con trỏ hàm, con trỏ tới mảng và mảng con trỏ.
- Nhận diện lỗi phổ biến: null pointer, uninitialized pointer, pointer to freed memory.

Tổng quan

Con trỏ là tính năng mạnh mẽ của C cho phép thao tác trực tiếp với địa chỉ bộ nhớ. Sử dụng con trỏ giúp viết code hiệu năng cao và mô tả cấu trúc dữ liệu động (linked lists, trees), nhưng cũng dễ dẫn đến lỗi nghiêm trọng nếu xử lý không cẩn thận.

1. Con trỏ cơ bản

- Khai báo: int *p; char *s; void *pv;
- Lấy địa chỉ: int x = 5; int *p = &x;
- Dereference: *p để truy cập/gán giá trị; luôn đảm bảo p != NULL trước khi deref.

Ví dụ:
int x = 10;
int *p = &x;
*p = 20; // x = 20

2. Pointer arithmetic và mối quan hệ với mảng

- Nếu p là int*, thì p + 1 tăng địa chỉ theo sizeof(int) bytes.
- Mảng khi truyền vào hàm sẽ hạ xuống thành pointer tới phần tử đầu; hàm nhận cả kích thước mảng để an toàn.

Ví dụ:
void print_array(int *a, size_t n) {
    for (size_t i = 0; i < n; ++i) printf("%d ", a[i]);
}

3. Con trỏ hàm

- Khai báo: int (*cmp)(const void*, const void*);
- Dùng làm callback: qsort, bsearch, hoặc API event.

Ví dụ comparator cho qsort:
int cmp_int(const void *a, const void *b) {
    int ia = *(const int*)a;
    int ib = *(const int*)b;
    return (ia > ib) - (ia < ib);
}

4. Lỗi phổ biến và cách phòng tránh

- Null dereference: luôn kiểm tra p != NULL.
- Use-after-free: sau khi free(p) gán p = NULL.
- Uninitialized pointer: khởi tạo pointer = NULL khi khai báo nếu chưa có địa chỉ.
- Buffer overflow: khi dùng pointer để viết vào vùng nhớ, đảm bảo chỉ ghi trong giới hạn.

5. Ví dụ thực hành

- pointer_examples.c: minh hoạ truy cập, arithmetic, aliasing và cách tránh lỗi.
- linked_list.c: implement linked list với hàm add/remove/print; chú ý quản lý bộ nhớ.

6. Công cụ debug và kiểm thử

- Valgrind (Linux) để dò rò bộ nhớ và use-after-free.
- ASAN (AddressSanitizer) khi build bằng clang/gcc với -fsanitize=address.

Ghi chú: kèm sơ đồ vùng nhớ (stack/heap/data/text) và ví dụ chạy được trong /code/chapter-08.
