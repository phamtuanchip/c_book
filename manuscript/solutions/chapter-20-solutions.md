# Chương 20 — Lời giải bài tập

Mã server hoàn chỉnh: `code/chapter-20/server.c`. Dưới đây là các đoạn thay đổi cho từng bài (chèn vào `server.c`).

## Bài 1: quan sát bằng `curl -v`

`curl -v http://127.0.0.1:8080/` hiển thị dòng `> GET / HTTP/1.1` (yêu cầu) và `< HTTP/1.1 200 OK`, `< Content-Length: ...` (phản hồi). Kiểm tra: `wc -c www/index.html` phải bằng `Content-Length`. `curl -I` gửi `HEAD`: cùng header và độ dài nhưng **không có body**.

## Bài 2: `/api/echo?msg=...` với thoát ký tự JSON

```c
// json_escape.c
#include <stdio.h>
#include <stddef.h>

/* Ghi bản thoát ký tự JSON của src vào dst (cap byte). Trả 0 nếu vừa, -1 nếu bị cắt. */
static int json_escape(const char *src, char *dst, size_t cap) {
    size_t w = 0;
    for (const unsigned char *p = (const unsigned char *)src; *p; p++) {
        char tmp[8];
        size_t n;
        switch (*p) {
            case '"':  n = (size_t)snprintf(tmp, sizeof tmp, "\\\""); break;
            case '\\': n = (size_t)snprintf(tmp, sizeof tmp, "\\\\"); break;
            case '\n': n = (size_t)snprintf(tmp, sizeof tmp, "\\n");  break;
            case '\r': n = (size_t)snprintf(tmp, sizeof tmp, "\\r");  break;
            case '\t': n = (size_t)snprintf(tmp, sizeof tmp, "\\t");  break;
            default:
                if (*p < 0x20) n = (size_t)snprintf(tmp, sizeof tmp, "\\u%04x", *p);   // ký tự điều khiển
                else { tmp[0] = (char)*p; tmp[1] = '\0'; n = 1; }                       // byte UTF-8 giữ nguyên
        }
        if (w + n + 1 > cap) { if (cap) dst[w] = '\0'; return -1; }
        for (size_t i = 0; i < n; i++) dst[w++] = tmp[i];
    }
    if (cap) dst[w] = '\0';
    return 0;
}

int main(void) {
    char out[128];
    json_escape("say \"hi\"\n\\", out, sizeof out);
    printf("{\"msg\":\"%s\"}\n", out);                 // {"msg":"say \"hi\"\n\\"}
    return 0;
}
```

Trong route: lấy `rq->query`, tìm khóa `msg=`, **giải mã `%XX`** (dùng `url_decode` cho chuỗi query), rồi `json_escape` vào bộ đệm `body`.

## Bài 3: cấm file/thư mục bắt đầu bằng `.`

Trong `path_is_unsafe`, thêm: một thành phần đường dẫn (giữa hai dấu `/`) bắt đầu bằng `.` → cấm:

```c
for (const char *s = p; *s; s++)
    if (*s == '/' && s[1] == '.') return 1;        // ".git", ".env", cũng như ".." và "."
```

Gọi trước bước ghép đường dẫn để trả `403`.

## Bài 4: `Last-Modified` và `304`

Thêm vào `serve_file`: sau `stat`, định dạng `st.st_mtime` theo RFC 7231 bằng `gmtime_r` + `strftime("%a, %d %b %Y %H:%M:%S GMT")`; gửi header `Last-Modified`. Đọc header `If-Modified-Since` từ bộ đệm yêu cầu (tìm không phân biệt hoa/thường bằng `strcasestr` hoặc duyệt từng dòng), `strptime` về `time_t`; nếu `st.st_mtime <= since` trả `304 Not Modified` không có body.

## Bài 5: giới hạn tốc độ theo IP (token bucket)

```c
// ratelimit.c
#define _POSIX_C_SOURCE 200809L
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>

#define SLOTS 1024

typedef struct { uint32_t ip; double tokens; double last; } Bucket;
static Bucket g_buckets[SLOTS];
static pthread_mutex_t g_rl_lock = PTHREAD_MUTEX_INITIALIZER;

static double now_s(void) { struct timespec t; clock_gettime(CLOCK_MONOTONIC, &t); return (double)t.tv_sec + (double)t.tv_nsec * 1e-9; }

/* Trả 1 nếu cho phép, 0 nếu vượt hạn mức (trả 429). Hạn mức: `rate` yêu cầu/giây, tối đa `burst`. */
static int rate_allow(uint32_t ip, double rate, double burst) {
    int ok;
    pthread_mutex_lock(&g_rl_lock);                       // nhiều worker cùng gọi -> phải khóa
    Bucket *b = &g_buckets[ip % SLOTS];
    double t = now_s();
    if (b->ip != ip) { b->ip = ip; b->tokens = burst; b->last = t; }     // đụng độ/khởi tạo (đơn giản hóa)
    b->tokens += (t - b->last) * rate;
    if (b->tokens > burst) b->tokens = burst;
    b->last = t;
    ok = b->tokens >= 1.0;
    if (ok) b->tokens -= 1.0;
    pthread_mutex_unlock(&g_rl_lock);
    return ok;
}

int main(void) {
    int allowed = 0;
    for (int i = 0; i < 20; i++) allowed += rate_allow(0x7f000001u, 5.0, 10.0);
    printf("%d/20 duoc phep\n", allowed);                 // ~10 (burst) do 20 yêu cầu tới ngay lập tức
    return 0;
}
```

