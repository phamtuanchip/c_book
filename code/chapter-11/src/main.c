// src/main.c
#include <stdio.h>
#include "geometry.h"

int main(void) {
    Point square[] = {{0, 0}, {4, 0}, {4, 3}, {0, 3}};
    Polygon poly = { square, 4 };
    printf("dien tich = %.2f\n", geo_polygon_area(&poly));           // 12.00
    printf("khoang cach = %.2f\n", geo_distance(square[0], square[2]));   // 5.00
    return 0;
}
