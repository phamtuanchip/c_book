// udp_ping.c
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
