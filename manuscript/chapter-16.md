# Chương 16 — Mạng cơ bản (sockets)

## Mục tiêu chương

- Hiểu mô hình mạng: **địa chỉ IP, cổng, TCP/UDP**, mô hình client–server.
- Viết **TCP client và server** bằng Berkeley sockets API: `socket`, `bind`, `listen`, `accept`, `connect`, `send`, `recv`, `close`.
- Hiểu vì sao TCP là **luồng byte** (không có ranh giới thông điệp) và cách xử lý **gửi/nhận một phần**, đóng khung (framing).
- Xử lý nhiều client bằng: (1) thread-per-connection, (2) `select`/`poll`.
- Phân biệt **blocking** và **non-blocking**; biết `epoll` và mô hình sự kiện.
- Viết UDP đơn giản; hiểu chuyển đổi **thứ tự byte mạng** (`htons`, `ntohl`).
- Xử lý lỗi và bảo mật cơ bản; biết khác biệt **Winsock** trên Windows.

> **Môi trường:** ví dụ dùng POSIX sockets (Linux, macOS, WSL). Phần 16.11 chỉ ra thay đổi cho Windows/Winsock. Bạn có thể thử các chương trình trên cùng máy qua địa chỉ `127.0.0.1` (loopback).

## 16.1. Kiến thức nền về mạng

### Địa chỉ IP và cổng

- **Địa chỉ IP** xác định một máy: IPv4 dạng `192.168.1.10` (32 bit), IPv6 dạng `2001:db8::1` (128 bit). `127.0.0.1` (IPv4) và `::1` (IPv6) là **loopback** — chính máy bạn.
- **Cổng (port)** (16 bit, 0–65535) xác định **ứng dụng** trên máy đó. Cổng < 1024 thường dành cho dịch vụ hệ thống (cần quyền quản trị); HTTP là 80, HTTPS 443, SSH 22.
- Một kết nối TCP được xác định bởi bộ bốn: **(IP nguồn, cổng nguồn, IP đích, cổng đích)**.

### TCP và UDP

| | TCP | UDP |
|---|---|---|
| Kiểu | Hướng kết nối (connection-oriented) | Không kết nối (datagram) |
| Độ tin cậy | Đảm bảo đến nơi, đúng thứ tự, không trùng | Không đảm bảo (có thể mất, đảo thứ tự, trùng) |
| Đơn vị dữ liệu | **Luồng byte**, không có ranh giới thông điệp | Từng **gói (datagram)** có ranh giới |
| Chi phí | Cao hơn (bắt tay, ACK, kiểm soát tắc nghẽn) | Thấp |
| Dùng cho | Web, email, file, hầu hết ứng dụng | DNS, streaming, game, đo đạc |

### Mô hình client–server

```text
   SERVER                                  CLIENT
socket()                                socket()
bind()    (gắn vào IP:cổng)
listen()  (sẵn sàng nhận kết nối)
accept()  ◄──────── kết nối ────────    connect()
   │                                       │
recv()/send()  ◄────── dữ liệu ──────►  send()/recv()
   │                                       │
close()                                 close()
```

Server **lắng nghe** ở một cổng đã biết; client **chủ động kết nối**. Mỗi kết nối chấp nhận được trả về một **socket mới** để nói chuyện với client đó, còn socket lắng nghe vẫn tiếp tục nhận kết nối khác.

## 16.2. Socket API cơ bản

**Socket** là một điểm cuối giao tiếp; trên POSIX nó là một **file descriptor** (số nguyên) — bạn có thể `read`/`write`/`close` như file.

```c
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>

int socket(int domain, int type, int protocol);
```

- `domain`: `AF_INET` (IPv4), `AF_INET6` (IPv6).
- `type`: `SOCK_STREAM` (TCP), `SOCK_DGRAM` (UDP).
- `protocol`: thường `0`.
- Trả về descriptor (≥ 0) hoặc **`-1`** (đặt `errno`).

```c
int fd = socket(AF_INET, SOCK_STREAM, 0);
if (fd < 0) { perror("socket"); return 1; }
```

### Địa chỉ socket: `sockaddr_in`

```c
struct sockaddr_in {
    sa_family_t    sin_family;   // AF_INET
    in_port_t      sin_port;     // cổng, THỨ TỰ BYTE MẠNG
    struct in_addr sin_addr;     // địa chỉ IPv4, THỨ TỰ BYTE MẠNG
    /* ... */
};
```

Hầu hết hàm nhận `struct sockaddr *` (kiểu tổng quát) nên phải ép kiểu: `(struct sockaddr *)&addr`.

