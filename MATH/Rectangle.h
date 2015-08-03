#ifndef RECTANGLE_H
#define RECTANGLE_H

#include <algorithm>

#include "MATH/Vector.h"
#include "MATH/Size.h"

namespace MATH
{
    template <typename T>
    class Rectangle final
    {
    public:
        Rectangle() {}
        Rectangle(Vector2<T> topLeft, Vector2<T> bottomRight)
            : origin(topLeft)
            , size(bottomRight - topLeft) {

        }

        Rectangle(T left, T top, T width, T height)
            : origin(left, top)
            , size(width, height) {
        }

        Rectangle(const Rectangle &other) {
            setRect(other.minX(), other.minY(), other.width(), other.height());
        }

        Rectangle& operator= (const Rectangle& other) {
            setRect(other.minX(), other.minY(), other.width(), other.height());
            return *this;
        }

        inline void setRect(T xPos, T yPos, T width, T height) {
            origin.set(xPos, yPos);
            size.setSize(width, height);
        }

        inline bool intersect(const Rectangle &rhs) const {
            return !(maxX() < rhs.minX() ||
                    rhs.maxX() < minX() ||
                    maxY() < rhs.minY() ||
                    rhs.maxY() < minY());
        }

        inline bool contains(const Vector2f &p) const {
            return (minX() <= p.x) && (minY() <= p.y) && (maxX() > p.x) && (maxY() > p.y);
        }

        inline void intersect(const Rectangle &rhs) {
            origin.x = std::max(minX(), rhs.minX());
            origin.y = std::max(minY(), rhs.minY());
            size.width = std::max(std::min(maxX(), rhs.maxX()), minX()) - minX();
            size.height = std::max(std::min(maxY(), rhs.maxY()), minY()) - minY();
        }

        inline void merge(const Rectangle &rhs) {
            if (rhs.empty()) return;
            if (empty()) {
                *this = rhs;
                return;
            }

            origin.x = std::min(minX(), rhs.minX());
            origin.y = std::min(minY(), rhs.minY());
            size.width = std::max(maxX(), rhs.maxX()) - minX();
            size.height = std::max(maxY(), rhs.maxY()) - minY();
        }

        inline void translate(const Vector2<T> &p) {
            origin.add(p);
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
          return (minX() >= rhs.minX())
                  && (minY() >= rhs.minY())
                  && (maxX() <= rhs.maxX())
                  && (maxY() <= rhs.maxY());
        }

        inline bool overlaps(const Rectangle &rhs) const {
          return minX() < rhs.maxX() &&
                  minY() < rhs.maxY() &&
                  maxX() > rhs.minX() &&
                  maxY() > rhs.minY();
        }

        inline T area() const {
            return empty() ? 0 : size.width * size.height; }

        inline Vector2<T> dimensions() const {
            return Vector2<T>(width(), height());
        }

        inline T width() const {
            return size.width;
        }

        inline T height() const {
            return size.height;
        }

        inline T minX() const {
            return origin.x;
        }

        inline T minY() const {
            return origin.y;
        }

        inline T maxX() const {
            return origin.x + size.width;
        }

        inline T maxY() const {
            return origin.y + size.height;
        }

        inline T midX() const {
            return T(origin.x + size.width / 2.0f);
        }

        inline T midY() const {
            return T(origin.y + size.height / 2.0f);
        }

    public:
        Vector2<T> origin;
        Size<T> size;
      };

    typedef Rectangle<float> Rectf;
    static const Rectf RectfZERO(0.0f, 0.0f, 0.0f, 0.0f);
    typedef Rectangle<int> Recti;
}

#endif // RECTANGLE_H
