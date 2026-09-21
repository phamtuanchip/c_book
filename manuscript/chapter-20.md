# Chương 20 — Project: Web server đơn giản

## Mục tiêu chương

- Xây một **web server HTTP/1.1 nhỏ** bằng C: phục vụ **file tĩnh** và vài **route động** (JSON API).
- Vận dụng kiến thức socket (chương 16), luồng (chương 15), file I/O (chương 12), xử lý lỗi (chương 13) và bảo mật (chương 18).
- Hiểu cấu trúc một yêu cầu/phản hồi HTTP và cách **phân tích** yêu cầu an toàn.
- So sánh các kiến trúc: **thread-per-connection**, **thread pool**, **event loop**; cài đặt thread pool có hàng đợi giới hạn.
- Ngăn **directory traversal**, giới hạn kích thước/thời gian, tắt server **êm (graceful shutdown)**.
- Kiểm thử bằng `curl`, `ab`/`wrk`, và sanitizer.

> **Môi trường:** mã dùng POSIX (Linux, macOS, WSL). Trên Windows, chạy trong WSL hoặc chuyển sang Winsock theo bảng ở chương 16. Đây là **server học tập**: chưa có TLS, chưa xử lý đầy đủ HTTP; đừng phơi ra Internet công cộng.

## 20.1. HTTP trong 5 phút

HTTP là giao thức **văn bản** kiểu **yêu cầu–phản hồi** chạy trên TCP (mặc định cổng 80). Client gửi một **yêu cầu**, server trả một **phản hồi**.

### Yêu cầu

```text
GET /index.html HTTP/1.1\r\n            ← dòng yêu cầu: PHƯƠNG THỨC  ĐƯỜNG DẪN  PHIÊN BẢN
Host: localhost:8080\r\n                ← các header, mỗi dòng "Tên: giá trị"
User-Agent: curl/8.0\r\n
Accept: */*\r\n
\r\n                                    ← dòng trống kết thúc phần header
```

### Phản hồi

```text
HTTP/1.1 200 OK\r\n                     ← dòng trạng thái: PHIÊN BẢN  MÃ  LÝ DO
Content-Type: text/html\r\n             ← header
Content-Length: 44\r\n
Connection: close\r\n
\r\n                                    ← hết header
<html><body>Xin chao!</body></html>     ← body (đúng 44 byte)
```

- Mỗi dòng kết thúc bằng **`\r\n`** (CRLF), không chỉ `\n`.
- `Content-Length` cho client biết body dài bao nhiêu byte — đây chính là **đóng khung** (chương 16) của HTTP.
- `Connection: close` báo server sẽ đóng kết nối sau khi trả lời (đơn giản hơn *keep-alive*).

### Các mã trạng thái cần thiết

| Mã | Ý nghĩa | Khi nào |
|---|---|---|
| `200 OK` | thành công | trả file/JSON |
| `400 Bad Request` | yêu cầu sai cú pháp | không phân tích được dòng yêu cầu |
| `403 Forbidden` | bị cấm | đường dẫn nguy hiểm (`..`) |
| `404 Not Found` | không có | file không tồn tại |
| `405 Method Not Allowed` | phương thức không hỗ trợ | POST/PUT... |
| `414 URI Too Long` | đường dẫn quá dài | |
| `431 Request Header Fields Too Large` | header quá lớn | |
| `500 Internal Server Error` | lỗi phía server | |
| `503 Service Unavailable` | quá tải | hàng đợi đầy |

Phạm vi ta hỗ trợ: phương thức **GET** và **HEAD** (HEAD như GET nhưng không có body), header đọc nhưng bỏ qua, không hỗ trợ body của yêu cầu, không nén, không keep-alive.

Thử một server có sẵn để quan sát: `curl -v http://example.com/` hiện đầy đủ yêu cầu (`>`) và phản hồi (`<`).

## 20.2. Kiến trúc

Ba lựa chọn (chương 16):

| Kiến trúc | Ý tưởng | Ưu | Nhược |
|---|---|---|---|
| **Thread-per-connection** | Mỗi kết nối một luồng mới | Rất dễ viết | Tốn tài nguyên khi nhiều kết nối; tạo/hủy luồng liên tục; dễ bị cạn tài nguyên |
| **Thread pool** | `N` luồng cố định lấy kết nối từ hàng đợi | Giới hạn tài nguyên; dùng lại luồng; vẫn dễ viết | Kết nối chậm chiếm một luồng |
| **Event loop** (`epoll`) | Một (hoặc vài) luồng theo dõi mọi socket, không chặn | Mở rộng tới hàng chục nghìn kết nối | Mã phức tạp (máy trạng thái) |

Ta chọn **thread pool**: cân bằng giữa đơn giản và an toàn tài nguyên. Sơ đồ:

```text
                    ┌──────────────────────┐
  client ──┐        │  luồng chính         │       ┌─ worker 1 ─┐
  client ──┼──────► │  poll + accept()     │──┐    │ lấy fd,    │
  client ──┘        │  (kiểm tra cờ dừng)  │  │    │ xử lý,     │
                    └──────────────────────┘  │    │ close(fd)  │
                                              ▼    └────────────┘
                                     ┌────────────────┐   ┌─ worker 2 ─┐
                                     │ hàng đợi fd    │──►│    ...     │
                                     │ (giới hạn 64)  │   └────────────┘
                                     └────────────────┘   ┌─ worker N ─┐
                                                          └────────────┘
```

Luồng chính chỉ **chấp nhận kết nối** và bỏ `fd` vào hàng đợi; **worker** làm phần việc nặng. Nếu hàng đợi **đầy**, luồng chính trả `503` ngay và đóng kết nối — **áp lực ngược (backpressure)** thay vì để hàng đợi phình vô hạn.

## 20.3. Khung mã và các tiện ích

Cấu trúc thư mục:

```text
webserver/
├── server.c
├── www/
│   ├── index.html
│   └── style.css
└── Makefile
```

`www/index.html` để thử:

```html
<!doctype html>
<html><head><meta charset="utf-8"><title>C server</title><link rel="stylesheet" href="/style.css"></head>
<body><h1>Xin chào từ máy chủ C!</h1><p>Giờ hiện tại: <a href="/api/time">/api/time</a></p></body></html>
```

### Phần đầu: include, cấu hình, gửi đủ dữ liệu

```c
// server.c
#define _POSIX_C_SOURCE 200809L
#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <poll.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#define MAX_HEADER   8192          // tối đa cho toàn bộ phần header của yêu cầu
#define QUEUE_CAP    64            // số kết nối chờ tối đa
#define IO_TIMEOUT_S 5             // giây chờ tối đa cho mỗi lần recv/send
#define PATH_MAX_LOCAL 4096        // bộ đệm cho realpath() (PATH_MAX không phải lúc nào cũng có)

static const char *g_root = "./www";
static volatile sig_atomic_t g_stop = 0;

// Gửi ĐỦ n byte. Dùng MSG_NOSIGNAL để không bị SIGPIPE nếu client đóng giữa chừng.
static int send_all(int fd, const void *data, size_t n) {
    const char *p = data;
    while (n > 0) {
        ssize_t r = send(fd, p, n, MSG_NOSIGNAL);
        if (r < 0) {
            if (errno == EINTR) continue;
            return -1;                          // gồm cả EAGAIN do hết thời gian chờ
        }
        p += r;
        n -= (size_t)r;
    }
    return 0;
}
```

### Ghi log

```c
// Ghi 1 dòng log cho mỗi yêu cầu: thời gian, phương thức, đường dẫn, mã trạng thái, số byte body.
static void log_request(const char *method, const char *path, int status, long bytes) {
    char ts[32];
    time_t now = time(NULL);
    struct tm tmv;
    localtime_r(&now, &tmv);
    strftime(ts, sizeof ts, "%Y-%m-%d %H:%M:%S", &tmv);
    fprintf(stderr, "%s %-4s %-30.30s %d %ld\n", ts, method, path, status, bytes);   // một lệnh fprintf -> không bị xen dòng
}
```

Cắt đường dẫn khi in (`%-30.30s`) để **kẻ tấn công không thể làm nổ log** bằng đường dẫn dài hoặc chứa ký tự điều khiển; thực tế nên lọc cả ký tự xuống dòng để chống **log injection**.

## 20.4. Phản hồi và loại nội dung (MIME)

### Gửi header

```c
static const char *reason_phrase(int status) {
    switch (status) {
        case 200: return "OK";
        case 400: return "Bad Request";
        case 403: return "Forbidden";
        case 404: return "Not Found";
        case 405: return "Method Not Allowed";
        case 414: return "URI Too Long";
        case 431: return "Request Header Fields Too Large";
        case 503: return "Service Unavailable";
        default:  return "Internal Server Error";
    }
}

static int send_header(int fd, int status, const char *ctype, size_t length) {
    char h[512];
    int n = snprintf(h, sizeof h,
        "HTTP/1.1 %d %s\r\n"
        "Content-Type: %s\r\n"
        "Content-Length: %zu\r\n"
        "Connection: close\r\n"
        "X-Content-Type-Options: nosniff\r\n"
        "\r\n",
        status, reason_phrase(status), ctype, length);
    if (n < 0 || (size_t)n >= sizeof h) return -1;
    return send_all(fd, h, (size_t)n);
}

// Trả một phản hồi ngắn (lỗi hoặc JSON). Trả số byte body để ghi log.
static long send_text(int fd, int status, const char *ctype, const char *body, int head_only) {
    size_t len = strlen(body);
    if (send_header(fd, status, ctype, len) != 0) return 0;
    if (!head_only && send_all(fd, body, len) != 0) return 0;
    return head_only ? 0 : (long)len;
}
```

### Loại nội dung theo đuôi file

Trình duyệt dựa vào `Content-Type` để biết cách hiển thị. Gửi sai (ví dụ CSS với `text/plain`) khiến trình duyệt từ chối áp dụng.