```c
struct sockaddr_in addr;
memset(&addr, 0, sizeof addr);              // xóa sạch (quan trọng: trường thừa phải bằng 0)
addr.sin_family = AF_INET;
addr.sin_port   = htons(8080);              // đổi sang thứ tự byte mạng
inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);   // chuỗi -> nhị phân
```

### Thứ tự byte mạng

Mạng dùng **big-endian** (chương 2), còn máy bạn thường little-endian. Luôn chuyển đổi số nhiều byte đặt trong gói tin:

| Hàm | Ý nghĩa |
|---|---|
| `htons(x)` | host → network, 16 bit (cổng) |
| `htonl(x)` | host → network, 32 bit |
| `ntohs(x)` | network → host, 16 bit |
| `ntohl(x)` | network → host, 32 bit |

### Chuyển đổi địa chỉ

```c
inet_pton(AF_INET, "192.168.1.10", &addr.sin_addr);     // text -> nhị phân (trả 1 nếu thành công)

char text[INET_ADDRSTRLEN];
inet_ntop(AF_INET, &addr.sin_addr, text, sizeof text);  // nhị phân -> text
```

Dùng `inet_pton`/`inet_ntop` (hỗ trợ IPv6) thay cho `inet_addr`/`inet_ntoa` cũ.

### Phân giải tên miền: `getaddrinfo`

Để kết nối tới `example.com`, cần tra DNS. `getaddrinfo` là cách hiện đại, **hỗ trợ cả IPv4 và IPv6**, và nên dùng thay cho lắp `sockaddr_in` thủ công:

```c
struct addrinfo hints, *res, *p;
memset(&hints, 0, sizeof hints);
hints.ai_family   = AF_UNSPEC;        // IPv4 hoặc IPv6
hints.ai_socktype = SOCK_STREAM;

int rc = getaddrinfo("example.com", "80", &hints, &res);
if (rc != 0) { fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rc)); return 1; }

int fd = -1;
for (p = res; p != NULL; p = p->ai_next) {          // thử từng địa chỉ cho tới khi được
    fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
    if (fd < 0) continue;
    if (connect(fd, p->ai_addr, p->ai_addrlen) == 0) break;
    close(fd);
    fd = -1;
}
freeaddrinfo(res);                                   // nhớ giải phóng
```

## 16.3. TCP server

Các bước: `socket` → `bind` → `listen` → vòng lặp `accept` → `recv`/`send` → `close`.

```c
int bind(int fd, const struct sockaddr *addr, socklen_t len);   // gắn socket vào IP:cổng
int listen(int fd, int backlog);                                // backlog: độ dài hàng đợi kết nối chờ
int accept(int fd, struct sockaddr *client, socklen_t *len);    // chặn tới khi có client; trả socket mới
```

### Server echo tuần tự (một client tại một thời điểm)

```c
// echo_server.c — server phản hồi lại mọi byte nhận được
#define _POSIX_C_SOURCE 200809L
#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

// Gửi ĐỦ n byte (send có thể gửi ít hơn yêu cầu). Trả 0 nếu thành công, -1 nếu lỗi.
static int send_all(int fd, const char *buf, size_t n) {
    size_t sent = 0;
    while (sent < n) {
        ssize_t r = send(fd, buf + sent, n - sent, 0);
        if (r < 0) {
            if (errno == EINTR) continue;        // bị tín hiệu ngắt: thử lại
            return -1;
        }
        sent += (size_t)r;
    }
    return 0;
}

int main(int argc, char *argv[]) {
    int port = argc > 1 ? atoi(argv[1]) : 8080;

    int srv = socket(AF_INET, SOCK_STREAM, 0);
    if (srv < 0) { perror("socket"); return 1; }

    int yes = 1;                                                     // cho phép bind lại cổng ngay sau khi tắt server
    setsockopt(srv, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes);     // (tránh lỗi "Address already in use")

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof addr);
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);                        // mọi giao diện mạng
    addr.sin_port = htons((uint16_t)port);

    if (bind(srv, (struct sockaddr *)&addr, sizeof addr) < 0) { perror("bind"); return 1; }
    if (listen(srv, 16) < 0) { perror("listen"); return 1; }
    printf("Lang nghe tren cong %d...\n", port);

    for (;;) {
        struct sockaddr_in cli;
        socklen_t clen = sizeof cli;
        int fd = accept(srv, (struct sockaddr *)&cli, &clen);
        if (fd < 0) {
            if (errno == EINTR) continue;
            perror("accept");
            break;
        }
        char ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &cli.sin_addr, ip, sizeof ip);
        printf("Ket noi tu %s:%d\n", ip, ntohs(cli.sin_port));

        char buf[1024];
        ssize_t n;
        while ((n = recv(fd, buf, sizeof buf, 0)) > 0) {
            if (send_all(fd, buf, (size_t)n) < 0) { perror("send"); break; }
        }
        if (n < 0) perror("recv");
        printf("Dong ket noi\n");
        close(fd);
    }
    close(srv);
    return 0;
}
```

