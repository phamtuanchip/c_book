# Chương 9 — Quản lý bộ nhớ và lỗi phổ biến

## Mục tiêu chương

- Hiểu chính xác **stack** và **heap** khác nhau ở đâu, khi nào dùng cái nào.
- Dùng `malloc`, `calloc`, `realloc`, `free` đúng cách, biết ý nghĩa của từng hàm và các bẫy.
- Nhận diện, tái hiện và sửa các lỗi bộ nhớ: rò rỉ, use-after-free, double free, tràn heap, đọc bộ nhớ chưa khởi tạo.
- Xác định **quyền sở hữu (ownership)** của bộ nhớ và thiết kế API rõ ràng.
- Dùng thành thạo công cụ: AddressSanitizer, Valgrind, cppcheck/clang-tidy.
- Xây dựng cấu trúc dữ liệu động (mảng động, cây nhị phân) và giải phóng nó đúng cách.

## 9.1. Stack và heap

Chương 2 đã giới thiệu; ở đây ta đi sâu.

| Tiêu chí | Stack | Heap |
|---|---|---|
| Ai quản lý | Compiler tự động | Lập trình viên (`malloc`/`free`) |
| Tốc độ cấp phát | Rất nhanh (dịch một thanh ghi) | Chậm hơn (allocator tìm khối trống) |
| Kích thước | Nhỏ, cố định (thường 1–8 MB) | Lớn, giới hạn bởi RAM |
| Thời gian sống | Đến khi hàm kết thúc | Đến khi bạn `free` |
| Lỗi điển hình | Stack overflow (đệ quy sâu, mảng cục bộ quá lớn) | Rò rỉ, use-after-free, double free |
| Kích thước biết khi nào | Lúc biên dịch (trừ VLA) | Lúc chạy |

Khi nào **bắt buộc** dùng heap?

1. **Kích thước chỉ biết lúc chạy** (đọc n từ đầu vào rồi tạo mảng n phần tử).
2. **Dữ liệu cần sống lâu hơn hàm tạo ra nó** (trả về chuỗi/struct cho người gọi).
3. **Dữ liệu lớn** (ảnh, bộ đệm hàng MB) — không nên đặt trên stack.
4. **Cấu trúc động** (danh sách liên kết, cây, bảng băm) có số phần tử thay đổi.

## 9.2. Các hàm cấp phát (`<stdlib.h>`)

### `malloc`

```c
void *malloc(size_t size);
```

Xin `size` **byte** liên tiếp. Trả về con trỏ tới vùng nhớ (nội dung **chưa được khởi tạo** — là rác), hoặc `NULL` nếu thất bại.

```c
int *arr = malloc(10 * sizeof *arr);    // 10 số nguyên
if (arr == NULL) {
    fprintf(stderr, "het bo nho\n");
    return -1;
}
```

Mẹo và quy tắc:

- Dùng `sizeof *arr` (thay vì `sizeof(int)`) để luôn đúng khi bạn đổi kiểu của `arr`.
- **Luôn kiểm tra `NULL`.** Trên máy hiện đại `malloc` hiếm khi thất bại với vùng nhỏ, nhưng trên hệ nhúng hoặc khi xin lượng lớn, nó xảy ra thật.
- Khi nhân để tính kích thước, hãy cẩn thận **tràn số** (`n * sizeof *arr` với `n` do người dùng nhập). Kiểm tra: `if (n > SIZE_MAX / sizeof *arr) /* lỗi */`, hoặc dùng `calloc`.
- `malloc(0)` có kết quả do triển khai quy định (có thể `NULL` hoặc một con trỏ hợp lệ không dùng được) — tránh.

### `calloc`

```c
void *calloc(size_t count, size_t size);
```

Cấp phát `count × size` byte và **khởi tạo tất cả bằng 0**. Nó còn **kiểm tra tràn** phép nhân giúp bạn — nên tốt hơn `malloc(count * size)` khi `count` không tin cậy.

```c
int *zeros = calloc(100, sizeof *zeros);   // 100 số 0
```

### `realloc`

```c
void *realloc(void *ptr, size_t new_size);
```

Đổi kích thước một khối đã cấp phát. Nội dung cũ được giữ (đến mức nhỏ hơn của hai kích thước). Có thể **mở rộng tại chỗ** hoặc **cấp khối mới, chép dữ liệu, giải phóng khối cũ** — nên con trỏ trả về **có thể khác** `ptr`.

