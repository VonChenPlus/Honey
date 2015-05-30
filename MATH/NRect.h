#ifndef NRECT_H
#define NRECT_H

#include <algorithm>
#include "MATH/Point.h"

namespace MATH
{
    class Rect
    {
    public:
        Rect() {}
        Rect(Point topLeft_, Point bottomRight_) : topLeft(topLeft_), bottomRight(bottomRight_) {}
        Rect(int x1, int y1, int x2, int y2) : topLeft(x1, y1), bottomRight(x2, y2) {}
        inline void setXYWH(int x, int y, int w, int h) {
          topLeft.x = x; topLeft.y = y; bottomRight.x = x+w; bottomRight.y = y+h;
        }

        inline Rect intersect(const Rect &r) const {
          Rect result;
          result.topLeft.x = std::max(topLeft.x, r.topLeft.x);
          result.topLeft.y = std::max(topLeft.y, r.topLeft.y);
          result.bottomRight.x = std::max(std::min(bottomRight.x, r.bottomRight.x), result.topLeft.x);
          result.bottomRight.y = std::max(std::min(bottomRight.y, r.bottomRight.y), result.topLeft.y);
          return result;
        }

        inline Rect union_boundary(const Rect &r) const {
          if (r.empty()) return *this;
          if (empty()) return r;
          Rect result;
          result.topLeft.x = std::min(topLeft.x, r.topLeft.x);
          result.topLeft.y = std::min(topLeft.y, r.topLeft.y);
          result.bottomRight.x = std::max(bottomRight.x, r.bottomRight.x);
          result.bottomRight.y = std::max(bottomRight.y, r.bottomRight.y);
          return result;
        }

        inline Rect translate(const Point &p) const {
          return Rect(topLeft.translate(p), bottomRight.translate(p));
        }

        inline bool equals(const Rect &r) const {return r.topLeft.equals(topLeft) && r.bottomRight.equals(bottomRight);}
        inline bool empty() const {return (topLeft.x >= bottomRight.x) || (topLeft.y >= bottomRight.y);}
        inline void clear() {topLeft = Point(); bottomRight = Point();}
        inline bool enclosed_by(const Rect &r) const {
          return (topLeft.x >= r.topLeft.x) && (topLeft.y >= r.topLeft.y) && (bottomRight.x <= r.bottomRight.x) && (bottomRight.y <= r.bottomRight.y);
        }
        inline bool overlaps(const Rect &r) const {
          return topLeft.x < r.bottomRight.x && topLeft.y < r.bottomRight.y && bottomRight.x > r.topLeft.x && bottomRight.y > r.topLeft.y;
        }
        inline unsigned int area() const {return empty() ? 0 : (bottomRight.x - topLeft.x) * (bottomRight.y - topLeft.y);}
        inline Point dimensions() const {return Point(width(), height());}
        inline int width() const {return bottomRight.x - topLeft.x;}
        inline int height() const {return bottomRight.y - topLeft.y;}
        inline bool contains(const Point &p) const {
          return (topLeft.x <= p.x) && (topLeft.y <= p.y) && ( bottomRight.x > p.x) && (bottomRight.y > p.y);
        }

        Point topLeft;
        Point bottomRight;
      };
}

#endif // NRECT_H