Điểm quan trọng:

- **`SO_REUSEADDR`**: nếu thiếu, sau khi tắt server và chạy lại ngay, `bind` thường báo `Address already in use` (cổng ở trạng thái TIME_WAIT).
- **`INADDR_ANY`** (`0.0.0.0`): lắng nghe trên mọi địa chỉ máy; dùng `inet_pton("127.0.0.1")` để chỉ nhận từ chính máy (an toàn hơn khi thử nghiệm).
- **`recv` trả về:** `> 0` số byte nhận được; **`0`** = phía bên kia **đã đóng kết nối** (kết thúc bình thường); **`< 0`** = lỗi.
- Server này chỉ phục vụ **một client mỗi lần** (client thứ hai phải đợi). Các mục 16.6–16.7 giải quyết vấn đề đó.

## 16.4. TCP client

Các bước: `socket` → `connect` → `send`/`recv` → `close`.

```c
// echo_client.c
#define _POSIX_C_SOURCE 200809L
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    const char *host = argc > 1 ? argv[1] : "127.0.0.1";
    int port = argc > 2 ? atoi(argv[2]) : 8080;

    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) { perror("socket"); return 1; }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof addr);
    addr.sin_family = AF_INET;
    addr.sin_port = htons((uint16_t)port);
    if (inet_pton(AF_INET, host, &addr.sin_addr) != 1) {
        fprintf(stderr, "dia chi IP khong hop le: %s\n", host);
        return 1;
    }

    if (connect(fd, (struct sockaddr *)&addr, sizeof addr) < 0) {
        perror("connect");             // "Connection refused" nếu không có server nào lắng nghe
        return 1;
    }

    char line[512];
    while (fgets(line, sizeof line, stdin)) {
        size_t len = strlen(line);
        if (send(fd, line, len, 0) < 0) { perror("send"); break; }

        char reply[512];
        ssize_t n = recv(fd, reply, sizeof reply - 1, 0);
        if (n <= 0) { if (n < 0) perror("recv"); else puts("server dong ket noi"); break; }
        reply[n] = '\0';
        printf("server: %s", reply);
    }
    close(fd);
    return 0;
}
```

Thử: mở hai terminal — chạy `./echo_server 8080` ở terminal 1 và `./echo_client 127.0.0.1 8080` ở terminal 2, gõ chữ và nhấn Enter. Không có client, bạn cũng có thể dùng `nc 127.0.0.1 8080` (netcat) để thử server.

## 16.5. TCP là luồng byte — hiểu đúng để tránh lỗi

Đây là hiểu lầm số một của người mới học socket: **TCP không bảo tồn ranh giới các lần `send`.**

Nếu client gọi `send("abc")` rồi `send("def")`, phía nhận có thể:

- một lần `recv` được `"abcdef"`;
- hoặc `"ab"` rồi `"cdef"`;
- hoặc `"a"`, `"bcd"`, `"ef"`.

Không có sự tương ứng giữa số lần `send` và số lần `recv`. Hệ quả bắt buộc:

1. **`send` có thể gửi ít hơn yêu cầu** (đặc biệt khi bộ đệm đầy): phải lặp như `send_all`.
2. **`recv` có thể trả ít hơn mong đợi:** phải lặp cho tới khi đủ số byte cần (`recv_all`), hoặc cho tới khi gặp dấu kết thúc thông điệp.
3. Giao thức của bạn phải tự **đóng khung (framing)** để biết một thông điệp bắt đầu/kết thúc ở đâu.

### Ba cách đóng khung phổ biến

**(a) Ký tự phân cách** (như dòng `\n` trong HTTP header, SMTP, Redis đơn giản):

```c
// Đọc tới khi gặp '\n'. Trả số byte đã đọc (kể cả '\n'), 0 nếu đối phương đóng, -1 nếu lỗi.
ssize_t recv_line(int fd, char *buf, size_t cap) {
    size_t n = 0;
    while (n + 1 < cap) {
        char c;
        ssize_t r = recv(fd, &c, 1, 0);      // đọc từng byte: đơn giản nhưng chậm; thực tế dùng bộ đệm
        if (r < 0) { if (errno == EINTR) continue; return -1; }
        if (r == 0) break;                   // đối phương đóng
        buf[n++] = c;
        if (c == '\n') break;
    }
    buf[n] = '\0';
    return (ssize_t)n;
}
```