**Cách dùng đúng:** dùng con trỏ tạm, để không mất con trỏ cũ khi thất bại.

```c
int *tmp = realloc(arr, new_n * sizeof *arr);
if (tmp == NULL) {
    // arr VẪN hợp lệ và VẪN cần free
    free(arr);
    return -1;
}
arr = tmp;             // chỉ gán lại khi thành công
```

**Cách sai kinh điển:**

```c
arr = realloc(arr, new_n * sizeof *arr);   // nếu thất bại: arr = NULL và khối cũ bị RÒ RỈ
```

Ngoài ra, mọi con trỏ khác đang trỏ vào khối cũ (`int *first = arr;`) có thể trở thành **treo** sau `realloc`.

### `free`

```c
void free(void *ptr);
```

Trả khối nhớ về cho hệ thống. Quy tắc:

- Chỉ `free` con trỏ **được trả về từ `malloc`/`calloc`/`realloc`** (chính xác địa chỉ đó, không phải `p + 1`).
- `free(NULL)` **an toàn** (không làm gì) — nên không cần `if (p) free(p)`.
- Sau `free`, **không được dùng** vùng nhớ nữa. Thói quen tốt: `p = NULL;` ngay sau.
- Không `free` hai lần (double free), không `free` biến trên stack hay chuỗi literal.

### Allocator hoạt động thế nào (bức tranh)

`malloc` không xin hệ điều hành mỗi lần. Nó quản lý một "kho" bộ nhớ lấy từ hệ điều hành (qua `brk`/`mmap`), chia thành các khối và giữ **metadata** (kích thước khối...) ngay trước khối bạn nhận được:

```text
   ┌──────────────┬────────────────────────────────┐
   │  metadata    │      vùng người dùng (bạn)      │
   │ (kích thước) │  ← con trỏ malloc trả về       │
   └──────────────┴────────────────────────────────┘
```

Vì thế ghi tràn khỏi khối của bạn **phá metadata** của khối kế, và lỗi thường chỉ bộc lộ về sau, khi gọi `malloc`/`free` lần khác — rất khó truy vết nếu không có công cụ.

## 9.3. Các lỗi bộ nhớ phổ biến

Với mỗi lỗi, hãy hiểu **nguyên nhân**, **hậu quả**, **cách phát hiện**, **cách sửa**.

### 1. Rò rỉ bộ nhớ (memory leak)

Cấp phát nhưng **quên `free`** (hoặc mất con trỏ trước khi kịp `free`). Bộ nhớ không dùng được nữa nhưng cũng không được trả lại.

```c
// leak_demo.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void process(void) {
    char *buf = malloc(1024);
    if (!buf) return;
    strcpy(buf, "hello");
    printf("%s\n", buf);
    // quên free(buf)  -> rò rỉ 1024 byte mỗi lần gọi
}

int main(void) {
    for (int i = 0; i < 1000; i++) process();    // rò rỉ ~1 MB
    return 0;
}
```

Hậu quả: chương trình chạy lâu (server, daemon) dần ăn hết RAM và bị hệ điều hành kill. Với chương trình chạy ngắn, hệ điều hành thu hồi mọi thứ khi thoát, nên rò rỉ "vô hại" — nhưng vẫn là lỗi cần sửa.

Các dạng rò rỉ khác:

```c
char *p = malloc(10);
p = malloc(20);           // mất địa chỉ khối 10 byte -> rò rỉ
```

```c
int *f(void) {
    int *a = malloc(100);
    if (!a) return NULL;
    int *b = malloc(100);
    if (!b) return NULL;  // quên free(a) trên đường lỗi -> rò rỉ
    ...
}
```

Sửa: mỗi `malloc` phải có đúng một `free` **trên mọi đường thoát** (kể cả đường lỗi) — dùng mẫu `goto cleanup` (chương 5 và 13).

### 2. Use-after-free (dùng sau khi giải phóng)

```c
// use_after_free.c
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    int *p = malloc(sizeof *p);
    *p = 42;
    free(p);
    printf("%d\n", *p);      // LỖI: đọc vùng nhớ đã trả lại -> UB
    *p = 7;                  // còn tệ hơn: ghi -> có thể phá dữ liệu của phần khác
    return 0;
}
```

Nguy hiểm vì chương trình **thường vẫn "chạy đúng"** (dữ liệu chưa bị ghi đè), nhưng lỗi bùng phát ở thời điểm không đoán trước; và đây là nguồn của nhiều lỗ hổng khai thác được. Phát hiện: AddressSanitizer (`heap-use-after-free`). Sửa: gán `p = NULL` sau `free`, thiết kế quyền sở hữu rõ ràng.