```c
static const char *mime_type(const char *path) {
    static const struct { const char *ext, *type; } table[] = {
        {".html", "text/html; charset=utf-8"},
        {".htm",  "text/html; charset=utf-8"},
        {".css",  "text/css; charset=utf-8"},
        {".js",   "application/javascript; charset=utf-8"},
        {".json", "application/json"},
        {".txt",  "text/plain; charset=utf-8"},
        {".png",  "image/png"},
        {".jpg",  "image/jpeg"},
        {".jpeg", "image/jpeg"},
        {".gif",  "image/gif"},
        {".svg",  "image/svg+xml"},
        {".ico",  "image/x-icon"},
        {".pdf",  "application/pdf"},
    };
    const char *dot = strrchr(path, '.');
    if (dot) {
        for (size_t i = 0; i < sizeof table / sizeof table[0]; i++)
            if (strcasecmp(dot, table[i].ext) == 0) return table[i].type;
    }
    return "application/octet-stream";            // không biết: tải về thay vì hiển thị
}
```

## 20.5. Phân tích yêu cầu

Đọc yêu cầu **có giới hạn** và **theo giao thức**: dữ liệu TCP đến từng mảnh nên phải lặp tới khi thấy `\r\n\r\n` (chương 16).

```c
typedef struct {
    char method[8];
    char target[1024];          // đường dẫn đã bỏ query, đã giải mã %XX
    char version[16];
    char query[256];            // phần sau '?' (nếu có)
} Request;

// Đọc header vào buf. Trả độ dài (>0) khi thấy "\r\n\r\n"; 0 nếu đối phương đóng/hết thời gian;
// -1 nếu header vượt giới hạn.
static ssize_t read_header(int fd, char *buf, size_t cap) {
    size_t len = 0;
    for (;;) {
        if (len + 1 >= cap) return -1;                         // header quá lớn
        ssize_t n = recv(fd, buf + len, cap - 1 - len, 0);
        if (n < 0) { if (errno == EINTR) continue; return 0; } // lỗi hoặc hết thời gian
        if (n == 0) return 0;                                  // đối phương đóng
        len += (size_t)n;
        buf[len] = '\0';
        if (strstr(buf, "\r\n\r\n") != NULL) return (ssize_t)len;
    }
}
```

`strstr` dừng ở byte NUL đầu tiên; dữ liệu độc hại có thể chèn NUL, nhưng lúc đó `strstr` chỉ thấy phần đầu — ta vẫn không đọc quá giới hạn `cap`, nên an toàn. (Server thật dùng cách quét tìm `\r\n\r\n` trên `len` byte thực nhận.)

### Giải mã phần trăm (URL decoding)

URL mã hóa ký tự đặc biệt dạng `%XX`: `/a%20b.txt` là `/a b.txt`. Phải **giải mã trước khi kiểm tra an toàn** — nếu không, kẻ tấn công gửi `%2e%2e/` (là `../`) để lách bộ lọc.

```c
static int hexval(int c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

// Giải mã tại chỗ. Trả 0 nếu hợp lệ; -1 nếu %XX hỏng hoặc sinh ra byte NUL.
static int url_decode(char *s) {
    char *w = s;
    for (const char *r = s; *r; r++) {
        if (*r == '%') {
            int hi = hexval((unsigned char)r[1]);
            int lo = hi >= 0 ? hexval((unsigned char)r[2]) : -1;    // không đọc r[2] nếu r[1] đã lỗi
            if (hi < 0 || lo < 0) return -1;
            int v = hi * 16 + lo;
            if (v == 0) return -1;                                  // %00: chèn NUL để cắt chuỗi — cấm
            *w++ = (char)v;
            r += 2;
        } else {
            *w++ = *r;
        }
    }
    *w = '\0';
    return 0;
}
```

### Phân tích dòng yêu cầu

```c
// Trả 0 nếu OK, hoặc mã lỗi HTTP (400/414...).
static int parse_request(const char *buf, Request *rq) {
    char target[1300];
    memset(rq, 0, sizeof *rq);

    // độ rộng tối đa trong định dạng ngăn tràn: 7 + '\0' = 8; 1299 + '\0'; 15 + '\0'
    if (sscanf(buf, "%7s %1299s %15s", rq->method, target, rq->version) != 3) return 400;
    if (strncmp(rq->version, "HTTP/1.", 7) != 0) return 400;

    char *q = strchr(target, '?');
    if (q) { *q++ = '\0'; snprintf(rq->query, sizeof rq->query, "%s", q); }

    if (strlen(target) >= sizeof rq->target) return 414;
    if (target[0] != '/') return 400;
    snprintf(rq->target, sizeof rq->target, "%s", target);

    if (url_decode(rq->target) != 0) return 400;
    return 0;
}
```

Lưu ý phòng thủ ngay từ dòng `sscanf`: mọi `%s` đều có **giới hạn độ rộng** (chương 4, 18). Giới hạn kích thước từng trường để tránh tràn và từ chối yêu cầu quá dài bằng `414`.

## 20.6. Phục vụ file tĩnh an toàn

