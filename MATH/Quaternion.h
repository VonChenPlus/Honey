#ifndef QUATERNION_H
#define QUATERNION_H

#include "MATH/Vector.h"

namespace MATH
{
    class Matrix4;

    class Quaternion final
    {
    public:
        float x;
        float y;
        float z;
        float w;

        Quaternion();
        Quaternion(float xx, float yy, float zz, float ww);
        Quaternion(float* array);
        Quaternion(const Matrix4& m);
        Quaternion(const Vector3f& axis, float angle);
        Quaternion(const Quaternion& copy);
        ~Quaternion();

        static const Quaternion& identity();
        static const Quaternion& zero();
        static void createFromRotationMatrix(const Matrix4& m, Quaternion* dst);
        static void createFromAxisAngle(const Vector3f& axis, float angle, Quaternion* dst);
        static void multiply(const Quaternion& q1, const Quaternion& q2, Quaternion* dst);
        static void lerp(const Quaternion& q1, const Quaternion& q2, float t, Quaternion* dst);
        static void slerp(const Quaternion& q1, const Quaternion& q2, float t, Quaternion* dst);
        static void squad(const Quaternion& q1, const Quaternion& q2, const Quaternion& s1, const Quaternion& s2, float t, Quaternion* dst);

        bool isIdentity() const;
        bool isZero() const;

        void conjugate();
        Quaternion getConjugated() const;

        bool inverse();
        Quaternion getInversed() const;

        void multiply(const Quaternion& q);

        void normalize();
        Quaternion getNormalized() const;

        void set(float xx, float yy, float zz, float ww);
        void set(float* array);
        void set(const Matrix4& m);
        void set(const Vector3f& axis, float angle);
        void set(const Quaternion& q);
        void setIdentity();

        float toAxisAngle(Vector3f* e) const;

        inline const Quaternion operator*(const Quaternion& q) const;
        inline Vector3f operator*(const Vector3f& v) const;
        inline Quaternion& operator*=(const Quaternion& q);

        static const Quaternion ZERO;

    private:
        static void slerp(float q1x, float q1y, float q1z, float q1w, float q2x, float q2y, float q2z, float q2w, float t, float* dstx, float* dsty, float* dstz, float* dstw);

        static void slerpForSquad(const Quaternion& q1, const Quaternion& q2, float t, Quaternion* dst);
    };
}

#endif // QUATERNION_H

