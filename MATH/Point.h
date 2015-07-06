#ifndef POINT_H
#define POINT_H

#include <cmath>

namespace MATH
{
    struct Point
    {
        Point() {}
        Point(double x_, double y_) : x(x_), y(y_) {}

        double distanceTo(const Point &other) const {
            double dx = other.x - x, dy = other.y - y;
            return sqrt(dx*dx + dy*dy);
        }

        inline bool equals(const Point &p) const {return x == p.x && y == p.y;}
        inline Point translate(const Point &p) const {return Point(x + p.x, y + p.y);}

        double x;
        double y;
    };
}

#endif // POINT_H