Đây là chỗ dễ có lỗ hổng **directory traversal** nhất. Nếu server chỉ nối `"./www" + đường_dẫn` mà không kiểm tra, yêu cầu `GET /../../etc/passwd` sẽ đọc file ngoài thư mục web.

### Các lớp phòng thủ

1. **Giải mã trước, kiểm tra sau** (đã làm ở `url_decode`).
2. **Từ chối** đường dẫn chứa thành phần `..`, ký tự `\`, hoặc byte điều khiển.
3. **Chỉ phục vụ file thường** (`S_ISREG`) — không phải thư mục, thiết bị, FIFO.
4. **Mạnh nhất:** dùng `realpath()` để phân giải symlink và `..`, rồi **kiểm tra kết quả nằm trong thư mục gốc**.

```c
// Trả 1 nếu đường dẫn (đã giải mã) chứa thành phần ".." hoặc ký tự nguy hiểm.
static int path_is_unsafe(const char *p) {
    if (strchr(p, '\\')) return 1;
    for (const unsigned char *c = (const unsigned char *)p; *c; c++)
        if (*c < 0x20 || *c == 0x7f) return 1;              // ký tự điều khiển
    // tìm thành phần ".." đứng riêng giữa các dấu '/'
    for (const char *s = p; (s = strstr(s, "..")) != NULL; s += 2) {
        int at_start = (s == p) || (s[-1] == '/');
        int at_end   = (s[2] == '\0') || (s[2] == '/');
        if (at_start && at_end) return 1;
    }
    return 0;
}
```

Hàm phục vụ file:

```c
static long serve_file(int fd, const Request *rq, int head_only, int *status_out) {
    if (path_is_unsafe(rq->target)) {
        *status_out = 403;
        return send_text(fd, 403, "text/plain", "403 Forbidden\n", head_only);
    }

    char rel[1100];
    snprintf(rel, sizeof rel, "%s", rq->target);
    size_t rl = strlen(rel);
    if (rel[rl - 1] == '/' && rl + 10 < sizeof rel) strcat(rel, "index.html");   // "/" -> "/index.html" (đã kiểm tra chỗ)

    char full[1300];
    int n = snprintf(full, sizeof full, "%s%s", g_root, rel);
    if (n < 0 || (size_t)n >= sizeof full) {
        *status_out = 414;
        return send_text(fd, 414, "text/plain", "414 URI Too Long\n", head_only);
    }

    // Lớp phòng thủ cuối: phân giải symlink và kiểm tra vẫn nằm trong thư mục gốc
    char resolved[PATH_MAX_LOCAL], root_real[PATH_MAX_LOCAL];
    if (realpath(full, resolved) == NULL || realpath(g_root, root_real) == NULL) {
        *status_out = 404;
        return send_text(fd, 404, "text/plain", "404 Not Found\n", head_only);
    }
    size_t root_len = strlen(root_real);
    if (strncmp(resolved, root_real, root_len) != 0 ||
        (resolved[root_len] != '/' && resolved[root_len] != '\0')) {
        *status_out = 403;
        return send_text(fd, 403, "text/plain", "403 Forbidden\n", head_only);
    }

    struct stat st;
    if (stat(resolved, &st) != 0 || !S_ISREG(st.st_mode)) {
        *status_out = 404;
        return send_text(fd, 404, "text/plain", "404 Not Found\n", head_only);
    }

    FILE *f = fopen(resolved, "rb");
    if (!f) {
        *status_out = 404;
        return send_text(fd, 404, "text/plain", "404 Not Found\n", head_only);
    }

    *status_out = 200;
    if (send_header(fd, 200, mime_type(resolved), (size_t)st.st_size) != 0) { fclose(f); return 0; }

    long sent = 0;
    if (!head_only) {
        char chunk[16384];
        size_t got;
        while ((got = fread(chunk, 1, sizeof chunk, f)) > 0) {
            if (send_all(fd, chunk, got) != 0) break;              // client đã đi -> dừng
            sent += (long)got;
        }
    }
    fclose(f);
    return sent;
}
```

Điểm nhấn về kiểm tra thư mục gốc: so sánh **tiền tố** phải kèm điều kiện ký tự kế là `/` hoặc kết thúc chuỗi. Nếu chỉ `strncmp`, thư mục `/srv/www-secret` sẽ "khớp" tiền tố `/srv/www`.

Thư mục `www` được đọc từng khối 16 KB nên một file lớn không chiếm nhiều RAM. Trên Linux có thể dùng `sendfile()` để kernel chép trực tiếp file → socket không qua bộ nhớ người dùng (nhanh hơn); ta dùng `fread`/`send` vì di động và dễ hiểu.

## 20.7. Router: route động

Route là ánh xạ **đường dẫn → hàm xử lý**. Kiểm tra bảng route **trước**, rồi mới rơi về file tĩnh.

```c
typedef long (*Handler)(int fd, const Request *rq, int head_only, int *status_out);

