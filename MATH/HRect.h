#ifndef NRECT_H
#define NRECT_H

#include <algorithm>
#include "MATH/Vector.h"

namespace MATH
{
    class Rect
    {
    public:
        Rect() {}
        Rect(Vector2f topLeft_, Vector2f bottomRight_) : topLeft(topLeft_), bottomRight(bottomRight_) {}
        Rect(int left, int top, int right, int bottom) : topLeft(left, top), bottomRight(right, bottom) {}
        inline void setRect(int xPos, int yPos, int width, int height) {
          topLeft.x = xPos; topLeft.y = yPos; bottomRight.x = xPos + width; bottomRight.y = yPos + height;
        }

        inline Rect intersect(const Rect &r) const {
          Rect result;
          result.topLeft.x = std::max(topLeft.x, r.topLeft.x);
          result.topLeft.y = std::max(topLeft.y, r.topLeft.y);
          result.bottomRight.x = std::max(std::min(bottomRight.x, r.bottomRight.x), result.topLeft.x);
          result.bottomRight.y = std::max(std::min(bottomRight.y, r.bottomRight.y), result.topLeft.y);
          return result;
        }

        inline Rect unionBoundary(const Rect &r) const {
          if (r.empty()) return *this;
          if (empty()) return r;
          Rect result;
          result.topLeft.x = std::min(topLeft.x, r.topLeft.x);
          result.topLeft.y = std::min(topLeft.y, r.topLeft.y);
          result.bottomRight.x = std::max(bottomRight.x, r.bottomRight.x);
          result.bottomRight.y = std::max(bottomRight.y, r.bottomRight.y);
          return result;
        }

        inline Rect translate(const Vector2f &p) const {
          return Rect(Vector2f::add(topLeft, p), Vector2f::add(bottomRight, p));
        }

        inline bool equals(const Rect &r) const {return r.topLeft.equals(topLeft) && r.bottomRight.equals(bottomRight);}
        inline bool empty() const {return (topLeft.x >= bottomRight.x) || (topLeft.y >= bottomRight.y);}
        inline void clear() {topLeft = Vector2f(); bottomRight = Vector2f();}
        inline bool enclosed_by(const Rect &r) const {
          return (topLeft.x >= r.topLeft.x) && (topLeft.y >= r.topLeft.y) && (bottomRight.x <= r.bottomRight.x) && (bottomRight.y <= r.bottomRight.y);
        }
        inline bool overlaps(const Rect &r) const {
          return topLeft.x < r.bottomRight.x && topLeft.y < r.bottomRight.y && bottomRight.x > r.topLeft.x && bottomRight.y > r.topLeft.y;
        }
        inline unsigned int area() const {return empty() ? 0 : (bottomRight.x - topLeft.x) * (bottomRight.y - topLeft.y);}
        inline Vector2f dimensions() const {return Vector2f(width(), height());}
        inline int width() const {return bottomRight.x - topLeft.x;}
        inline int height() const {return bottomRight.y - topLeft.y;}
        inline bool contains(const Vector2f &p) const {
          return (topLeft.x <= p.x) && (topLeft.y <= p.y) && ( bottomRight.x > p.x) && (bottomRight.y > p.y);
        }

        Vector2f topLeft;
        Vector2f bottomRight;
      };
}

#endif // NRECT_H