**(b) Tiền tố độ dài** (nhị phân, hiệu quả, dùng rộng rãi): mỗi thông điệp bắt đầu bằng độ dài payload (4 byte, **thứ tự byte mạng**):

```c
// Nhận đúng n byte (hoặc thất bại)
int recv_all(int fd, void *buf, size_t n) {
    char *p = buf;
    size_t got = 0;
    while (got < n) {
        ssize_t r = recv(fd, p + got, n - got, 0);
        if (r < 0) { if (errno == EINTR) continue; return -1; }
        if (r == 0) return -1;               // đóng giữa chừng thông điệp
        got += (size_t)r;
    }
    return 0;
}

int send_msg(int fd, const void *data, uint32_t len) {
    uint32_t nlen = htonl(len);
    if (send_all(fd, (const char *)&nlen, sizeof nlen) < 0) return -1;
    return send_all(fd, data, len);
}

// Nhận một thông điệp; *out cấp phát bằng malloc (người gọi free). Giới hạn max để chống tấn công.
int recv_msg(int fd, char **out, uint32_t *out_len, uint32_t max) {
    uint32_t nlen;
    if (recv_all(fd, &nlen, sizeof nlen) < 0) return -1;
    uint32_t len = ntohl(nlen);
    if (len > max) return -1;                // KHÔNG tin độ dài do đối phương gửi -> chống hết bộ nhớ
    char *buf = malloc(len ? len : 1);
    if (!buf) return -1;
    if (recv_all(fd, buf, len) < 0) { free(buf); return -1; }
    *out = buf; *out_len = len;
    return 0;
}
```

**(c) Độ dài cố định** (đơn giản, ít linh hoạt).

Luôn **giới hạn kích thước tối đa** của thông điệp khi đọc độ dài do phía kia gửi — nếu không, kẻ tấn công gửi độ dài 4 GB khiến bạn `malloc` hết bộ nhớ.

### Đóng kết nối và `shutdown`

`close(fd)` đóng cả hai chiều. `shutdown(fd, SHUT_WR)` chỉ đóng **chiều gửi** (báo cho phía kia "hết dữ liệu", `recv` của họ trả 0), nhưng bạn vẫn còn nhận được. Hữu ích khi client gửi hết yêu cầu rồi đợi phản hồi.

Ghi dữ liệu vào socket mà đối phương đã đóng có thể phát tín hiệu **`SIGPIPE`** làm chương trình bị giết. Server nên bỏ qua nó: `signal(SIGPIPE, SIG_IGN);` hoặc dùng cờ `MSG_NOSIGNAL` trong `send`.

## 16.6. Phục vụ nhiều client: thread-per-connection

Mỗi kết nối `accept` được giao cho một luồng riêng (chương 15). Server luôn quay lại `accept` ngay.

```c
// echo_server_mt.c (phần chính; dùng lại send_all ở trên)
#include <pthread.h>

typedef struct { int fd; struct sockaddr_in addr; } Client;

static void *client_thread(void *arg) {
    Client *c = arg;                         // đã cấp phát heap; luồng sở hữu và phải free
    char buf[1024];
    ssize_t n;
    while ((n = recv(c->fd, buf, sizeof buf, 0)) > 0) {
        if (send_all(c->fd, buf, (size_t)n) < 0) break;
    }
    close(c->fd);
    free(c);
    return NULL;
}

/* trong vòng lặp accept: */
for (;;) {
    Client *c = malloc(sizeof *c);
    if (!c) continue;
    socklen_t clen = sizeof c->addr;
    c->fd = accept(srv, (struct sockaddr *)&c->addr, &clen);
    if (c->fd < 0) { free(c); if (errno == EINTR) continue; perror("accept"); break; }

    pthread_t t;
    if (pthread_create(&t, NULL, client_thread, c) != 0) {
        close(c->fd); free(c);
        continue;
    }
    pthread_detach(t);                       // không cần join: luồng tự dọn khi xong
}
```

Chú ý:

- **Không** truyền `&fd` của biến trên stack của vòng lặp: đọc trễ sẽ thấy giá trị của client sau (race, chương 15). Cấp phát `Client` trên heap cho mỗi kết nối.
- `pthread_detach` vì server chạy vô hạn.
- **Giới hạn số kết nối/luồng** (semaphore hoặc bộ đếm): kẻ tấn công mở hàng nghìn kết nối có thể làm cạn tài nguyên.