static long api_time(int fd, const Request *rq, int head_only, int *status_out) {
    (void)rq;
    time_t now = time(NULL);
    struct tm tmv;
    gmtime_r(&now, &tmv);
    char iso[32];
    strftime(iso, sizeof iso, "%Y-%m-%dT%H:%M:%SZ", &tmv);

    char body[128];
    snprintf(body, sizeof body, "{\"time\":\"%s\",\"unix\":%lld}\n", iso, (long long)now);
    *status_out = 200;
    return send_text(fd, 200, "application/json", body, head_only);
}

static long api_health(int fd, const Request *rq, int head_only, int *status_out) {
    (void)rq;
    *status_out = 200;
    return send_text(fd, 200, "application/json", "{\"status\":\"ok\"}\n", head_only);
}

static const struct { const char *path; Handler fn; } ROUTES[] = {
    { "/api/time",   api_time   },
    { "/api/health", api_health },
};

static Handler find_route(const char *path) {
    for (size_t i = 0; i < sizeof ROUTES / sizeof ROUTES[0]; i++)
        if (strcmp(ROUTES[i].path, path) == 0) return ROUTES[i].fn;
    return NULL;
}
```

Với JSON có dữ liệu do người dùng cung cấp (ví dụ `/api/echo?msg=...`), **bạn phải thoát ký tự đặc biệt** (`"`, `\`, ký tự điều khiển) khi chèn vào JSON; nếu không, kẻ tấn công phá cấu trúc JSON (*injection*). Bài tập 3 yêu cầu làm việc này.

## 20.8. Xử lý một kết nối

```c
static void handle_connection(int fd) {
    // Chống client chậm/treo chiếm mãi một worker
    struct timeval tv = { .tv_sec = IO_TIMEOUT_S, .tv_usec = 0 };
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof tv);
    setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof tv);

    char buf[MAX_HEADER];
    ssize_t hl = read_header(fd, buf, sizeof buf);
    if (hl == 0) return;                                        // đóng/hết thời gian: không cần trả lời
    if (hl < 0) {
        long b = send_text(fd, 431, "text/plain", "431 Header Too Large\n", 0);
        log_request("?", "?", 431, b);
        return;
    }

    Request rq;
    int perr = parse_request(buf, &rq);
    if (perr != 0) {
        long b = send_text(fd, perr, "text/plain", perr == 414 ? "414 URI Too Long\n" : "400 Bad Request\n", 0);
        log_request("?", "?", perr, b);
        return;
    }

    int head_only = (strcmp(rq.method, "HEAD") == 0);
    if (strcmp(rq.method, "GET") != 0 && !head_only) {
        long b = send_text(fd, 405, "text/plain", "405 Method Not Allowed\n", 0);
        log_request(rq.method, rq.target, 405, b);
        return;
    }

    int status = 500;
    long bytes;
    Handler h = find_route(rq.target);
    bytes = h ? h(fd, &rq, head_only, &status) : serve_file(fd, &rq, head_only, &status);
    log_request(rq.method, rq.target, status, bytes);
}
```

Quy trình: **giới hạn thời gian → đọc header có giới hạn → phân tích → kiểm tra phương thức → route/file → log**. Mỗi bước có đường thoát lỗi rõ ràng và trả mã HTTP phù hợp; không có `exit()` nào trong đường xử lý yêu cầu (một yêu cầu lỗi **không được** làm sập server).

## 20.9. Thread pool và hàng đợi có giới hạn

Ta dùng lại mẫu **bounded queue** của chương 15, chứa các `fd`.

```c
typedef struct {
    int    fds[QUEUE_CAP];
    size_t head, count;
    int    closed;
    pthread_mutex_t lock;
    pthread_cond_t  not_empty;
} FdQueue;

static FdQueue g_queue = { .lock = PTHREAD_MUTEX_INITIALIZER, .not_empty = PTHREAD_COND_INITIALIZER };

// Không chặn: trả 0 nếu thêm được, -1 nếu đầy (để luồng chính trả 503)
static int queue_try_put(FdQueue *q, int fd) {
    int rc = -1;
    pthread_mutex_lock(&q->lock);
    if (q->count < QUEUE_CAP) {
        q->fds[(q->head + q->count) % QUEUE_CAP] = fd;
        q->count++;
        rc = 0;
        pthread_cond_signal(&q->not_empty);
    }
    pthread_mutex_unlock(&q->lock);
    return rc;
}

// Chặn tới khi có fd; trả -1 nếu queue đã đóng và hết việc (worker phải thoát)
static int queue_get(FdQueue *q) {
    pthread_mutex_lock(&q->lock);
    while (q->count == 0 && !q->closed)
        pthread_cond_wait(&q->not_empty, &q->lock);
    if (q->count == 0) { pthread_mutex_unlock(&q->lock); return -1; }
    int fd = q->fds[q->head];
    q->head = (q->head + 1) % QUEUE_CAP;
    q->count--;
    pthread_mutex_unlock(&q->lock);
    return fd;
}

static void queue_close(FdQueue *q) {
    pthread_mutex_lock(&q->lock);
    q->closed = 1;
    pthread_cond_broadcast(&q->not_empty);            // đánh thức mọi worker để chúng thoát
    pthread_mutex_unlock(&q->lock);
}

static void *worker_main(void *arg) {
    (void)arg;
    int fd;
    while ((fd = queue_get(&g_queue)) >= 0) {
        handle_connection(fd);
        shutdown(fd, SHUT_WR);                         // báo "hết dữ liệu gửi" để client đọc đủ trước khi đóng
        close(fd);
    }
    return NULL;
}
```

