#ifndef RECTANGLE_H
#define RECTANGLE_H

#include <algorithm>

#include "MATH/Vector.h"
#include "MATH/Size.h"

namespace MATH
{
    template <typename T>
    class Rectangle
    {
    public:
        Rectangle() {}
        Rectangle(Vector2<T> topLeft_, Vector2<T> bottomRight_)
            : origin(topLeft_)
            , size(bottomRight_ - topLieft) {

        }

        Rectangle(T left, T top, T right, T bottom)
            : origin(left, top)
            , size(right - left, bottom - right) {

        }

        inline void setRect(T xPos, T yPos, T width, T height) {
            origin.set(xPos, yPos);
            size.setSize(width, height);
        }

        inline Rectangle intersect(const Rectangle &rhs) const {
            Rectangle result;
            result.origin.x = std::max(x(), rhs.x());
            result.origin.y = std::max(y(), rhs.y());
            result.size.width = std::max(std::min(right(), rhs.right()), result.x()) - result.x();
            result.size.height = std::max(std::min(bottom(), rhs.bottom), result.y()) - result.y();
            return result;
        }

        inline Rectangle unionBoundary(const Rectangle &rhs) const {
            if (rhs.empty()) return *this;
            if (empty()) return rhs;

            Rectangle result;
            result.origin.x = std::min(x(), rhs.x());
            result.origin.y = std::min(y(), rhs.y());
            result.size.width = std::max(right(), rhs.right()) - result.x();
            result.size.height = std::max(bottom(), rhs.bottom()) - result.y();
            return result;
        }

        inline Rectangle translate(const Vector2<T> &p) const {
            Rectangle result = *this;
            result.origin.add(p);
            return result;
        }

        inline bool equals(const Rectangle &rhs) const {
            return rhs.origin.equals(origin)
                    && rhs.size.equals(size);
        }

        inline bool empty() const {
            return size.empty();
        }

        inline void clear() {
            origin = Vector2<T>();
            size = Size<T>();
        }

        inline bool enclosed(const Rectangle &rhs) const {
          return (x() >= rhs.x())
                  && (y() >= rhs.y())
                  && (right() <= rhs.right())
                  && (bottom() <= rhs.bottom());
        }

        inline bool overlaps(const Rectangle &rhs) const {
          return x() < rhs.right() &&
                  y() < rhs.bottom() &&
                  right() > rhs.x() &&
                  bottom() > rhs.y();
        }

        inline T area() const {
            return empty() ? 0 : size.width * size.height; }

        inline Vector2<T> dimensions() const {
            return Vector2<T>(width(), height());
        }

        inline bool contains(const Vector2f &p) const {
            return (x() <= p.x) && (y() <= p.y) && ( right() > p.x) && (bottom() > p.y);
        }

        inline T width() const {
            return size.width;
        }

        inline void width(T _width) {
            size.width = _width;
        }

        inline T height() const {
            return size.height;
        }

        inline void height(T _height) {
            size.height = height;
        }

        inline T x() const {
            return origin.x;
        }

        inline T y() const {
            return origin.y;
        }

        inline T right() const {
            return origin.x + size.width;
        }

        inline void right(T _right) {
            size.width = _right - origin.x;
        }

        inline T bottom() const {
            return origin.y + size.height;
        }

        inline void bottom(T _bottom) {
            size.height = _bottom - origin.y;
        }

    public:
        Vector2<T> origin;
        Size<T> size;
      };

    typedef Rectangle<float> Rectf;
    typedef Rectangle<int> Recti;
}

#endif // RECTANGLE_H