Ưu điểm: mã dễ viết và đọc như đơn luồng (mỗi kết nối một dòng suy nghĩ tuần tự). Nhược điểm: mỗi luồng tốn bộ nhớ stack (~MB) và chi phí chuyển ngữ cảnh — **không mở rộng** tới hàng chục nghìn kết nối (bài toán C10K).

## 16.7. Multiplexing: `select`, `poll`, `epoll`

Thay vì một luồng cho mỗi client, một luồng **theo dõi nhiều socket** và chỉ xử lý socket **đã sẵn sàng**.

### Blocking và non-blocking

- **Blocking (mặc định):** `recv` chặn tới khi có dữ liệu; `accept` chặn tới khi có kết nối.
- **Non-blocking:** đặt cờ `O_NONBLOCK`; `recv`/`accept`/`send` **trả về ngay**, với `-1` và `errno == EAGAIN` (hoặc `EWOULDBLOCK`) nếu chưa sẵn sàng.

```c
#include <fcntl.h>
int set_nonblocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags < 0) return -1;
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}
```

### `poll` — đơn giản và không giới hạn 1024 như `select`

```c
#include <poll.h>

int poll(struct pollfd fds[], nfds_t nfds, int timeout_ms);

struct pollfd { int fd; short events; short revents; };
// events: POLLIN (có dữ liệu để đọc), POLLOUT (có thể ghi); revents: kết quả, gồm cả POLLERR, POLLHUP
```

Server echo đa khách bằng `poll`, **một luồng duy nhất**:

```c
// echo_poll.c (phần vòng lặp chính; dùng lại phần khởi tạo `srv` ở echo_server.c)
#include <poll.h>

#define MAX_CLIENTS 64

struct pollfd fds[MAX_CLIENTS + 1];
int nfds = 1;
fds[0].fd = srv;              // fds[0] luôn là socket lắng nghe
fds[0].events = POLLIN;

for (;;) {
    int ready = poll(fds, nfds, -1);           // -1: chờ vô hạn
    if (ready < 0) { if (errno == EINTR) continue; perror("poll"); break; }

    // 1) có kết nối mới?
    if (fds[0].revents & POLLIN) {
        int fd = accept(srv, NULL, NULL);
        if (fd >= 0) {
            if (nfds <= MAX_CLIENTS) {
                fds[nfds].fd = fd;
                fds[nfds].events = POLLIN;
                nfds++;
            } else {
                close(fd);                      // đầy: từ chối
            }
        }
    }

    // 2) client nào có dữ liệu?
    for (int i = 1; i < nfds; ) {
        if (fds[i].revents & (POLLIN | POLLHUP | POLLERR)) {
            char buf[1024];
            ssize_t n = recv(fds[i].fd, buf, sizeof buf, 0);
            if (n <= 0) {                       // đóng hoặc lỗi -> gỡ khỏi danh sách
                close(fds[i].fd);
                fds[i] = fds[nfds - 1];         // đưa phần tử cuối vào chỗ trống
                nfds--;
                continue;                       // xét lại chỉ số i (giờ chứa phần tử mới)
            }
            send_all(fds[i].fd, buf, (size_t)n);   // (nếu send có thể chặn -> cần bộ đệm ghi + POLLOUT)
        }
        i++;
    }
}
```

Điều quan trọng: **một luồng đơn** phục vụ được hàng nghìn kết nối vì không có luồng nào bị chặn để chờ một client chậm. Cái giá là mã theo kiểu **máy trạng thái** (mỗi client có trạng thái riêng, bộ đệm riêng), phức tạp hơn khi cần đọc dở dang/ghi dở dang.

### `select` và `epoll`

- **`select`:** cổ điển, di động (cả Windows), nhưng giới hạn `FD_SETSIZE` (thường 1024) và phải dựng lại tập mỗi lần.
- **`poll`:** như trên, không giới hạn cứng nhưng vẫn duyệt toàn bộ mảng mỗi lần: `O(n)`.
- **`epoll` (Linux)** / **`kqueue` (BSD, macOS)**: đăng ký một lần, kernel chỉ trả về **những descriptor sẵn sàng**: `O(số sự kiện)`. Nền tảng của nginx, Redis, Node.js.

