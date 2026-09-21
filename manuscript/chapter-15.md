# Chương 15 — Đa luồng & đồng bộ (POSIX threads)

Mục tiêu chương:

- Hiểu cách tạo/hủy thread bằng pthreads, đồng bộ hóa bằng mutex và condition variables.
- Nhận diện race condition, deadlock và cách phòng tránh.

1. Tạo và quản lý thread

- pthread_create, pthread_join: tạo thread và đợi thread kết thúc.
- Truyền tham số vào thread bằng struct, tránh truyền địa chỉ biến cục bộ đã biến mất.

2. Mutex và bảo vệ shared data

- pthread_mutex_t mutex; pthread_mutex_lock(&mutex); /* critical section */ pthread_mutex_unlock(&mutex);
- Giữ critical sections ngắn, tránh lock ordering khác nhau gây deadlock.

3. Condition variables

- pthread_cond_wait, pthread_cond_signal: dùng cho producer-consumer pattern.
- Luôn khóa mutex trước khi gọi pthread_cond_wait.

4. Race condition, deadlock và strategies

- Race: truy cập shared state không được bảo vệ.
- Deadlock: avoid by ordering locks consistently, using trylock or lock hierarchy.

5. Tools và debugging

- Helgrind (Valgrind tool) và ThreadSanitizer (TSAN) để phát hiện race.

Bài tập

- Cài đặt producer-consumer với buffer giới hạn: multiple producers, multiple consumers, và kiểm thử với threads.

Ghi chú: kèm code trong /code/chapter-15 với Makefile để build trên POSIX; trên Windows hướng dẫn tương đương với Win32 threads.