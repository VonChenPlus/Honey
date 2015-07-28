#ifndef AFFINETRANSFORM_H
#define AFFINETRANSFORM_H

#include "MATH/Size.h"
#include "MATH/Rectangle.h"
#include "MATH/Matrix.h"
#include "MATH/Vector.h"

namespace MATH
{
    /**@{
     Affine transform
     a   b    0
     c   d    0
     tx  ty   1

     Identity
     1   0    0
     0   1    0
     0   0    1
     */
    struct AffineTransform {
        float a, b, c, d;
        float tx, ty;

        static const AffineTransform IDENTITY;
    };

    /**@}*/

    /**Make affine transform.*/
    AffineTransform AffineTransformMake(float a, float b, float c, float d, float tx, float ty);

    /**Multiply point (x,y,1) by a  affine tranform.*/
    Vector2f PointApplyAffineTransform(const Vector2f& point, const AffineTransform& t);

    /**Multiply size (width,height,0) by a  affine tranform.*/
    Sizef SizeApplyAffineTransform(const Sizef& size, const AffineTransform& t);

    /**Make identity affine transform.*/
    AffineTransform AffineTransformMakeIdentity();
    /**Transform Rect, which will transform the four vertice of the point.*/
    Rectf RectApplyAffineTransform(const Rectf& rect, const AffineTransform& anAffineTransform);
    /**@{
     Transform Vector2f and Rect by Matrix4.
     */
    Rectf RectApplyTransform(const Rectf& rect, const Matrix4& transform);
    Vector2f PointApplyTransform(const Vector2f& point, const Matrix4& transform);
    /**@}*/
    /**
     Translation, equals
     1  0  1
     0  1  0   * affinetransform
     tx ty 1
     */
    AffineTransform AffineTransformTranslate(const AffineTransform& t, float tx, float ty);
    /**
     Rotation, equals
     cos(angle)   sin(angle)   0
     -sin(angle)  cos(angle)   0  * AffineTransform
     0            0            1
     */
    AffineTransform AffineTransformRotate(const AffineTransform& aTransform, float anAngle);
    /**
     Scale, equals
     sx   0   0
     0    sy  0  * affineTransform
     0    0   1
     */
    AffineTransform AffineTransformScale(const AffineTransform& t, float sx, float sy);
    /**Concat two affine transform, t1 * t2*/
    AffineTransform AffineTransformConcat(const AffineTransform& t1, const AffineTransform& t2);
    /**Compare affine transform.*/
    bool AffineTransformEqualToTransform(const AffineTransform& t1, const AffineTransform& t2);
    /**Get the inverse of affine transform.*/
    AffineTransform AffineTransformInvert(const AffineTransform& t);
    /**Concat Matrix4, return t1 * t2.*/
    Matrix4 TransformConcat(const Matrix4& t1, const Matrix4& t2);

    extern const AffineTransform AffineTransformIdentity;
}

#endif // AFFINETRANSFORM_H
