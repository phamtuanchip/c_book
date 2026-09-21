# Chương 5 — Điều khiển luồng (Control Flow)

Mục tiêu chương:

- Hiểu và phân biệt if/else, switch, for, while, do-while.
- Biết dùng break và continue hợp lý.
- Viết chương trình kiểm soát luồng với input hợp lệ và tránh infinite loop.

1. If / else

Cấu trúc:

if (condition) {
    // code
} else if (other_condition) {
    // code
} else {
    // code
}

- Condition là biểu thức trả về giá trị boolean (trong C, 0 = false, khác 0 = true).
- Short-circuit: với && và ||, C sẽ không đánh giá biểu thức phải nếu kết quả đã rõ.

Ví dụ: phân loại số

2. Switch

switch (expr) {
    case 1: /* ... */ break;
    case 2: /* ... */ break;
    default: /* ... */
}

- Lưu ý fall-through: nếu thiếu break, execution sẽ tiếp tục case tiếp theo.
- Thường dùng cho lựa chọn rời rạc (menu, xử lý enum).

3. Vòng lặp: for, while, do-while

for (int i = 0; i < n; i++) { /* ... */ }
while (condition) { /* ... */ }
do { /* ... */ } while (condition);

- for thích hợp khi biết số lần lặp. while/do-while dùng cho lặp theo điều kiện.
- break: thoát vòng lặp; continue: bỏ phần còn lại và sang lần lặp tiếp theo.

4. Tránh infinite loop và lỗi logic

- Đảm bảo điều kiện vòng lặp sẽ thay đổi theo cách dẫn tới kết thúc.
- Thận trọng khi dùng biến unsigned trong phép so sánh (có thể vòng lặp không kết thúc).

5. Ví dụ thực hành

- In bảng cửu chương (for)
- Kiểm tra số nguyên tố (for with early exit)
- Menu đơn giản dùng switch

6. Bài tập

1) Viết chương trình in bảng cửu chương của 1..9 dùng for.
2) Viết hàm is_prime(int n) và test từ 2..100.
3) Viết menu: [1] In bảng cửu chương, [2] Kiểm tra số nguyên tố, [0] Thoát.

Gợi ý biên dịch: gcc -std=c11 -Wall -Wextra -o prog file.c

Ghi chú: kèm file mẫu trong /code/chapter-05 có README hướng dẫn và test cases.