### 3. Double free

```c
char *s = malloc(10);
free(s);
free(s);                 // LỖI: giải phóng hai lần -> hỏng cấu trúc của allocator, crash hoặc bị khai thác
```

Thường xảy ra khi **hai nơi cùng nghĩ mình sở hữu** khối nhớ. Sửa: xác định ai sở hữu; gán `NULL` sau `free`.

### 4. Tràn heap (heap buffer overflow)

```c
char *s = malloc(5);
strcpy(s, "hello");      // cần 6 byte (5 ký tự + '\0'): ghi tràn 1 byte
```

Lỗi *off-by-one* rất hay gặp khi quên `+1` cho `'\0'`. Sửa: `malloc(strlen(str) + 1)`. Phát hiện: AddressSanitizer (`heap-buffer-overflow`).

### 5. Đọc bộ nhớ chưa khởi tạo

```c
int *a = malloc(3 * sizeof *a);
printf("%d\n", a[0]);    // giá trị rác. Dùng calloc hoặc gán trước khi đọc.
```

Valgrind báo `Conditional jump or move depends on uninitialised value(s)`; MemorySanitizer (clang) cũng bắt được.

### 6. Free sai con trỏ

```c
int arr[10];
free(arr);               // LỖI: arr nằm trên stack
char *s = "abc";
free(s);                 // LỖI: literal không được cấp phát bằng malloc
int *p = malloc(10 * sizeof *p);
free(p + 1);             // LỖI: không phải địa chỉ mà malloc trả về
```

### 7. Con trỏ treo (dangling pointer)

Bất kỳ con trỏ nào còn giữ địa chỉ của vùng nhớ **đã hết hiệu lực** (đã `free`, hoặc là biến cục bộ đã ra khỏi phạm vi, hoặc `realloc` đã di chuyển khối).

### 8. Stack overflow

```c
void recurse(void) { recurse(); }         // đệ quy vô hạn
int main(void) { char big[100 * 1024 * 1024]; }   // mảng cục bộ 100 MB
```

Triệu chứng: `Segmentation fault` hoặc `Stack overflow`. Sửa: dùng heap cho dữ liệu lớn, thêm điều kiện dừng cho đệ quy.

## 9.4. Công cụ phát hiện lỗi

### AddressSanitizer (ASan) — công cụ số một

Được tích hợp trong gcc và clang; **chỉ cần thêm cờ biên dịch**:

```bash
gcc -std=c11 -g -O1 -fsanitize=address -fno-omit-frame-pointer -o prog prog.c
./prog
```

Ví dụ báo cáo cho `use_after_free.c`:

```text
==12345==ERROR: AddressSanitizer: heap-use-after-free on address 0x602000000010
READ of size 4 at 0x602000000010 thread T0
    #0 0x... in main use_after_free.c:9
0x602000000010 is located 0 bytes inside of 4-byte region [0x602000000010,0x602000000014)
freed by thread T0 here:
    #0 0x... in free
    #1 0x... in main use_after_free.c:8
previously allocated by thread T0 here:
    #0 0x... in malloc
    #1 0x... in main use_after_free.c:6
```

Cách đọc: dòng đầu là **loại lỗi** (`heap-use-after-free`); tiếp theo là **nơi lỗi xảy ra** (dòng 9); rồi **nơi khối bị free** (dòng 8) và **nơi được cấp phát** (dòng 6). Ba thông tin đó thường đủ để tìm ra nguyên nhân. ASan cũng phát hiện rò rỉ khi chương trình kết thúc (LeakSanitizer, có sẵn trên Linux/macOS).

Kết hợp thêm `-fsanitize=undefined` để bắt UB (tràn số, dịch bit...). Chạy chậm hơn khoảng 2×, nên dùng khi phát triển/kiểm thử chứ không phải bản phát hành.

Trên Windows (MinGW) hỗ trợ ASan còn hạn chế; hãy dùng **WSL**, hoặc MSVC (`cl /fsanitize=address`).

### Valgrind (Linux, macOS cũ)

```bash
gcc -std=c11 -g -O0 -o prog prog.c
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./prog
```

Valgrind không cần biên dịch lại đặc biệt, phát hiện được rò rỉ, use-after-free, đọc dữ liệu chưa khởi tạo. Chạy chậm hơn 20–50×. Báo cáo rò rỉ điển hình:

