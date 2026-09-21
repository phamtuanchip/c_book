# Chương 13 — Xử lý lỗi & ngoại lệ trong C

Mục tiêu chương:

- Hiểu cách báo lỗi hiệu quả trong C: mã trả về, errno, perror, strerror và assert.
- Thiết kế API có kiểm tra lỗi rõ ràng và logging để dễ debug.

1. Kiểm tra giá trị trả về và errno

- Luôn kiểm tra giá trị trả về của hàm I/O và hệ thống (fopen, malloc, read, write).
- errno (từ <errno.h>) chứa mã lỗi khi hàm hệ thống trả về lỗi; dùng perror hoặc strerror(errno) để in thông báo có nghĩa.

Ví dụ:
FILE *f = fopen(path, "r");
if (!f) { perror("fopen"); /* xử lý lỗi */ }

2. assert vs runtime error handling

- assert() (từ <assert.h>) dùng để kiểm tra giả thiết nội bộ khi debug; không dùng assert cho kiểm soát luồng hoặc xử lý lỗi từ input.
- assert bị loại bỏ khi biên dịch với NDEBUG; dùng return codes và logging cho runtime.

3. Thiết kế mã lỗi và API

- Trả về int/error enum hoặc struct { int code; char *msg } cho các hàm phức tạp.
- Ví dụ:
typedef enum { ERR_OK = 0, ERR_IO = 1, ERR_OOM = 2 } error_t;
error_t read_config(const char *path);

- Document ownership: ai chịu free chuỗi lỗi, ai trả lỗi.

4. Logging và debug macros

- Dùng macro LOG(level, fmt, ...) để chuẩn hoá thông báo. Ví dụ:
#define LOG_ERR(fmt, ...) fprintf(stderr, "ERROR: " fmt "\n", ##__VA_ARGS__)
- Giữ logging nhẹ ở production hoặc điều khiển bằng macro #ifdef DEBUG.

5. Best practices

- Kiểm tra mọi return từ hàm có thể fail.
- Trả mã lỗi rõ ràng, tránh dùng -1/0 nếu không có ý nghĩa.
- Thử nghiệm: viết test cản lỗi (fault injection) để đảm bảo chương trình xử lý hỏng hóc.

Bài tập

- Viết wrapper safe_fopen(path, mode, error_buf, bufsize) — trả NULL on error và ghi thông báo người dùng vào error_buf.
- Viết chương trình demo: cố ý gây lỗi I/O (không tồn tại file) và hiển thị thông báo hữu ích.

Ghi chú: kèm ví dụ trong /code/chapter-13 bao gồm safe_fopen và tests.