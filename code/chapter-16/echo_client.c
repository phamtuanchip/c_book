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
