// src/geometry.c
#include "geometry.h"
#include <math.h>

double geo_distance(Point a, Point b) {
    double dx = a.x - b.x, dy = a.y - b.y;
    return sqrt(dx * dx + dy * dy);
}

double geo_polygon_area(const Polygon *poly) {
    if (poly == NULL || poly->count < 3) return 0.0;
    double sum = 0.0;
    for (size_t i = 0; i < poly->count; i++) {
        Point p = poly->pts[i];
        Point q = poly->pts[(i + 1) % poly->count];
        sum += p.x * q.y - q.x * p.y;
    }
    return fabs(sum) / 2.0;
}