## 20.10. `main`: khởi tạo, vòng lặp chấp nhận, tắt êm

```c
static void on_signal(int sig) { (void)sig; g_stop = 1; }     // chỉ đặt cờ (chương 13)

int main(int argc, char *argv[]) {
    int port     = argc > 1 ? atoi(argv[1]) : 8080;
    int nthreads = argc > 2 ? atoi(argv[2]) : 4;
    if (argc > 3) g_root = argv[3];
    if (port <= 0 || port > 65535 || nthreads < 1 || nthreads > 256) {
        fprintf(stderr, "Cach dung: %s [cong] [so_luong] [thu_muc_goc]\n", argv[0]);
        return 2;
    }

    // Tín hiệu: SIGINT/SIGTERM -> dừng êm. Không dùng SA_RESTART để poll() trả về EINTR ngay.
    struct sigaction sa;
    memset(&sa, 0, sizeof sa);
    sa.sa_handler = on_signal;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGINT,  &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
    signal(SIGPIPE, SIG_IGN);                                 // ghi vào socket đã đóng không giết server

    int srv = socket(AF_INET, SOCK_STREAM, 0);
    if (srv < 0) { perror("socket"); return 1; }
    int yes = 1;
    setsockopt(srv, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes);

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof addr);
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);            // CHỈ nhận từ máy này khi thử nghiệm
    addr.sin_port = htons((uint16_t)port);
    if (bind(srv, (struct sockaddr *)&addr, sizeof addr) < 0) { perror("bind"); return 1; }
    if (listen(srv, 128) < 0) { perror("listen"); return 1; }

    pthread_t *workers = calloc((size_t)nthreads, sizeof *workers);
    if (!workers) { perror("calloc"); return 1; }
    int started = 0;
    for (int i = 0; i < nthreads; i++) {
        if (pthread_create(&workers[i], NULL, worker_main, NULL) != 0) { perror("pthread_create"); break; }
        started++;
    }
    fprintf(stderr, "Server chay tai http://127.0.0.1:%d/ (%d luong, goc: %s). Nhan Ctrl+C de dung.\n",
            port, started, g_root);

    // Vòng lặp chấp nhận: poll với timeout 1 giây để định kỳ kiểm tra cờ dừng
    struct pollfd pfd = { .fd = srv, .events = POLLIN };
    while (!g_stop) {
        int r = poll(&pfd, 1, 1000);
        if (r < 0) { if (errno == EINTR) continue; perror("poll"); break; }
        if (r == 0) continue;                                 // hết thời gian, quay lại kiểm tra g_stop

        int fd = accept(srv, NULL, NULL);
        if (fd < 0) {
            if (errno == EINTR || errno == ECONNABORTED) continue;
            perror("accept");
            break;
        }
        if (queue_try_put(&g_queue, fd) != 0) {               // hàng đợi đầy: quá tải
            const char *msg = "HTTP/1.1 503 Service Unavailable\r\nContent-Length: 0\r\nConnection: close\r\n\r\n";
            send(fd, msg, strlen(msg), MSG_NOSIGNAL);
            close(fd);
            log_request("-", "-", 503, 0);
        }
    }

    // Tắt êm: ngừng nhận, để worker xử lý hết những kết nối đã vào hàng đợi, rồi join
    fprintf(stderr, "\nDang dung server...\n");
    close(srv);
    queue_close(&g_queue);
    for (int i = 0; i < started; i++) pthread_join(workers[i], NULL);
    free(workers);
    fprintf(stderr, "Da dung.\n");
    return 0;
}
```

Chú ý:

- **`INADDR_LOOPBACK`:** an toàn khi phát triển. Đổi thành `INADDR_ANY` chỉ khi bạn hiểu rủi ro.
- **`poll` với timeout 1 s:** vì `accept` chặn, nếu chỉ dùng `accept` thì khi nhận Ctrl+C có thể vẫn kẹt lại. Kiểm tra cờ định kỳ là cách đơn giản để dừng êm.
- **`signal(SIGPIPE, SIG_IGN)`** kèm `MSG_NOSIGNAL`: hai lớp bảo vệ.
- **Thứ tự tắt:** đóng socket lắng nghe → đóng queue → join worker (worker xử lý nốt việc rồi thoát khi `queue_get` trả -1).
- Không có biến toàn cục thay đổi ngoài `g_stop` và hàng đợi có khóa; `g_root` chỉ đọc sau khi khởi động, nên các worker đọc an toàn.

## 20.11. Biên dịch, chạy, kiểm thử

`Makefile`:

