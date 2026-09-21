# Chương 9 — Quản lý bộ nhớ và lỗi phổ biến

Mục tiêu chương:

- Hiểu chính xác sự khác nhau giữa stack và heap, cách malloc/calloc/realloc/free hoạt động.
- Nhận diện và sửa các lỗi bộ nhớ phổ biến: memory leak, use-after-free, double-free, dangling pointer.
- Sử dụng công cụ dynamic (Valgrind, ASAN) và static (clang-tidy, cppcheck) để tìm lỗi.

1. Stack vs Heap — minh hoạ

- Stack: lưu biến cục bộ, tự động giải phóng khi rời scope. Kích thước giới hạn.
- Heap: cấp phát động qua malloc/calloc/realloc; phải free thủ công.

Ví dụ ngắn:

int *make_array(size_t n) {
    // Trả pointer tới heap; caller phải free
    int *a = malloc(n * sizeof *a);
    if (!a) return NULL;
    return a;
}

2. Các lỗi thường gặp (chi tiết & cách sửa)

- Memory leak: quên free. Cách: xác định ownership, mỗi malloc phải có free tương ứng, viết tests và dùng Valgrind.
- Use-after-free: dùng con trỏ sau khi free → undefined behavior. Sửa: đặt pointer = NULL sau free; kiểm tra NULL.
- Double free: free cùng địa chỉ hai lần → crash; giữ ownership rõ ràng.
- Dangling pointer: con trỏ trỏ tới vùng đã free; tránh bằng NULL và thiết kế API rõ ràng.

3. Công cụ và cách dùng

- Valgrind (Linux): valgrind --leak-check=full ./prog — liệt kê leak, use-after-free.
- AddressSanitizer: gcc -fsanitize=address -g -o prog prog.c — phát hiện use-after-free, OOB.
- static analyzers: clang-tidy, cppcheck — tìm pattern nguy hiểm trước khi chạy.

4. Patterns an toàn

- Kiểm tra kết quả malloc/calloc: if (!ptr) { /* xử lý lỗi */ }
- Dùng RAII-like patterns trong C: wrapper struct có hàm init/free để tránh leak.
- Dùng valgrind/asan trong CI cho bài tập và ví dụ.

5. Ví dụ thực hành

- /code/chapter-09/leak_example.c: tạo leak cố tình và sửa.
- /code/chapter-09/use_after_free.c: minh hoạ và cách phát hiện bằng ASAN.
- /code/chapter-09/tree.c: cấp phát/giải phóng đúng cho binary tree.

Bài tập

1) Viết và test một binary tree: allocate và free mọi node; kiểm tra bằng Valgrind.
2) Viết ví dụ minh họa use-after-free và sửa nó bằng ownership rõ ràng.

Ghi chú: kèm README hướng dẫn sử dụng Valgrind và cách build với -fsanitize=address.