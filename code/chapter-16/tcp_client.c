/* Simple TCP client
Build: gcc -Wall -Wextra -std=c11 -o tcp_client tcp_client.c
Run: ./tcp_client 127.0.0.1 8080
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

int main(int argc, char **argv) {
    if (argc != 3) { fprintf(stderr, "Usage: %s HOST PORT\n", argv[0]); return 1; }
    const char *host = argv[1]; int port = atoi(argv[2]);
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) { perror("socket"); return 1; }
    struct sockaddr_in addr = {0}; addr.sin_family = AF_INET; addr.sin_port = htons(port);
    if (inet_pton(AF_INET, host, &addr.sin_addr) <= 0) { perror("inet_pton"); close(fd); return 1; }
    if (connect(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) { perror("connect"); close(fd); return 1; }
    char buf[512]; while (fgets(buf, sizeof(buf), stdin)) {
        size_t len = strlen(buf);
        if (send(fd, buf, len, 0) != (ssize_t)len) { perror("send"); break; }
        ssize_t n = recv(fd, buf, sizeof(buf)-1, 0);
        if (n <= 0) break; buf[n]='\0'; printf("Echo: %s", buf);
    }
    close(fd);
    return 0;
}
