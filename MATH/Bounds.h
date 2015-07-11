#ifndef BOUNDS_H
#define BOUNDS_H

#include "MATH/Vector.h"

namespace MATH
{
    // Resolved bounds on screen after layout.
    template <typename T>
    class Bounds
    {
    public:
        Bounds()
            : left(MATHZERO<T>())
            , top(MATHZERO<T>())
            , width(MATHZERO<T>())
            , height(MATHZERO<T>()) {

        }

        Bounds(const T &_x, const T &_y, const T &_w, const T &_h)
            : left(_x)
            , top(_y)
            , width(_w)
            , height(_h) {

        }

        bool contains(const T &px, const T &py) const {
            return (!(px < left || py < top) && px < left + width && py < top + height);
        }

        bool intersects(const Bounds &other) const {
            return !(left > other.right() || right() < other.left || top > other.bottom() || bottom() < other.top);
        }

        void clip(const Bounds &clipTo) {
            if (left < clipTo.left) {
                width -= clipTo.left - left;
                left = clipTo.left;
            }
            if (top < clipTo.top) {
                height -= clipTo.top - top;
                top = clipTo.top;
            }
            if (right() > clipTo.right()) {
                width = clipTo.right() - left;
            }
            if (bottom() > clipTo.bottom()) {
                height = clipTo.bottom() - top;
            }
        }

        T right() const { return left + width; }
        T bottom() const { return top + height; }
        T centerX() const { return left + width * 0.5f; }
        T centerY() const { return top + height * 0.5f; }
        Vector2<T> center() const {
            return Vector2<T>(centerX(), centerY());
        }

        T area() { return width * height; }

        Bounds expand(T amount) const {
            return Bounds(left - amount, top - amount, width + amount * 2, height + amount * 2);
        }
        Bounds offset(T xAmount, T yAmount) const {
            return Bounds(left + xAmount, top + yAmount, width, height);
        }
\
    public:
        T left;
        T top;
        T width;
        T height;
    };

    typedef Bounds<float> Boundsf;
}

#endif // BOUNDS_H