```make
CC      := gcc
CFLAGS  := -std=c11 -Wall -Wextra -Wpedantic -Wshadow -O2 -g -D_FORTIFY_SOURCE=2 -fstack-protector-strong
LDLIBS  := -pthread

server: server.c
	$(CC) $(CFLAGS) $< -o $@ $(LDLIBS)

asan: server.c
	$(CC) -std=c11 -Wall -Wextra -g -O1 -fsanitize=address,undefined -fno-omit-frame-pointer $< -o server_asan $(LDLIBS)

tsan: server.c
	$(CC) -std=c11 -Wall -Wextra -g -O1 -fsanitize=thread $< -o server_tsan $(LDLIBS)

clean:
	rm -f server server_asan server_tsan
```

Chạy và thử:

```bash
make && ./server 8080 4 ./www
```

Trong terminal khác:

```bash
curl -i http://127.0.0.1:8080/                    # index.html: 200, Content-Type: text/html
curl -i http://127.0.0.1:8080/style.css           # text/css
curl -i http://127.0.0.1:8080/api/time            # JSON
curl -I http://127.0.0.1:8080/                    # HEAD: chỉ header, Content-Length đúng, không body
curl -i http://127.0.0.1:8080/khong-co.html       # 404
curl -i -X POST http://127.0.0.1:8080/            # 405
curl -i --path-as-is 'http://127.0.0.1:8080/../server.c'          # phải 403 (thử traversal)
curl -i --path-as-is 'http://127.0.0.1:8080/%2e%2e/server.c'      # 403 sau khi giải mã
curl -i 'http://127.0.0.1:8080/%00'               # 400 (NUL)
printf 'GET / HTTP/1.1\r\n' | nc 127.0.0.1 8080   # header chưa xong -> chờ 5 giây rồi đóng (timeout)
```

Mở `http://127.0.0.1:8080/` bằng trình duyệt: trang HTML nạp thêm CSS qua yêu cầu thứ hai.

**Kiểm tra tải:** công cụ `ab` (ApacheBench) hoặc `wrk`:

```bash
ab -n 20000 -c 50 http://127.0.0.1:8080/api/health
wrk -t2 -c100 -d10s http://127.0.0.1:8080/
```

Quan sát yêu cầu/giây, tỷ lệ lỗi; thử thay đổi số worker (`./server 8080 1`, `... 16`) và xem ảnh hưởng.

**Kiểm tra lỗi bộ nhớ và đồng bộ:** chạy `make asan`/`make tsan`, chạy các lệnh `curl` và `ab` ở trên; không có báo cáo nghĩa là sạch. Thử gửi dữ liệu rác/dài bằng `nc` hoặc một script:

```bash
head -c 100000 /dev/urandom | nc 127.0.0.1 8080          # không được crash
python3 -c "print('GET /' + 'a'*5000 + ' HTTP/1.1\r\n\r\n', end='')" | nc 127.0.0.1 8080    # 414
```

## 20.12. Bảo mật và giới hạn — kiểm tra cuối cùng

| Nguy cơ | Biện pháp trong mã |
|---|---|
| Directory traversal (`../`, `%2e%2e`, symlink) | Giải mã trước; `path_is_unsafe`; `realpath` + kiểm tra tiền tố; chỉ `S_ISREG` |
| Tràn bộ đệm khi phân tích | `sscanf` với độ rộng; header giới hạn 8 KB; kiểm tra độ dài trường |
| Slowloris (client gửi chậm giữ kết nối) | `SO_RCVTIMEO`/`SO_SNDTIMEO`; số worker cố định |
| Cạn tài nguyên do quá nhiều kết nối | Hàng đợi giới hạn + `503`; số luồng cố định |
| Log injection | Cắt độ dài; nên loại bỏ `\r\n` khỏi giá trị log |
| `SIGPIPE` giết server | `SIG_IGN` + `MSG_NOSIGNAL` |
| Lộ file nhạy cảm (`.env`, `.git`) | Nên chặn tên file bắt đầu bằng `.` (bài tập) |
| Chạy với quyền cao | Chạy bằng người dùng thường, `chroot`/container, bind cổng cao |
| Nội dung người dùng chèn vào JSON/HTML | Thoát ký tự (bài tập 3) |
| Thiếu HTTPS | Đặt sau reverse proxy có TLS (nginx, Caddy) |

Đây là mã học tập; nếu triển khai thật, hãy dùng server đã được kiểm chứng (nginx, Caddy...) hoặc ít nhất bổ sung: TLS, giới hạn tốc độ theo IP, xử lý `Range`, `If-Modified-Since`, `keep-alive`, chunked encoding, và fuzz bộ phân tích (chương 18).

## 20.13. Hướng mở rộng

