#ifndef POINT_H
#define POINT_H

#include <cmath>

namespace MATH
{
    struct Point
    {
        Point() {}
        Point(float x_, float y_) : x(x_), y(y_) {}

        float distanceTo(const Point &other) const {
            float dx = other.x - x, dy = other.y - y;
            return sqrtf(dx*dx + dy*dy);
        }

        inline bool equals(const Point &p) const {return x == p.x && y == p.y;}
        inline Point translate(const Point &p) const {return Point(x + p.x, y + p.y);}

        float x;
        float y;
    };
}

#endif // POINT_H

