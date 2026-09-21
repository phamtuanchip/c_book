# Chương 14 — Tiền xử lý & macro nâng cao

Mục tiêu chương:

- Hiểu chi tiết về macro, các lỗi phổ biến, và khi nào nên dùng inline function thay vì macro.
- Học X-macros và conditional compilation để tổ chức code linh hoạt.

1. Parametric macros, stringification và token-pasting

- #define SQR(x) ((x)*(x)) — ví dụ đơn giản nhưng có side-effects nếu x có tác dụng phụ.
- # và ##: stringification (#x) và token-pasting (a ## b) hữu ích cho code-gen trong C.

2. Tránh side-effects trong macro

- Macro không kiểm tra kiểu và có thể evaluate tham số nhiều lần. Giải pháp: dùng inline function cho các phép toán an toàn.

3. X-macros pattern

- X-macros giúp giữ danh sách hằng số/structs/handlers ở một nơi và generate code lặp lại an toàn.
- Ví dụ: định nghĩa danh sách lỗi một nơi và generate enum + string table.

4. Conditional compilation

- #ifdef FEATURE_X để bật/tắt code theo nền tảng hoặc flags biên dịch.
- Sử dụng feature test macros để kiểm tra hàm thư viện của platform.

5. Ví dụ thực hành

- Viết LOG(level, fmt, ...) macro và so sánh với static inline void log(level, const char*, ...).
- Dùng X-macro để generate enum và lookup table cho các lỗi.

Bài tập

- Viết X-macro cho danh sách lệnh CLI và generate switch-case xử lý tự động.

Ghi chú: kèm ví dụ trong /code/chapter-14, và so sánh compile-time outputs.