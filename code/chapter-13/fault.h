// fault.h
#ifdef FAULT_INJECTION
extern int g_fail_after;               // số lần gọi thành công trước khi cấp phát bắt đầu lỗi
void *test_malloc(size_t n);
#define MALLOC(n) test_malloc(n)
#else
#define MALLOC(n) malloc(n)
#endif
