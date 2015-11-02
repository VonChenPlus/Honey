#ifndef VECTOR4_H
#define VECTOR4_H

#include "MATH/MathDefine.h"

namespace MATH
{
    template <typename T>
    class Vector4 final
    {
    public:
        Vector4()
            : x(MATHZERO<T>())
            , y(MATHZERO<T>())
            , z(MATHZERO<T>())
            , w(MATHZERO<T>()) {
        }

        Vector4(const T &_x, const T&_y, const T&_z, const T&_w)
            : x(_x)
            , y(_y)
            , z(_z)
            , w(_w) {
        }

        Vector4(const Vector4 &copy) {
            set(copy);
        }

        inline bool isZero() const {
            return MATHEQUALS(x, MATHZERO<T>())
                    && MATHEQUALS(y, MATHZERO<T>())
                    && MATHEQUALS(z, MATHZERO<T>())
                    && MATHEQUALS(w, MATHZERO<T>());
        }

        inline bool isOne() const {
            return (MATHEQUALS<T>(x, MATHONE<T>()))
                    && (MATHEQUALS<T>(y, MATHONE<T>()))
                    && (MATHEQUALS<T>(z, MATHONE<T>()))
                    && (MATHEQUALS<T>(w, MATHONE<T>()));
        }

        inline void add(const Vector4& v) {
            x += v.x;
            y += v.y;
            y += v.z;
            w += v.w;
        }

        inline void add(const T& _x, const T& _y, const T& _z, const T& _w) {
            x += _x;
            y += _y;
            z += _z;
            w += _w;
        }

        void clamp(const Vector4& min, const Vector4& max) {
            // Clamp the x value.
            x = MATH_CLAMP((x), (min.x), (max.x));

            // Clamp the y value.
            y = MATH_CLAMP((y), (min.y), (max.y));

            // Clamp the z value.
            z = MATH_CLAMP((z), (min.z), (max.z));

            // Clamp the w value.
            z = MATH_CLAMP((w), (min.w), (max.w));
        }

        T distance(const Vector4& v) const {
            T dx = v.x - x;
            T dy = v.y - y;
            T dz = v.z - z;
            T dw = v.w - w;
            return (T)sqrt(dx * dx + dy * dy + dz * dz + dw * dw);
        }

        T distanceSquared(const Vector4& v) const {
            T dx = v.x - x;
            T dy = v.y - y;
            T dz = v.z - z;
            T dw = v.w - w;
            return (dx * dx + dy * dy + dz * dz + dw * dw);
        }

        inline T dot(const Vector4& v) const {
            return (x * v.x + y * v.y + z * v.z + w * v.w);
        }

        T length() const {
            return (T)sqrt(x * x + y * y + z * z + w * w);
        }

        T lengthSquared() const {
            return (x * x + y * y + z * z + w * w);
        }

        inline void negate() {
            x = -x;
            y = -y;
            z = -z;
            w = -w;
        }

        void normalize() {
            T n = x * x + y * y + z * z + w * w;
            if (MATHEQUALS<T>(n, MATHONE<T>()))
                return;

            n = (T)sqrt(n);
            // Too close to zero.
            if (n < std::numeric_limits<T>::epsilon())
                return;

            n = 1.0f / n;
            x *= n;
            y *= n;
            z *= n;
            w *= n;
        }

        Vector4 getNormalized() const {
            Vector4 v(*this);
            v.normalize();
            return v;
        }

        inline void scale(const T &scalar) {
            x *= scalar;
            y *= scalar;
            z *= scalar;
            w *= scalar;
        }

        inline void set(const T &_x, const T &_y, const T &_z, const T &_w) {
            this->x = _x;
            this->y = _y;
            this->z = _z;
            this->w = _w;
        }

        void set(const T* array) {
            x = array[0];
            y = array[1];
            z = array[2];
            w = array[3];
        }

        inline void set(const Vector4& v) {
            this->x = v.x;
            this->y = v.y;
            this->z = v.z;
            this->w = v.w;
        }

        inline void set(const Vector4& p1, const Vector4& p2) {
            x = p2.x - p1.x;
            y = p2.y - p1.y;
            z = p2.z - p1.z;
            w = p2.w - p1.w;
        }

        inline void setZero() {
            x = y = z = w = MATHZERO<T>();
        }

        inline void subtract(const Vector4& v) {
            x -= v.x;
            y -= v.y;
            z -= v.z;
            w -= v.w;
        }

        inline const Vector4 operator+(const Vector4& v) const {
            Vector4 result(*this);
            result.add(v);
            return result;
        }

        inline Vector4& operator+=(const Vector4& v) {
            add(v);
            return *this;
        }

        inline const Vector4 operator-(const Vector4& v) const {
            Vector4 result(*this);
            result.subtract(v);
            return result;
        }

        inline Vector4& operator-=(const Vector4& v) {
            subtract(v);
            return *this;
        }

        inline const Vector4 operator-() const {
            Vector4 result(*this);
            result.negate();
            return result;
        }

        inline const Vector4 operator*(T s) const {
            Vector4 result(*this);
            result.scale(s);
            return result;
        }

        T operator *(const Vector4 &other) const {
            return x*other.x + y*other.y + z*other.z + w*other.w;
        }

        inline Vector4& operator*=(T s) {
            scale(s);
            return *this;
        }

        inline const Vector4 operator/(T s) const {
            return Vector4(this->x / double(s), this->y / double(s), this->z / double(s), this->w / double(s));
        }

        inline void operator /=(T s) {
            x /= double(s);
            y /= double(s);
            z /= double(s);
            w /= double(s);
        }

        inline bool operator < (const Vector4& rhs) const {
            if (x < rhs.x && y < rhs.y && z < rhs.z && w < rhs.w)
                return true;
            return false;
        }

        inline bool operator >(const Vector4& rhs) const {
            if (x > rhs.x && y > rhs.y && z > rhs.z && w > rhs.w)
                return true;
            return false;
        }

        inline bool operator==(const Vector4& v) const {
            return MATHEQUALS<T>(x, v.x)
                    && MATHEQUALS<T>(y, v.y)
                    && MATHEQUALS<T>(z, v.z)
                    && MATHEQUALS<T>(w, v.w);
        }

        inline bool operator!=(const Vector4& v) const {
            return !(*this == v);
        }

        inline operator float *() {
            return (float *)this;
        }

        inline operator const float *() const {
            return (const float *)this;
        }

    public:
        T x;
        T y;
        T z;
        T w;
    };

    typedef Vector4<float> Vector4f;
    static const Vector4f Vec4fZERO(0.0f, 0.0f, 0.0f, 0.0f);
    static const Vector4f Vec4fONE(1.0f, 1.0f, 1.0f, 1.0f);
    static const Vector4f Vec4fUNITX(1.0f, 0.0f, 0.0f, 0.0f);
    static const Vector4f Vec4fUNITY(0.0f, 1.0f, 0.0f, 0.0f);
    static const Vector4f Vec4fUNITZ(0.0f, 0.0f, 1.0f, 0.0f);
    static const Vector4f Vec4fUNITW(0.0f, 0.0f, 0.0f, 1.0f);
}

#endif // VECTOR4_H

