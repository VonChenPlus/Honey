#include "MATH/AffineTransform.h"

#include <algorithm>
#include <math.h>

namespace MATH
{
    AffineTransform AffineTransformMake(float a, float b, float c, float d, float tx, float ty)
    {
        AffineTransform t;
        t.a = a; t.b = b; t.c = c; t.d = d; t.tx = tx; t.ty = ty;
        return t;
    }

    Vector2f PointApplyAffineTransform(const Vector2f& point, const AffineTransform& t)
    {
      Vector2f p;
      p.x = (float)((double)t.a * point.x + (double)t.c * point.y + t.tx);
      p.y = (float)((double)t.b * point.x + (double)t.d * point.y + t.ty);
      return p;
    }

    Vector2f PointApplyTransform(const Vector2f& point, const Matrix4& transform)
    {
        Vector3f vec(point.x, point.y, 0);
        transform.transformPoint(&vec);
        return Vector2f(vec.x, vec.y);
    }


    Sizef SizeApplyAffineTransform(const Sizef& size, const AffineTransform& t)
    {
      Sizef s;
      s.width = (float)((double)t.a * size.width + (double)t.c * size.height);
      s.height = (float)((double)t.b * size.width + (double)t.d * size.height);
      return s;
    }


    AffineTransform AffineTransformMakeIdentity()
    {
        return AffineTransformMake(1.0, 0.0, 0.0, 1.0, 0.0, 0.0);
    }

    extern const AffineTransform AffineTransformIdentity = AffineTransformMakeIdentity();
    const AffineTransform AffineTransform::IDENTITY = AffineTransformMakeIdentity();

    Rectf RectApplyAffineTransform(const Rectf& rect, const AffineTransform& anAffineTransform)
    {
        float top    = rect.minY();
        float left   = rect.minX();
        float right  = rect.maxX();
        float bottom = rect.maxY();

        Vector2f topLeft = PointApplyAffineTransform(Vector2f(left, top), anAffineTransform);
        Vector2f topRight = PointApplyAffineTransform(Vector2f(right, top), anAffineTransform);
        Vector2f bottomLeft = PointApplyAffineTransform(Vector2f(left, bottom), anAffineTransform);
        Vector2f bottomRight = PointApplyAffineTransform(Vector2f(right, bottom), anAffineTransform);

        float minX = std::min(std::min(topLeft.x, topRight.x), std::min(bottomLeft.x, bottomRight.x));
        float maxX = std::max(std::max(topLeft.x, topRight.x), std::max(bottomLeft.x, bottomRight.x));
        float minY = std::min(std::min(topLeft.y, topRight.y), std::min(bottomLeft.y, bottomRight.y));
        float maxY = std::max(std::max(topLeft.y, topRight.y), std::max(bottomLeft.y, bottomRight.y));

        return Rectf(minX, minY, (maxX - minX), (maxY - minY));
    }

    Rectf RectApplyTransform(const Rectf& rect, const Matrix4& transform)
    {
        float top    = rect.minY();
        float left   = rect.minX();
        float right  = rect.maxX();
        float bottom = rect.maxY();

        Vector3f topLeft(left, top, 0);
        Vector3f topRight(right, top, 0);
        Vector3f bottomLeft(left, bottom, 0);
        Vector3f bottomRight(right, bottom, 0);
        transform.transformPoint(&topLeft);
        transform.transformPoint(&topRight);
        transform.transformPoint(&bottomLeft);
        transform.transformPoint(&bottomRight);

        float minX = std::min(std::min(topLeft.x, topRight.x), std::min(bottomLeft.x, bottomRight.x));
        float maxX = std::max(std::max(topLeft.x, topRight.x), std::max(bottomLeft.x, bottomRight.x));
        float minY = std::min(std::min(topLeft.y, topRight.y), std::min(bottomLeft.y, bottomRight.y));
        float maxY = std::max(std::max(topLeft.y, topRight.y), std::max(bottomLeft.y, bottomRight.y));

        return Rectf(minX, minY, (maxX - minX), (maxY - minY));
    }


    AffineTransform AffineTransformTranslate(const AffineTransform& t, float tx, float ty)
    {
        return AffineTransformMake(t.a, t.b, t.c, t.d, t.tx + t.a * tx + t.c * ty, t.ty + t.b * tx + t.d * ty);
    }

    AffineTransform AffineTransformScale(const AffineTransform& t, float sx, float sy)
    {
        return AffineTransformMake(t.a * sx, t.b * sx, t.c * sy, t.d * sy, t.tx, t.ty);
    }

    AffineTransform AffineTransformRotate(const AffineTransform& t, float anAngle)
    {
        float sine = sinf(anAngle);
        float cosine = cosf(anAngle);

        return AffineTransformMake(    t.a * cosine + t.c * sine,
                                        t.b * cosine + t.d * sine,
                                        t.c * cosine - t.a * sine,
                                        t.d * cosine - t.b * sine,
                                        t.tx,
                                        t.ty);
    }

    /* Concatenate `t2' to `t1' and return the result:
         t' = t1 * t2 */
    AffineTransform AffineTransformConcat(const AffineTransform& t1, const AffineTransform& t2)
    {
        return AffineTransformMake(    t1.a * t2.a + t1.b * t2.c, t1.a * t2.b + t1.b * t2.d, //a,b
                                        t1.c * t2.a + t1.d * t2.c, t1.c * t2.b + t1.d * t2.d, //c,d
                                        t1.tx * t2.a + t1.ty * t2.c + t2.tx,                  //tx
                                        t1.tx * t2.b + t1.ty * t2.d + t2.ty);                  //ty
    }

    Matrix4 TransformConcat(const Matrix4& t1, const Matrix4& t2)
    {
        return t1 * t2;
    }

    /* Return true if `t1' and `t2' are equal, false otherwise. */
    bool AffineTransformEqualToTransform(const AffineTransform& t1, const AffineTransform& t2)
    {
        return (t1.a == t2.a && t1.b == t2.b && t1.c == t2.c && t1.d == t2.d && t1.tx == t2.tx && t1.ty == t2.ty);
    }

    AffineTransform AffineTransformInvert(const AffineTransform& t)
    {
        float determinant = 1 / (t.a * t.d - t.b * t.c);

        return AffineTransformMake(determinant * t.d, -determinant * t.b, -determinant * t.c, determinant * t.a,
                                determinant * (t.c * t.ty - t.d * t.tx), determinant * (t.b * t.tx - t.a * t.ty) );
    }
}
