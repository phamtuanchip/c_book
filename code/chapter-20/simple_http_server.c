/* Minimal HTTP server (single-threaded) serving a fixed response
Build: gcc -Wall -Wextra -std=c11 -o http_server simple_http_server.c
Run: ./http_server 8080
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

const char response[] = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: 13\r\n\r\nHello, world!";

int main(int argc, char **argv) {
    if (argc != 2) { fprintf(stderr, "Usage: %s PORT\n", argv[0]); return 1; }
    int port = atoi(argv[1]);
    int s = socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0) { perror("socket"); return 1; }
    int opt = 1; setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    struct sockaddr_in addr = {0}; addr.sin_family = AF_INET; addr.sin_addr.s_addr = INADDR_ANY; addr.sin_port = htons(port);
    if (bind(s, (struct sockaddr*)&addr, sizeof(addr)) < 0) { perror("bind"); close(s); return 1; }
    if (listen(s, 5) < 0) { perror("listen"); close(s); return 1; }
    printf("HTTP server listening on %d\n", port);
    while (1) {
        int fd = accept(s, NULL, NULL);
        if (fd < 0) { perror("accept"); continue; }
        char buf[2048]; ssize_t n = recv(fd, buf, sizeof(buf)-1, 0);
        if (n > 0) {
            buf[n] = '\0';
            // naive parse: ignore and always respond
            send(fd, response, sizeof(response)-1, 0);
        }
        close(fd);
    }
    close(s);
    return 0;
}
