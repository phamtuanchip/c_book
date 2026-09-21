// deadlock.c
#define _POSIX_C_SOURCE 200809L
#include <pthread.h>
#include <unistd.h>

static pthread_mutex_t A = PTHREAD_MUTEX_INITIALIZER, B = PTHREAD_MUTEX_INITIALIZER;

static void *t1(void *x) { (void)x; pthread_mutex_lock(&A); sleep(1); pthread_mutex_lock(&B); pthread_mutex_unlock(&B); pthread_mutex_unlock(&A); return NULL; }
static void *t2(void *x) { (void)x; pthread_mutex_lock(&B); sleep(1); pthread_mutex_lock(&A); pthread_mutex_unlock(&A); pthread_mutex_unlock(&B); return NULL; }

int main(void) {
    pthread_t a, b;
    pthread_create(&a, NULL, t1, NULL);
    pthread_create(&b, NULL, t2, NULL);
    pthread_join(a, NULL);        // treo mãi
    pthread_join(b, NULL);
    return 0;
}
