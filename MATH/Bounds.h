#ifndef BOUNDS_H
#define BOUNDS_H

#include "MATH/Point.h"

namespace MATH
{
    // Resolved bounds on screen after layout.
    struct Bounds
    {
        Bounds() : x(0), y(0), w(0), h(0) {}
        Bounds(float x_, float y_, float w_, float h_) : x(x_), y(y_), w(w_), h(h_) {}

        bool contains(float px, float py) const {
            return (px >= x && py >= y && px < x + w && py < y + h);
        }

        bool intersects(const Bounds &other) const {
            return !(x > other.x2() || x2() < other.x || y > other.y2() || y2() < other.y);
        }

        void clip(const Bounds &clipTo) {
            if (x < clipTo.x) {
                w -= clipTo.x - x;
                x = clipTo.x;
            }
            if (y < clipTo.y) {
                h -= clipTo.y - y;
                y = clipTo.y;
            }
            if (x2() > clipTo.x2()) {
                w = clipTo.x2() - x;
            }
            if (y2() > clipTo.y2()) {
                h = clipTo.y2() - y;
            }
        }

        float x2() const { return x + w; }
        float y2() const { return y + h; }
        float centerX() const { return x + w * 0.5f; }
        float centerY() const { return y + h * 0.5f; }
        Point center() const {
            return Point(centerX(), centerY());
        }
        Bounds expand(float amount) const {
            return Bounds(x - amount, y - amount, w + amount * 2, h + amount * 2);
        }
        Bounds offset(float xAmount, float yAmount) const {
            return Bounds(x + xAmount, y + yAmount, w, h);
        }

        float x;
        float y;
        float w;
        float h;
    };
}

#endif // BOUNDS_H