```c
#include <sys/epoll.h>

int ep = epoll_create1(0);
struct epoll_event ev = { .events = EPOLLIN, .data.fd = srv };
epoll_ctl(ep, EPOLL_CTL_ADD, srv, &ev);

struct epoll_event events[64];
for (;;) {
    int n = epoll_wait(ep, events, 64, -1);
    for (int i = 0; i < n; i++) {
        int fd = events[i].data.fd;
        /* xử lý fd sẵn sàng: accept nếu fd == srv, ngược lại recv/send */
    }
}
```

Với client non-blocking phải xử lý `EAGAIN`: **đọc cho tới khi hết** (edge-triggered) hoặc đọc một phần rồi để vòng sau (level-triggered, mặc định).

### Chọn mô hình nào?

| Mô hình | Độ phức tạp mã | Số kết nối | Ghi chú |
|---|---|---|---|
| Tuần tự | Thấp nhất | 1 | Chỉ để học/công cụ nhỏ |
| Thread-per-connection | Thấp | Hàng trăm–vài nghìn | Dễ viết; cần chú ý đồng bộ |
| `select`/`poll` một luồng | Trung bình | Hàng trăm–nghìn | Không cần khóa |
| `epoll`/`kqueue` (event loop) | Cao | Hàng chục nghìn+ | Hiệu năng cao, thường kèm thread pool |

## 16.8. UDP

UDP không có kết nối: mỗi lần gửi/nhận là **một gói độc lập** kèm địa chỉ.

```c
ssize_t sendto(int fd, const void *buf, size_t len, int flags,
               const struct sockaddr *dest, socklen_t dlen);
ssize_t recvfrom(int fd, void *buf, size_t len, int flags,
                 struct sockaddr *src, socklen_t *slen);
```

```c
// udp_echo_server.c (rút gọn)
int fd = socket(AF_INET, SOCK_DGRAM, 0);
struct sockaddr_in addr = {0};
addr.sin_family = AF_INET;
addr.sin_addr.s_addr = htonl(INADDR_ANY);
addr.sin_port = htons(9000);
bind(fd, (struct sockaddr *)&addr, sizeof addr);

for (;;) {
    char buf[1500];
    struct sockaddr_in from;
    socklen_t flen = sizeof from;
    ssize_t n = recvfrom(fd, buf, sizeof buf, 0, (struct sockaddr *)&from, &flen);
    if (n < 0) { perror("recvfrom"); break; }
    sendto(fd, buf, (size_t)n, 0, (struct sockaddr *)&from, flen);   // gửi lại đúng người gửi
}
```

Đặc điểm cần nhớ:

- **Không cần `listen`/`accept`/`connect`** (dù có thể gọi `connect` để gán địa chỉ mặc định).
- **Ranh giới thông điệp được giữ:** một `recvfrom` nhận trọn một datagram (nếu bộ đệm nhỏ hơn, phần thừa bị **cắt bỏ**).
- **Không tin cậy:** gói có thể mất/trùng/đảo. Ứng dụng phải tự đánh số, xác nhận, gửi lại nếu cần.
- Kích thước hợp lý ≤ ~1400 byte để tránh phân mảnh IP.

## 16.9. Ví dụ: gửi HTTP GET thô

HTTP/1.1 là giao thức **văn bản** chạy trên TCP. Yêu cầu tối thiểu:

```text
GET / HTTP/1.1\r\n
Host: example.com\r\n
Connection: close\r\n
\r\n
```

```c
// http_get.c
#define _POSIX_C_SOURCE 200809L
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    const char *host = argc > 1 ? argv[1] : "example.com";
    const char *path = argc > 2 ? argv[2] : "/";

    struct addrinfo hints = {0}, *res;
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    int rc = getaddrinfo(host, "80", &hints, &res);
    if (rc != 0) { fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rc)); return 1; }

    int fd = -1;
    for (struct addrinfo *p = res; p; p = p->ai_next) {
        fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (fd < 0) continue;
        if (connect(fd, p->ai_addr, p->ai_addrlen) == 0) break;
        close(fd); fd = -1;
    }
    freeaddrinfo(res);
    if (fd < 0) { fprintf(stderr, "khong ket noi duoc %s\n", host); return 1; }

    char req[512];
    int len = snprintf(req, sizeof req,
        "GET %s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\nUser-Agent: c-book/1.0\r\n\r\n", path, host);
    if (len < 0 || (size_t)len >= sizeof req) { fprintf(stderr, "yeu cau qua dai\n"); return 1; }
    if (send(fd, req, (size_t)len, 0) != len) { perror("send"); return 1; }   // (thực tế nên dùng send_all)

    char buf[4096];
    ssize_t n;
    while ((n = recv(fd, buf, sizeof buf, 0)) > 0) {
        fwrite(buf, 1, (size_t)n, stdout);         // in cả header lẫn body
    }
    if (n < 0) perror("recv");
    close(fd);
    return 0;
}
```

