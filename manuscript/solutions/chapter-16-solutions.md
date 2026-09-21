# Chương 16 — Lời giải bài tập

> Chạy trên Linux, macOS hoặc WSL. Mọi ví dụ dùng `127.0.0.1`.

## Bài 1: server tuần tự và hai `nc`

Mở `./echo_server 8080`; terminal A: `nc 127.0.0.1 8080`, gõ chữ → nhận lại. Terminal B: `nc 127.0.0.1 8080`, gõ chữ → **không có phản hồi** cho đến khi A đóng. Lý do: vòng `while (recv...)` phục vụ client A không quay lại `accept()`; kết nối của B nằm trong hàng đợi `listen` (đã `connect` thành công ở mức TCP nhưng chưa được `accept`). Khi A đóng, server `accept` B và phản hồi dồn lại.

## Bài 2: đo thời gian khứ hồi

```c
// rtt_client.c
#define _POSIX_C_SOURCE 200809L
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

static double now(void) { struct timespec t; clock_gettime(CLOCK_MONOTONIC, &t); return (double)t.tv_sec + (double)t.tv_nsec * 1e-9; }

int main(int argc, char **argv) {
    int port = argc > 1 ? atoi(argv[1]) : 8080;
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in a = { .sin_family = AF_INET, .sin_port = htons((uint16_t)port) };
    inet_pton(AF_INET, "127.0.0.1", &a.sin_addr);
    if (fd < 0 || connect(fd, (struct sockaddr *)&a, sizeof a) < 0) { perror("connect"); return 1; }

    char line[256], reply[256];
    while (fgets(line, sizeof line, stdin)) {
        size_t len = strlen(line);
        double t0 = now();
        if (send(fd, line, len, 0) != (ssize_t)len) break;
        ssize_t n = recv(fd, reply, sizeof reply - 1, 0);
        if (n <= 0) break;
        printf("RTT %.3f ms\n", (now() - t0) * 1e3);
    }
    close(fd);
    return 0;
}
```

Trên loopback RTT thường vài chục micro-giây.

## Bài 3: thread-per-connection

Dùng `client_thread` ở mục 16.6; thử với 100 client bằng vòng lặp shell: `for i in $(seq 100); do (echo hi | nc -q1 127.0.0.1 8080 &); done`. Kiểm tra: `ss -tan | grep 8080 | wc -l`. Cần `pthread_detach`, `Client` cấp phát heap, và giới hạn số kết nối đồng thời.

## Bài 4: một luồng bằng `poll`

Vòng lặp ở mục 16.7. So sánh với 1000 kết nối nhàn rỗi: bản đa luồng tạo 1000 luồng (mỗi luồng ~8 MB stack ảo, chi phí chuyển ngữ cảnh), bản `poll` chỉ dùng vài trăm KB và một luồng; đo bằng `ps -o nlwp,rss -p <pid>`.

## Bài 5: giao thức dòng `SET/GET/DEL`

Phân tích mỗi dòng bằng `sscanf(line, "%15s %63s %63s", cmd, key, value)` (có giới hạn độ rộng), tra bảng băm (chương 10, bài 9) rồi trả `OK\n`, `VALUE <v>\n`, `NOTFOUND\n`, `ERR\n`. Kiểm thử bằng `printf 'SET a 1\nGET a\nDEL a\nGET a\n' | nc 127.0.0.1 8080`. Với `poll` phải giữ **bộ đệm đọc riêng cho từng client** vì một dòng có thể đến từng mảnh.

## Bài 6: tiền tố độ dài

Dùng `send_msg`/`recv_msg` (mục 16.5). Client gửi 10 MB bằng một lần `send_msg` (bên trong `send_all` lặp nhiều lần vì `send` chỉ nhận một phần); server `recv_msg` với `max = 16 MB`, đảo byte rồi gửi trả. Kiểm tra `cmp` giữa dữ liệu gốc (đảo hai lần) và kết quả. Thử `len = 0xFFFFFFFF` để chắc chắn `max` chặn được.

## Bài 7: ping bằng UDP

```c
// udp_ping.c (client)
#define _POSIX_C_SOURCE 200809L
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

int main(void) {
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in srv = { .sin_family = AF_INET, .sin_port = htons(9000) };
    inet_pton(AF_INET, "127.0.0.1", &srv.sin_addr);

    struct timeval tv = { .tv_sec = 1, .tv_usec = 0 };
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof tv);       // chờ tối đa 1 giây cho mỗi gói

    int lost = 0, got = 0;
    double total = 0;
    for (uint32_t seq = 0; seq < 100; seq++) {
        uint32_t nseq = htonl(seq);
        struct timespec a, b;
        clock_gettime(CLOCK_MONOTONIC, &a);
        sendto(fd, &nseq, sizeof nseq, 0, (struct sockaddr *)&srv, sizeof srv);

        uint32_t reply;
        ssize_t n = recvfrom(fd, &reply, sizeof reply, 0, NULL, NULL);
        clock_gettime(CLOCK_MONOTONIC, &b);
        if (n == sizeof reply && ntohl(reply) == seq) {
            total += (double)(b.tv_sec - a.tv_sec) * 1e3 + (double)(b.tv_nsec - a.tv_nsec) * 1e-6;
            got++;
        } else lost++;                                              // mất gói hoặc quá hạn
    }
    printf("nhan %d, mat %d, RTT trung binh %.3f ms\n", got, lost, got ? total / got : 0.0);
    close(fd);
    return 0;
}
```

Server là `recvfrom`/`sendto` echo như mục 16.8. Trên loopback hầu như không mất gói; để thấy mất gói dùng `tc qdisc add dev lo root netem loss 10%` (Linux, cần root).

## Bài 8: HTTP GET, tách header và body

Thu toàn bộ phản hồi vào bộ đệm động (đọc đến khi `recv` trả 0 nhờ `Connection: close`), tìm `"\r\n\r\n"` bằng `strstr`/`memmem`; phần trước là header, phần sau là body. Ghi body bằng `fwrite` ra file (chế độ `"wb"`). Lưu ý phản hồi có thể dùng `Transfer-Encoding: chunked` — khi đó phải giải mã các khối `<độ dài hex>\r\n<dữ liệu>\r\n`; cách đơn giản để tránh: gửi `HTTP/1.0` thay cho `HTTP/1.1` (server thường không dùng chunked).

## Bài 9: idle timeout cho server `poll`

Lưu `last_active[i]` (`time(NULL)`) cho mỗi client, cập nhật mỗi khi nhận dữ liệu. Gọi `poll` với `timeout = 1000` ms; sau mỗi vòng, duyệt các client và đóng những kết nối có `now - last_active[i] > 30`. Nhớ **gỡ đúng cách khỏi mảng** (hoán đổi với phần tử cuối và không tăng `i`) như trong vòng lặp mục 16.7.
