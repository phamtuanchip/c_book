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