Server đáp lại một dòng trạng thái (`HTTP/1.1 200 OK`), các header, dòng trống, rồi body. Vì có `Connection: close`, server đóng kết nối khi xong, và `recv` trả 0 báo hiệu hết dữ liệu. Chương 20 xây dựng phía server. HTTPS cần thêm TLS (OpenSSL/mbedTLS), không được trình bày ở đây.

## 16.10. Xử lý lỗi, giới hạn thời gian và bảo mật

### Lỗi thường gặp và ý nghĩa

| Lỗi | Nguyên nhân |
|---|---|
| `Address already in use` (`EADDRINUSE`) | Cổng đang được dùng / TIME_WAIT → `SO_REUSEADDR` |
| `Connection refused` (`ECONNREFUSED`) | Không có tiến trình nào lắng nghe cổng đó |
| `Connection reset by peer` (`ECONNRESET`) | Đối phương đóng đột ngột |
| `Broken pipe` / `SIGPIPE` | Ghi vào kết nối đã đóng |
| `Permission denied` khi `bind` | Cổng < 1024 cần quyền quản trị |
| `Connection timed out` | Mạng/firewall chặn, máy đích không phản hồi |
| `Interrupted system call` (`EINTR`) | Bị tín hiệu ngắt: thử lại |

### Thời gian chờ (timeout)

Mặc định `recv`/`connect` có thể chặn **rất lâu**. Đặt giới hạn:

```c
struct timeval tv = { .tv_sec = 5, .tv_usec = 0 };
setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof tv);     // recv chờ tối đa 5 giây (trả -1, EAGAIN)
setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof tv);
```

Hoặc dùng `poll` với `timeout_ms`.

### Bảo mật khi viết server

- **Không tin bất cứ điều gì từ mạng.** Mọi độ dài, chỉ số, chuỗi đều phải kiểm tra; giới hạn kích thước thông điệp/dòng/header.
- **Chống tràn bộ đệm:** dùng độ dài đã kiểm tra khi chép; đảm bảo chuỗi nhận được có `'\0'` trước khi dùng như chuỗi C.
- **Giới hạn tài nguyên:** số kết nối, thời gian chờ, tốc độ, kích thước yêu cầu — chống *slowloris* và cạn tài nguyên.
- **Chỉ lắng nghe trên giao diện cần thiết** (`127.0.0.1` khi thử nghiệm).
- **Giảm quyền** sau khi `bind` cổng đặc quyền.
- Nếu cần bảo mật đường truyền: dùng **TLS** (OpenSSL, mbedTLS). Đừng tự "mã hóa" bằng cách nghĩ ra.
- Kiểm tra bằng sanitizer và fuzzer (chương 18, 21).

## 16.11. Windows: Winsock

Winsock rất giống Berkeley sockets, khác vài điểm:

| POSIX | Windows (Winsock) |
|---|---|
| `#include <sys/socket.h>`... | `#include <winsock2.h>`, `<ws2tcpip.h>` |
| Không cần khởi tạo | Phải gọi `WSAStartup(MAKEWORD(2,2), &wsa)` đầu chương trình; `WSACleanup()` khi xong |
| Socket là `int` | Socket là kiểu `SOCKET`; lỗi là `INVALID_SOCKET` |
| `close(fd)` | `closesocket(s)` |
| `errno` | `WSAGetLastError()` |
| `fcntl(..., O_NONBLOCK)` | `ioctlsocket(s, FIONBIO, &mode)` |
| `poll` | `WSAPoll` |
| Liên kết | `-lws2_32` |

```c
#ifdef _WIN32
  #include <winsock2.h>
  #include <ws2tcpip.h>
  typedef SOCKET socket_t;
  #define CLOSESOCKET closesocket
  static int net_init(void) { WSADATA w; return WSAStartup(MAKEWORD(2, 2), &w); }
  static void net_cleanup(void) { WSACleanup(); }
#else
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <arpa/inet.h>
  #include <unistd.h>
  typedef int socket_t;
  #define INVALID_SOCKET (-1)
  #define CLOSESOCKET close
  static int net_init(void) { return 0; }
  static void net_cleanup(void) {}
#endif
```

Biên dịch với MinGW: `gcc -std=c11 -Wall client.c -o client.exe -lws2_32`. Bọc các khác biệt trong một file nhỏ (như trên) là cách tốt để giữ phần còn lại của mã đa nền tảng. Nếu bạn dùng WSL, mọi ví dụ POSIX chạy nguyên xi.

