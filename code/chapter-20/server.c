// server.c — web server học tập (ghép từ các mục 20.3–20.10)
#define _XOPEN_SOURCE 700              // POSIX.1-2008 + XSI (cần cho realpath)
#include <arpa/inet.h>
#include <sys/time.h>
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

// Ghi 1 dòng log cho mỗi yêu cầu: thời gian, phương thức, đường dẫn, mã trạng thái, số byte body.
static void log_request(const char *method, const char *path, int status, long bytes) {
    char ts[32];
    time_t now = time(NULL);
    struct tm tmv;
    localtime_r(&now, &tmv);
    strftime(ts, sizeof ts, "%Y-%m-%d %H:%M:%S", &tmv);
    fprintf(stderr, "%s %-4s %-30.30s %d %ld\n", ts, method, path, status, bytes);   // một lệnh fprintf -> không bị xen dòng
}

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
