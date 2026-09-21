// platform.c
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>

#if defined(_WIN32)
  #include <windows.h>
  static void sleep_ms(unsigned ms) { Sleep(ms); }
  #define OS_NAME "Windows"
#elif defined(__unix__) || defined(__APPLE__)
  #include <time.h>
  static void sleep_ms(unsigned ms) {
      struct timespec ts = { ms / 1000, (long)(ms % 1000) * 1000000L };
      nanosleep(&ts, NULL);
  }
  #if defined(__APPLE__)
    #define OS_NAME "macOS"
  #else
    #define OS_NAME "Linux/Unix"
  #endif
#else
  #error "nen tang khong duoc ho tro"
#endif

int main(void) {
    printf("chay tren %s\n", OS_NAME);
    sleep_ms(100);
    return 0;
}
