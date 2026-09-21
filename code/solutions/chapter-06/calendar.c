// calendar.c
#include <stdbool.h>
#include <stdio.h>

static bool is_leap_year(int y) {
    return (y % 4 == 0 && y % 100 != 0) || (y % 400 == 0);
}

static int days_in_month(int m, int y) {
    switch (m) {
        case 4: case 6: case 9: case 11: return 30;
        case 2: return is_leap_year(y) ? 29 : 28;
        case 1: case 3: case 5: case 7: case 8: case 10: case 12: return 31;
        default: return -1;
    }
}

int main(void) {
    printf("2000: %d, 1900: %d, 2024: %d\n", is_leap_year(2000), is_leap_year(1900), is_leap_year(2024));   // 1 0 1
    printf("thang 2/2024 co %d ngay\n", days_in_month(2, 2024));                                            // 29
    return 0;
}
