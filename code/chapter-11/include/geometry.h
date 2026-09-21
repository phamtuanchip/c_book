// include/geometry.h
#ifndef GEOMETRY_H
#define GEOMETRY_H

#include <stddef.h>          // size_t: header này cần tự include những gì nó dùng

typedef struct {
    double x, y;
} Point;

typedef struct {
    Point *pts;
    size_t count;
} Polygon;

/* Khoảng cách Euclid giữa a và b. */
double geo_distance(Point a, Point b);

/*
 * Diện tích đa giác theo công thức Shoelace.
 * Trả về 0 nếu poly có ít hơn 3 đỉnh.
 */
double geo_polygon_area(const Polygon *poly);

#endif /* GEOMETRY_H */
