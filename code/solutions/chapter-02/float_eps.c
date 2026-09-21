// float_eps.c
#include <math.h>
#include <stdio.h>

int main(void) {
    float a = 0.1f, b = 0.2f, c = 0.3f;
    printf("a + b = %.9f, c = %.9f\n", a + b, c);
    printf("a + b == c ? %s\n", (a + b == c) ? "dung" : "sai");

    // Tìm epsilon nhỏ nhất (theo lũy thừa của 2) khiến so sánh gần đúng thành công
    float diff = fabsf((a + b) - c);
    printf("|(a+b) - c| = %.10g\n", diff);
    if (diff == 0.0f) {
        printf("hai gia tri bang nhau chinh xac\n");
    } else {
        for (float eps = 1.0f; eps > 0.0f; eps /= 2.0f) {
            if (diff > eps) {
                printf("epsilon nho nhat (luy thua 2) de a+b ~= c: %.10g\n", eps * 2.0f);
                break;
            }
        }
    }
    return 0;
}