Trong `main` của server, lấy IP từ `accept(srv, (struct sockaddr *)&cli, &len)` và gọi `rate_allow(ntohl(cli.sin_addr.s_addr), ...)` trước khi đưa vào hàng đợi; nếu `0` thì trả `429 Too Many Requests` và đóng.

## Bài 6: keep-alive

Đổi `handle_connection` thành vòng lặp: đọc yêu cầu → xử lý → nếu client gửi `Connection: keep-alive` (HTTP/1.1 mặc định là keep-alive) và số yêu cầu < 100 thì **không** đóng và quay lại đọc yêu cầu tiếp theo với `SO_RCVTIMEO = 5s` (hết hạn = nhàn rỗi → đóng). Header phản hồi phải đổi thành `Connection: keep-alive` và `Content-Length` **bắt buộc chính xác** (client dựa vào đó để biết phản hồi kết thúc). Cẩn thận dữ liệu của yêu cầu kế tiếp có thể đã nằm sẵn trong bộ đệm sau `\r\n\r\n` (pipelining).

## Bài 7: số worker và hiệu năng

Với `wrk -t2 -c100 -d10s`, đồ thị thường tăng tuyến tính đến ~số nhân CPU rồi bão hòa, sau đó có thể **giảm nhẹ** do chi phí chuyển ngữ cảnh và tranh chấp khóa hàng đợi. Với file nhỏ được cache bởi hệ điều hành, server bị giới hạn bởi CPU/syscall; với file lớn, bởi băng thông đĩa/mạng. Ghi cả tỷ lệ lỗi (`503` khi hàng đợi đầy).

## Bài 8: event loop bằng `epoll`

Cấu trúc: mỗi kết nối có struct `Conn { fd; state (READ_HDR / WRITE_RESP); buf; len; sent; }`. `epoll_wait` trả các fd sẵn sàng: `EPOLLIN` → `recv` (non-blocking) cho đến `EAGAIN`, thấy `\r\n\r\n` thì dựng phản hồi vào bộ đệm ghi và đổi sang theo dõi `EPOLLOUT`; `EPOLLOUT` → `send` cho đến hết hoặc `EAGAIN`. File lớn: dùng `sendfile`. Đo 10.000 kết nối nhàn rỗi: bộ nhớ mỗi kết nối ~vài KB thay vì stack luồng ~MB.

## Bài 9: fuzz `parse_request` và `url_decode`

Tách hai hàm này thành `http_parse.c` (không phụ thuộc socket) để có thể liên kết với fuzz target:

```c
int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    char buf[8192];
    if (size >= sizeof buf) size = sizeof buf - 1;
    memcpy(buf, data, size);
    buf[size] = '\0';
    Request rq;
    (void)parse_request(buf, &rq);
    return 0;
}
```

Chạy `clang -fsanitize=fuzzer,address,undefined`. Những lỗi hay gặp khi fuzz kiểu này: đọc `r[2]` khi `r[1]` là `'\0'` (đã xử lý ở `url_decode`), `sscanf` với `%s` thiếu độ rộng, và cắt chuỗi không có `'\0'`.

## Bài 10: upload file an toàn

Nguyên tắc: (1) đọc `Content-Length`, **từ chối nếu > 1 MB** (`413`); (2) đọc đúng ngần ấy byte vào bộ đệm hoặc file tạm, không tin độ dài lớn hơn thực tế; (3) **tên file do server tạo** (`mkstemp` hoặc ngẫu nhiên từ `getrandom`), bỏ qua tên client gửi; (4) mở bằng `open(path, O_WRONLY | O_CREAT | O_EXCL | O_NOFOLLOW, 0600)`; (5) không phục vụ lại file đã upload với `Content-Type` do client chọn (dùng `application/octet-stream` + `Content-Disposition: attachment`). Kiểm thử bằng `curl -F file=@x.bin` (hoặc `--data-binary`) và các trường hợp: quá lớn, thiếu `Content-Length`, kết nối đứt giữa chừng (file tạm phải bị xóa).
