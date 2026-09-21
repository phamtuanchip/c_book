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