```text
==999== 1,024 bytes in 1 blocks are definitely lost in loss record 1 of 1
==999==    at 0x4C2FB0F: malloc (vg_replace_malloc.c:299)
==999==    by 0x4005A7: process (leak_demo.c:7)
==999==    by 0x4005D6: main (leak_demo.c:15)
==999== LEAK SUMMARY:
==999==    definitely lost: 1,024 bytes in 1 blocks
```

`definitely lost` là rò rỉ chắc chắn; `still reachable` là bộ nhớ còn con trỏ tới lúc thoát (thường không nghiêm trọng).

### Phân tích tĩnh

Chương trình phân tích mã **không cần chạy**:

```bash
cppcheck --enable=all --std=c11 src/
clang-tidy src/*.c -- -std=c11
gcc -Wall -Wextra -fanalyzer src/*.c      # gcc 10+ có bộ phân tích tĩnh
```

Chúng tìm được `malloc` không kiểm tra, rò rỉ trên đường lỗi, dùng biến chưa khởi tạo... Kết hợp cả **tĩnh** (rẻ, chạy sớm) và **động** (chính xác, phải chạy qua đường mã).

## 9.5. Quyền sở hữu (ownership)

Hầu hết lỗi bộ nhớ đến từ việc **không rõ ai chịu trách nhiệm `free`**. C không có cơ chế của ngôn ngữ, nên phải thống nhất bằng quy ước và ghi chú tài liệu.

Quy tắc rõ ràng:

1. **Mỗi khối nhớ có đúng một "chủ".** Chủ chịu trách nhiệm `free`.
2. Khi truyền con trỏ, nói rõ: **cho mượn** (người nhận chỉ dùng, không `free`) hay **chuyển quyền sở hữu** (người nhận `free`).
3. Đặt tên và tài liệu hóa cho rõ:

```c
/* Trả về chuỗi mới được cấp phát; NGƯỜI GỌI phải free(). Trả về NULL nếu hết bộ nhớ. */
char *str_dup(const char *s);

/* Chỉ ĐỌC s; không giữ con trỏ sau khi hàm trả về. */
size_t str_count_words(const char *s);

/* Lấy quyền sở hữu buf: hàm này sẽ free(buf). */
void consume_buffer(char *buf);
```

4. **Cặp hàm tạo/hủy:** đặt tên đối xứng: `thing_create` ↔ `thing_destroy`, `list_new` ↔ `list_free`.
5. Cấu trúc chứa con trỏ cấp phát động phải có hàm hủy giải phóng **toàn bộ** thành viên.

### Mẫu `init`/`destroy` (giống RAII của C)

```c
// vec.c — mảng động (dynamic array)
#include <stdio.h>
#include <stdlib.h>

typedef struct {
    int    *data;
    size_t  size;      // số phần tử đang dùng
    size_t  cap;       // sức chứa hiện có
} Vec;

int vec_init(Vec *v) {
    v->data = NULL;
    v->size = 0;
    v->cap  = 0;
    return 0;
}

void vec_destroy(Vec *v) {
    free(v->data);
    v->data = NULL;
    v->size = v->cap = 0;
}

int vec_push(Vec *v, int value) {
    if (v->size == v->cap) {                        // hết chỗ -> tăng gấp đôi
        size_t new_cap = v->cap ? v->cap * 2 : 4;
        int *tmp = realloc(v->data, new_cap * sizeof *tmp);
        if (tmp == NULL) return -1;                 // v->data còn nguyên, không rò rỉ
        v->data = tmp;
        v->cap  = new_cap;
    }
    v->data[v->size++] = value;
    return 0;
}

int main(void) {
    Vec v;
    vec_init(&v);
    for (int i = 0; i < 1000; i++) {
        if (vec_push(&v, i * i) != 0) { vec_destroy(&v); return 1; }
    }
    printf("size = %zu, cap = %zu, last = %d\n", v.size, v.cap, v.data[v.size - 1]);
    vec_destroy(&v);
    return 0;
}
```

Vì sao tăng **gấp đôi** sức chứa? Nếu tăng thêm hằng số (ví dụ +4 mỗi lần), thêm `n` phần tử sẽ gọi `realloc` ~`n/4` lần và tổng chi phí sao chép là `O(n²)`. Tăng theo cấp số nhân làm chi phí trung bình mỗi lần `push` là `O(1)` (*amortized*). Đây là cách hoạt động của `std::vector` (C++) và `list` (Python).

