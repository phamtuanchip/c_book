// sigint.c
#define _POSIX_C_SOURCE 200809L
#include <signal.h>
#include <stdio.h>
#include <unistd.h>

static volatile sig_atomic_t g_count = 0;

static void on_sigint(int sig) {
    (void)sig;
    if (++g_count >= 2) _exit(130);            // lần hai: thoát ngay (chỉ dùng hàm an toàn với tín hiệu)
}

int main(void) {
    struct sigaction sa;
    sa.sa_handler = on_sigint;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);

    FILE *f = fopen("work.tmp", "w");
    while (g_count == 0) {
        if (f) fputs("tick\n", f);
        sleep(1);
    }
    printf("nhan Ctrl+C: dong file va thoat em\n");     // ngoài handler nên dùng printf được
    if (f) fclose(f);
    return 0;
}
