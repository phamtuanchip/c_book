// echo_server.c
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