## 9.6. Mẫu dọn dẹp trên đường lỗi

Khi hàm cấp phát nhiều tài nguyên, dùng `goto cleanup` để mọi đường thoát đều dọn dẹp:

```c
int load_data(const char *path, char **out_buf, size_t *out_len) {
    int rc = -1;
    FILE *f = NULL;
    char *buf = NULL;

    f = fopen(path, "rb");
    if (!f) goto done;

    if (fseek(f, 0, SEEK_END) != 0) goto done;
    long len = ftell(f);
    if (len < 0) goto done;
    rewind(f);

    buf = malloc((size_t)len + 1);
    if (!buf) goto done;
    if (fread(buf, 1, (size_t)len, f) != (size_t)len) goto done;
    buf[len] = '\0';

    *out_buf = buf;            // chuyển quyền sở hữu cho người gọi
    *out_len = (size_t)len;
    buf = NULL;                // đánh dấu "đã chuyển đi" để không free ở dưới
    rc = 0;

done:
    free(buf);                 // NULL nếu đã chuyển đi -> an toàn
    if (f) fclose(f);
    return rc;
}
```

Điểm khéo: đặt `buf = NULL` sau khi giao cho người gọi để đoạn dọn dẹp cuối không `free` nhầm.

## 9.7. Ví dụ hoàn chỉnh: cây nhị phân tìm kiếm

Một cấu trúc động điển hình: mỗi nút được cấp phát riêng; việc giải phóng phải duyệt cả cây.

```c
// tree.c
#include <stdio.h>
#include <stdlib.h>

typedef struct Node {
    int key;
    struct Node *left, *right;
} Node;

static Node *node_new(int key) {
    Node *n = malloc(sizeof *n);
    if (!n) return NULL;
    n->key = key;
    n->left = n->right = NULL;
    return n;
}

// Chèn khóa vào cây; trả về gốc mới (có thể là cùng gốc cũ). Trả NULL nếu hết bộ nhớ ở gốc.
Node *tree_insert(Node *root, int key) {
    if (root == NULL) return node_new(key);
    if (key < root->key) {
        Node *r = tree_insert(root->left, key);
        if (r == NULL) return root;                 // hết bộ nhớ: bỏ qua, giữ nguyên cây
        root->left = r;
    } else if (key > root->key) {
        Node *r = tree_insert(root->right, key);
        if (r == NULL) return root;
        root->right = r;
    }                                                // key trùng: không làm gì
    return root;
}

int tree_contains(const Node *root, int key) {
    while (root) {
        if (key == root->key) return 1;
        root = key < root->key ? root->left : root->right;
    }
    return 0;
}

// Duyệt trung thứ tự: in các khóa tăng dần
void tree_print(const Node *root) {
    if (!root) return;
    tree_print(root->left);
    printf("%d ", root->key);
    tree_print(root->right);
}

// Giải phóng: phải free con trước, rồi mới free cha (thứ tự hậu tố)
void tree_free(Node *root) {
    if (!root) return;
    tree_free(root->left);
    tree_free(root->right);
    free(root);
}

int main(void) {
    int keys[] = {50, 30, 70, 20, 40, 60, 80, 30};
    Node *root = NULL;
    for (size_t i = 0; i < sizeof keys / sizeof keys[0]; i++)
        root = tree_insert(root, keys[i]);

    tree_print(root);                                   // 20 30 40 50 60 70 80
    printf("\ncontains 60: %d, contains 65: %d\n", tree_contains(root, 60), tree_contains(root, 65));

    tree_free(root);
    return 0;
}
```

Sơ đồ cây được tạo:

```text
          50
        /    \
      30      70
     /  \    /  \
   20   40  60   80
```

Nếu `tree_free` giải phóng cha **trước**, ta sẽ mất địa chỉ hai con và rò rỉ toàn bộ. Kiểm tra bằng:

```bash
gcc -std=c11 -g -fsanitize=address,undefined -o tree tree.c && ./tree
```

Kết quả không có báo cáo lỗi/rò rỉ nghĩa là quản lý bộ nhớ đúng.

## 9.8. Kỹ thuật phòng thủ

1. **Gán `NULL` sau `free`** (dùng macro nếu muốn):
   ```c
   #define SAFE_FREE(p) do { free(p); (p) = NULL; } while (0)
   ```