## 16.12. Công cụ hỗ trợ

| Công cụ | Việc dùng |
|---|---|
| `nc` (netcat) | Client/server thô để thử: `nc -l 8080`, `nc 127.0.0.1 8080` |
| `curl -v http://127.0.0.1:8080/` | Thử server HTTP, xem header |
| `ss -tlnp` / `netstat -ano` | Xem cổng đang lắng nghe/kết nối |
| `tcpdump`, Wireshark | Bắt và phân tích gói tin thật |
| `strace -e trace=network ./prog` | Xem các lời gọi socket của chương trình |

## 16.13. Lỗi thường gặp

| Lỗi | Hậu quả | Cách tránh |
|---|---|---|
| Giả định một `send` = một `recv` | Dữ liệu bị cắt/gộp | Đóng khung (delimiter/độ dài) |
| Không xử lý `send`/`recv` một phần | Mất/thiếu dữ liệu | `send_all`, `recv_all` |
| Quên `htons`/`ntohs` | Cổng/độ dài sai | Luôn chuyển đổi số nhiều byte |
| Không xóa `sockaddr` (`memset`) | Lỗi khó hiểu | `memset(&addr, 0, sizeof addr)` |
| Không đặt `SO_REUSEADDR` | `Address already in use` | Đặt trước `bind` |
| `recv` trả 0 bị bỏ qua | Vòng lặp vô hạn | Xử lý `0` = đối phương đóng |
| Không đóng socket | Rò rỉ descriptor | `close` trên mọi đường thoát |
| Tin độ dài do đối phương gửi | Hết bộ nhớ/tràn | Kiểm tra tối đa |
| `SIGPIPE` giết server | Server chết bất ngờ | `SIG_IGN` / `MSG_NOSIGNAL` |
| Dùng chuỗi nhận được mà không có `'\0'` | Đọc ngoài bộ đệm | Tự thêm `'\0'` |
| Blocking `recv` không timeout | Treo mãi | `SO_RCVTIMEO`/`poll` |

## 16.14. Tóm tắt

- Socket là file descriptor; server: `socket → bind → listen → accept`; client: `socket → connect`.
- TCP là **luồng byte**: xử lý gửi/nhận một phần, tự đóng khung thông điệp; luôn kiểm tra `recv` trả `0`/`-1`.
- Dùng `htons`/`ntohs`/`htonl`/`ntohl` cho số nhiều byte; `inet_pton`/`getaddrinfo` cho địa chỉ.
- Nhiều client: thread-per-connection (đơn giản) hoặc `poll`/`epoll` (mở rộng); đặt timeout.
- UDP bảo toàn ranh giới gói nhưng không tin cậy.
- Server phải coi mọi dữ liệu mạng là không tin cậy và giới hạn tài nguyên.
- Windows dùng Winsock: `WSAStartup`, `closesocket`, `-lws2_32`.

## 16.15. Bài tập

1. Chạy `echo_server` và dùng `nc` để thử; sau đó mở hai `nc` cùng lúc để thấy client thứ hai bị chờ. Giải thích.
2. Viết **client echo** đọc từng dòng từ bàn phím và in phản hồi. Đo thời gian khứ hồi (`clock_gettime`).
3. Biến server echo thành **thread-per-connection**, và kiểm thử với 100 client song song bằng một script/chương trình.
4. Viết lại bằng **`poll`** trên một luồng. So sánh mức dùng bộ nhớ/thời gian với phiên bản đa luồng khi có 1000 kết nối.
5. Cài đặt giao thức đơn giản dạng dòng: `SET key value`, `GET key`, `DEL key` lưu trong bảng băm (server một luồng dùng `poll`). Client thử bằng `nc`.
6. Cài đặt giao thức **tiền tố độ dài**: client gửi thông điệp nhị phân bất kỳ, server trả về phiên bản đảo ngược byte. Kiểm thử với thông điệp dài 10 MB gửi qua nhiều lần `send`.
7. Viết **UDP** client/server đo độ trễ (ping): gửi 100 gói có số thứ tự, thống kê gói mất và thời gian trung bình.
8. Viết chương trình tải một trang bằng HTTP GET, tách **header** khỏi **body** (tìm `\r\n\r\n`) và ghi body ra file.
9. (Thử thách) Thêm **timeout không hoạt động** (idle timeout) vào server `poll`: đóng kết nối nếu client im lặng quá 30 giây.

Mã nguồn mẫu: /code/chapter-16
