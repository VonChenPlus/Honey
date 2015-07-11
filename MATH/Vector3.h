#ifndef VECTOR3_H
#define VECTOR3_H

#include "MATH/MathDef.h"

namespace MATH
{
    /**
     * Defines a 3-element floating point vector.
     */
    template <typename T>
    class Vector3
    {
    public:
        Vector3()
            : x(MATHZERO<T>())
            , y(MATHZERO<T>())
            , z(MATHZERO<T>()){
        }

        Vector3(const T &_x, const T&_y, const T&_z)
            : x(_x)
            , y(_y)
            , z(_z) {
        }

        Vector3(const Vector3 &copy) {
            set(copy);
        }

        inline bool isZero() const {
            return MATHEQUALS(x, MATHZERO<T>())
                    && MATHEQUALS(y, MATHZERO<T>())
                    && MATHEQUALS(z, MATHZERO<T>());
        }

        inline bool isOne() const {
            return (MATHEQUALS<T>(x, MATHONE<T>()))
                    && (MATHEQUALS<T>(y, MATHONE<T>()))
                    && (MATHEQUALS<T>(z, MATHONE<T>()));
        }

        inline void add(const Vector3& v) {
            x += v.x;
            y += v.y;
            y += v.z;
        }

        inline void add(const T& _x, const T& _y, const T& _z) {
            x += _x;
            y += _y;
            z += _z;
        }

        void clamp(const Vector3& min, const Vector3& max) {
            // Clamp the x value.
            x = MATH_CLAMP((x), (min.x), (max.x));

            // Clamp the y value.
            y = MATH_CLAMP((y), (min.y), (max.y));

            // Clamp the z value.
            z = MATH_CLAMP((z), (min.z), (max.z));
        }

        void cross(const Vector3& v) {
            T x = (this->y * v.z) - (this->z * v.y);
            T y = (this->z * v.x) - (this->x * v.z);
            T z = (this->x * v.y) - (this->y * v.x);

            this->x = x;
            this->y = y;
            this->z = z;
            // TODO
            // USE_NEON32
            // USE_NEON64
        }

        T distance(const Vector3& v) const {
            T dx = v.x - x;
            T dy = v.y - y;
            T dz = v.z - z;
            return (T)sqrt(dx * dx + dy * dy + dz * dz);
        }

        T distanceSquared(const Vector3& v) const {
            T dx = v.x - x;
            T dy = v.y - y;
            T dz = v.z - z;
            return (dx * dx + dy * dy + dz * dz);
        }

        inline T dot(const Vector3& v) const {
            return (x * v.x + y * v.y + z * v.z);
        }

        T length() const {
            return (T)sqrt(x * x + y * y + z * z);
        }

        T lengthSquared() const {
            return (x * x + y * y + z * z);
        }

        inline void negate() {
            x = -x;
            y = -y;
            z = -z;
        }

        void normalize() {
            T n = x * x + y * y + z * z;
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
        }

        Vector3 getNormalized() const {
            Vector3 v(*this);
            v.normalize();
            return v;
        }

        inline void scale(const T &scalar) {
            x *= scalar;
            y *= scalar;
            z *= scalar;
        }

        inline void set(const T &_x, const T &_y, const T &_z) {
            this->x = _x;
            this->y = _y;
            this->z = _z;
        }

        void set(const T* array) {
            x = array[0];
            y = array[1];
            z = array[2];
        }

        inline void set(const Vector3& v) {
            this->x = v.x;
            this->y = v.y;
            this->z = v.z;
        }

        inline void set(const Vector3& p1, const Vector3& p2) {
            x = p2.x - p1.x;
            y = p2.y - p1.y;
            z = p2.z - p1.z;
        }

        inline void setZero() {
            x = y = z = MATHZERO<T>();
        }

        inline void subtract(const Vector3& v) {
            x -= v.x;
            y -= v.y;
            z -= v.z;
        }

        /**
         * Updates this vector towards the given target using a smoothing function.
         * The given response time determines the amount of smoothing (lag). A longer
         * response time yields a smoother result and more lag. To force this vector to
         * follow the target closely, provide a response time that is very small relative
         * to the given elapsed time.
         *
         * @param target target value.
         * @param elapsedTime elapsed time between calls.
         * @param responseTime response time (in the same units as elapsedTime).
         */
        inline void smooth(const Vector3& target, const T &elapsedTime, const T &responseTime) {
            if (elapsedTime > 0) {
                *this += (target - *this) * (elapsedTime / (double)(elapsedTime + responseTime));
            }
        }

        /**
         * Linear interpolation between two vectors A and B by alpha which
         * is in the range [0,1]
         */
        inline Vector3 lerp(const Vector3& other, double alpha) const {
            return *this * (1.0 - alpha) + other * alpha;
        }

        inline const Vector3 operator+(const Vector3& v) const {
            Vector3 result(*this);
            result.add(v);
            return result;
        }

        inline Vector3& operator+=(const Vector3& v) {
            add(v);
            return *this;
        }

        inline const Vector3 operator-(const Vector3& v) const {
            Vector3 result(*this);
            result.subtract(v);
            return result;
        }

        inline Vector3& operator-=(const Vector3& v) {
            subtract(v);
            return *this;
        }

        inline const Vector3 operator-() const {
            Vector3 result(*this);
            result.negate();
            return result;
        }

        inline const Vector3 operator*(T s) const {
            Vector3 result(*this);
            result.scale(s);
            return result;
        }

        T operator *(const Vector3 &other) const {
            return x*other.x + y*other.y + z*other.z;
        }

        inline Vector3& operator*=(T s) {
            scale(s);
            return *this;
        }

        inline const Vector3 operator/(T s) const {
            return Vector3(this->x / double(s), this->y / double(s), this->z / double(s));
        }

        inline void operator /=(T s) {
            x /= double(s);
            y /= double(s);
            z /= double(s);
        }

        inline bool operator < (const Vector3& rhs) const {
            if (x < rhs.x && y < rhs.y && z < rhs.z)
                return true;
            return false;
        }

        inline bool operator >(const Vector3& rhs) const {
            if (x > rhs.x && y > rhs.y && z > rhs.z)
                return true;
            return false;
        }

        inline bool operator==(const Vector3& v) const {
            return MATHEQUALS<T>(x, v.x)
                    && MATHEQUALS<T>(y, v.y)
                    && MATHEQUALS<T>(z, v.z);
        }

        inline bool operator!=(const Vector3& v) const {
            return !(*this == v);
        }

        Vector3 operator %(const Vector3 &v) const {
            return Vector3(y*v.z-z*v.y, z*v.x-x*v.z, x*v.y-y*v.x);
        }

    public:
        T x;
        T y;
        T z;
    };

    typedef Vector3<float> Vector3f;
    static const Vector3f Vec3fZERO(0.0f, 0.0f, 0.0f);
    static const Vector3f Vec3fONE(1.0f, 1.0f, 1.0f);
    static const Vector3f Vec3fUNITX(1.0f, 0.0f, 0.0f);
    static const Vector3f Vec3fUNITY(0.0f, 1.0f, 0.0f);
    static const Vector3f Vec3fUNITZ(0.0f, 0.0f, 1.0f);
}

#endif // VECTOR3_H