2. **Không tự viết tay `sizeof(kiểu)`**: dùng `sizeof *ptr`.
3. **Bọc `malloc` trong hàm kiểm tra** khi chương trình coi hết bộ nhớ là lỗi nghiêm trọng:
   ```c
   void *xmalloc(size_t n) {
       void *p = malloc(n);
       if (!p) { fprintf(stderr, "het bo nho\n"); exit(EXIT_FAILURE); }
       return p;
   }
   ```
4. **Xóa dữ liệu nhạy cảm** (mật khẩu, khóa) trước khi `free`: dùng `memset_s` / `explicit_bzero` (vì `memset` thường bị compiler tối ưu bỏ).
5. **Dùng `calloc`** khi cần bộ nhớ khởi tạo về 0 và kiểm tra tràn.
6. **Tránh cấp phát nhỏ lặp đi lặp lại trong vòng lặp nóng** (chậm, gây phân mảnh); cấp phát một lần rồi tái sử dụng, hoặc dùng *arena/pool allocator*.
7. **Chạy sanitizer trong CI** để bắt lỗi trước khi phát hành (chương 21).

## 9.9. Bảng tóm tắt lỗi

| Lỗi | Dấu hiệu | Công cụ | Cách sửa |
|---|---|---|---|
| Rò rỉ | RAM tăng dần; Valgrind `definitely lost` | Valgrind, LeakSanitizer | Mỗi `malloc` một `free` trên mọi đường thoát |
| Use-after-free | ASan `heap-use-after-free` | ASan, Valgrind | Gán NULL sau free; rõ ownership |
| Double free | Crash trong `free`; ASan `attempting double-free` | ASan | Một chủ duy nhất |
| Heap overflow | ASan `heap-buffer-overflow` | ASan | Tính đủ kích thước (+1 cho `'\0'`) |
| Đọc chưa khởi tạo | Kết quả ngẫu nhiên | Valgrind, MSan | `calloc`/khởi tạo trước |
| Free sai chỗ | ASan `attempting free on address which was not malloc()-ed` | ASan | Chỉ free con trỏ gốc từ malloc |
| Stack overflow | Segfault khi đệ quy sâu | ASan, gdb | Điều kiện dừng; dùng heap |

## 9.10. Tóm tắt

- Stack tự động và nhanh nhưng nhỏ; heap linh hoạt nhưng bạn phải `free`.
- `malloc` không khởi tạo; `calloc` khởi tạo 0; `realloc` dùng con trỏ tạm; `free(NULL)` an toàn.
- Lỗi chính: rò rỉ, use-after-free, double free, tràn heap, đọc chưa khởi tạo.
- **Ownership rõ ràng** là chìa khóa: mỗi khối một chủ, ghi trong tài liệu hàm.
- Dùng **AddressSanitizer** và **Valgrind** như một phần bình thường của quy trình, không chỉ khi gặp lỗi.

## 9.11. Bài tập

1. Chạy `leak_demo.c` với Valgrind và ASan. Đọc báo cáo, tìm dòng gây rò rỉ, sửa và xác nhận báo cáo sạch.
2. Viết chương trình tái hiện lần lượt: use-after-free, double free, heap overflow (off-by-one). Chạy với ASan và ghi lại thông báo của từng lỗi.
3. Viết hàm `char *str_dup(const char *s)` và `char *str_concat(const char *a, const char *b)`. Ghi rõ ai phải `free`.
4. Mở rộng `Vec` với `vec_pop`, `vec_get`, `vec_insert(v, index, value)`, `vec_remove(v, index)`. Viết test và chạy với ASan.
5. Cài đặt `tree_delete` (xóa một khóa khỏi cây nhị phân tìm kiếm) và `tree_height`. Kiểm tra bằng Valgrind rằng không có rò rỉ.
6. Viết chương trình đọc toàn bộ một file văn bản vào một bộ đệm động (dùng `fread` + `realloc` tăng dần) và in số dòng.
7. Viết bộ cấp phát *arena* đơn giản: xin một khối lớn một lần, `arena_alloc(n)` cắt dần từ khối đó, `arena_free_all()` trả lại tất cả một lần. Thảo luận khi nào nó hữu ích.
8. (Thử thách) Viết wrapper `tracked_malloc`/`tracked_free` đếm số lần cấp phát/giải phóng và tổng byte, in báo cáo khi thoát (`atexit`), để tự phát hiện rò rỉ mà không cần công cụ ngoài.

Mã nguồn mẫu: /code/chapter-09
