/* Simple TCP echo server (IPv4) - blocking, single-threaded
Build: gcc -Wall -Wextra -std=c11 -o tcp_server tcp_server.c
Run: ./tcp_server 8080
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

int main(int argc, char **argv) {
    if (argc != 2) { fprintf(stderr, "Usage: %s PORT\n", argv[0]); return 1; }
    int port = atoi(argv[1]);
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0) { perror("socket"); return 1; }
    int opt = 1; setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET; addr.sin_addr.s_addr = INADDR_ANY; addr.sin_port = htons(port);
    if (bind(listen_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) { perror("bind"); close(listen_fd); return 1; }
    if (listen(listen_fd, 5) < 0) { perror("listen"); close(listen_fd); return 1; }
    printf("Listening on port %d\n", port);
    while (1) {
        struct sockaddr_in peer; socklen_t plen = sizeof(peer);
        int fd = accept(listen_fd, (struct sockaddr*)&peer, &plen);
        if (fd < 0) { perror("accept"); continue; }
        char host[INET_ADDRSTRLEN]; inet_ntop(AF_INET, &peer.sin_addr, host, sizeof(host));
        printf("Connection from %s:%d\n", host, ntohs(peer.sin_port));
        char buf[1024]; ssize_t n;
        while ((n = recv(fd, buf, sizeof(buf), 0)) > 0) {
            send(fd, buf, n, 0); // echo back
        }
        close(fd);
        printf("Connection closed\n");
    }
    close(listen_fd);
    return 0;
}