- **Keep-alive:** cho phép nhiều yêu cầu trên một kết nối (vòng lặp đọc yêu cầu, hết thời gian chờ nhàn rỗi).
- **`epoll` event loop:** thay thread pool bằng vòng lặp sự kiện non-blocking, máy trạng thái cho mỗi kết nối (đọc header → xử lý → ghi phản hồi); dùng `sendfile`.
- **Các phương thức `POST`:** đọc `Content-Length` byte body (có giới hạn), phân tích `application/x-www-form-urlencoded` hoặc JSON.
- **Cache và điều kiện:** `ETag`, `Last-Modified`/`If-Modified-Since` → `304 Not Modified`.
- **Nén:** `gzip` với zlib khi client gửi `Accept-Encoding: gzip`.
- **`Range` request:** hỗ trợ tải/phát tiếp (`206 Partial Content`).
- **Cấu hình:** đọc file cấu hình, hỗ trợ nhiều thư mục/route, `Host` ảo.
- **TLS:** liên kết OpenSSL/mbedTLS; hoặc để proxy làm.

## 20.14. Lỗi thường gặp

| Lỗi | Triệu chứng | Cách tránh |
|---|---|---|
| Thiếu `\r\n\r\n` hoặc `Content-Length` sai | Trình duyệt treo/cắt nội dung | Luôn đủ header, đúng độ dài |
| Không xử lý `send` một phần | File lớn bị cụt | `send_all` |
| Kiểm tra `..` trước khi giải mã | Bị bypass `%2e%2e` | Giải mã rồi mới kiểm tra |
| So tiền tố thư mục gốc thiếu ký tự phân cách | `/srv/www-x` khớp `/srv/www` | Kiểm tra `/` hoặc `'\0'` sau tiền tố |
| Đóng socket ngay sau `send` | Client mất phần cuối dữ liệu | `shutdown(SHUT_WR)` rồi `close`, hoặc đọc hết |
| Worker chặn mãi trên client treo | Cạn worker | Timeout socket |
| Truyền địa chỉ biến vòng lặp cho luồng | Sai `fd` | Đặt `fd` vào hàng đợi (giá trị, không phải địa chỉ) |
| Quên `MSG_NOSIGNAL`/`SIG_IGN` | Server chết bất ngờ | Cả hai |
| `exit()` trong xử lý yêu cầu | Một yêu cầu xấu giết cả server | Trả lỗi, đóng kết nối |
| Không chờ worker khi thoát | Cắt ngang yêu cầu đang xử lý | `queue_close` + `pthread_join` |

## 20.15. Tóm tắt

- HTTP là giao thức văn bản: dòng yêu cầu + header + dòng trống; phản hồi có dòng trạng thái, header (`Content-Length`, `Content-Type`), body.
- Phân tích yêu cầu **có giới hạn** ở mọi trường; **giải mã URL trước, kiểm tra an toàn sau**.
- Phục vụ file: chặn `..`, `realpath` + kiểm tra thư mục gốc, chỉ file thường; đặt `Content-Type` theo đuôi.
- Thread pool với **hàng đợi giới hạn** và `503` khi quá tải giữ tài nguyên ổn định; timeout socket chống client chậm.
- Tắt êm: cờ `sig_atomic_t`, `poll` timeout, đóng queue, join worker.
- Kiểm thử bằng `curl`, `ab`/`wrk`, ASan/TSan và dữ liệu rác.

## 20.16. Bài tập

1. Chạy server, dùng `curl -v` để quan sát yêu cầu/phản hồi. Xác minh `Content-Length` bằng `wc -c` trên file, và HEAD trả cùng độ dài nhưng không có body.
2. Thêm route `/api/echo?msg=...` trả JSON `{"msg":"..."}` với **thoát ký tự JSON** đúng (`"`, `\`, xuống dòng, ký tự điều khiển). Kiểm thử với chuỗi có dấu nháy và unicode.
3. Cấm phục vụ mọi file/thư mục có thành phần bắt đầu bằng `.` (như `.git`, `.env`) và trả `403`.
4. Thêm `Last-Modified` và xử lý `If-Modified-Since` trả `304 Not Modified` khi file không đổi.
5. Thêm **giới hạn tốc độ** theo địa chỉ IP (cửa sổ trượt hoặc token bucket) — trả `429 Too Many Requests` khi vượt quá; chú ý đồng bộ khi nhiều worker.
6. Thêm hỗ trợ `keep-alive`: lặp đọc yêu cầu trên cùng kết nối tới khi client đóng hoặc nhàn rỗi quá 5 giây; giới hạn số yêu cầu mỗi kết nối.
7. Đo `ab`/`wrk` với 1, 2, 4, 8, 16 worker; vẽ đồ thị yêu cầu/giây; giải thích điểm bão hòa (số nhân CPU, ngưỡng I/O).
8. Viết phiên bản **event loop** dùng `epoll` (Linux) hoặc `poll` cho các kết nối non-blocking: thử đạt 10.000 kết nối nhàn rỗi với ít tài nguyên hơn thread pool.
9. Tạo fuzz target cho `parse_request` và `url_decode` (chương 18); chạy libFuzzer với ASan/UBSan và sửa mọi lỗi tìm được.
10. (Thử thách) Thêm hỗ trợ `POST /upload` nhận file (tới 1 MB) lưu vào thư mục `uploads/` với tên file được **tạo ngẫu nhiên** (không dùng tên client gửi), kiểm tra kích thước, và ghi bằng `O_EXCL`.

Mã nguồn mẫu: /code/chapter-20
