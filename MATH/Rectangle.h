#ifndef RECTANGLE_H
#define RECTANGLE_H

#include <algorithm>

#include "MATH/Vector.h"

namespace MATH
{
    template <typename T>
    class Rectangle
    {
    public:
        Rectangle() {}
        Rectangle(Vector2<T> topLeft_, Vector2<T> bottomRight_)
            : topLeft(topLeft_)
            , bottomRight(bottomRight_) {

        }

        Rectangle(T left, T top, T right, T bottom)
            : topLeft(left, top)
            , bottomRight(right, bottom) {

        }

        inline void setRect(T xPos, T yPos, T width, T height) {
            topLeft.x = xPos;
            topLeft.y = yPos;
            bottomRight.x = xPos + width;
            bottomRight.y = yPos + height;
        }

        inline Rectangle intersect(const Rectangle &rhs) const {
          Rectangle result;
          result.topLeft.x = std::max(topLeft.x, rhs.topLeft.x);
          result.topLeft.y = std::max(topLeft.y, rhs.topLeft.y);
          result.bottomRight.x = std::max(std::min(bottomRight.x, rhs.bottomRight.x), result.topLeft.x);
          result.bottomRight.y = std::max(std::min(bottomRight.y, rhs.bottomRight.y), result.topLeft.y);
          return result;
        }

        inline Rectangle unionBoundary(const Rectangle &rhs) const {
          if (rhs.empty()) return *this;
          if (empty()) return rhs;

          Rectangle result;
          result.topLeft.x = std::min(topLeft.x, rhs.topLeft.x);
          result.topLeft.y = std::min(topLeft.y, rhs.topLeft.y);
          result.bottomRight.x = std::max(bottomRight.x, rhs.bottomRight.x);
          result.bottomRight.y = std::max(bottomRight.y, rhs.bottomRight.y);
          return result;
        }

        inline Rectangle translate(const Vector2<T> &p) const {
          return Rectangle(Vector2<T>::add(topLeft, p), Vector2f::add(bottomRight, p));
        }

        inline bool equals(const Rectangle &r) const {
            return r.topLeft.equals(topLeft)
                    && r.bottomRight.equals(bottomRight);
        }

        inline bool empty() const {
            return !(topLeft < bottomRight);
        }

        inline void clear() {
            topLeft = Vector2<T>();
            bottomRight = Vector2<T>();
        }

        inline bool enclosed(const Rectangle &rhs) const {
          return (topLeft.x >= rhs.topLeft.x)
                  && (topLeft.y >= rhs.topLeft.y)
                  && (bottomRight.x <= rhs.bottomRight.x)
                  && (bottomRight.y <= rhs.bottomRight.y);
        }

        inline bool overlaps(const Rectangle &rhs) const {
          return topLeft.x < rhs.bottomRight.x &&
                  topLeft.y < rhs.bottomRight.y &&
                  bottomRight.x > rhs.topLeft.x &&
                  bottomRight.y > rhs.topLeft.y;
        }

        inline T area() const {
            return empty() ? 0 : (bottomRight.x - topLeft.x) * (bottomRight.y - topLeft.y);}

        inline Vector2<T> dimensions() const {
            return Vector2<T>(width(), height());
        }

        inline T width() const {
            return bottomRight.x - topLeft.x;
        }

        inline T height() const {
            return bottomRight.y - topLeft.y;
        }

        inline bool contains(const Vector2f &p) const {
          return (topLeft.x <= p.x) && (topLeft.y <= p.y) && ( bottomRight.x > p.x) && (bottomRight.y > p.y);
        }

    public:
        Vector2<T> topLeft;
        Vector2<T> bottomRight;
      };

    typedef Rectangle<float> Rectf;
    typedef Rectangle<int> Recti;
}

#endif // RECTANGLE_H
