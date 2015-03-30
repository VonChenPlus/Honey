#ifndef POINT_H
#define POINT_H

#include <cmath>

namespace MATH
{
    struct Point
    {
        Point() {}
        Point(float x_, float y_) : x(x_), y(y_) {}

        float x;
        float y;

        float distanceTo(const Point &other) const
        {
            float dx = other.x - x, dy = other.y - y;
            return sqrtf(dx*dx + dy*dy);
        }

        /*
        FocusDirection directionTo(const Point &other) const {
            int angle = atan2f(other.y - y, other.x - x) / (2 * M_PI) - 0.125;

        }*/
    };
}

#endif // POINT_H

