#ifndef VECTOR2_H
#define VECTOR2_H

#include "MATH/MathDef.h"

namespace MATH
{
    /**
     * Defines a 2-element floating point vector.
     */
    template <typename T>
    class Vector2
    {
    public:
        Vector2()
            : x(MATHZERO<T>())
            , y(MATHZERO<T>()) {
        }

        Vector2(const T &_x, const T&_y)
            : x(_x)
            , y(_y) {
        }

        Vector2(const Vector2 &copy) {
            set(copy);
        }

        inline bool isZero() const {
            return x == MATHZERO<T>()
                    && y == MATHZERO<T>();
        }

        inline bool isOne() const {
            return (MATHEQUALS<T>(x, MATHONE<T>()))
                    && (MATHEQUALS<T>(y, MATHONE<T>()));
        }

        inline void add(const Vector2& v) {
            x += v.x;
            y += v.y;
        }

        static Vector2 add(const Vector2& v1, const Vector2& v2) {
            Vector2 result = v1;
            result.add(v2);
            return result;
        }

        void clamp(const Vector2& min, const Vector2& max) {
            // Clamp the x value.
            if (x < min.x)
                x = min.x;
            if (x > max.x)
                x = max.x;

            // Clamp the y value.
            if (y < min.y)
                y = min.y;
            if (y > max.y)
                y = max.y;
        }

        T distance(const Vector2& v) const {
            T dx = v.x - x;
            T dy = v.y - y;
            return (T)sqrt(dx * dx + dy * dy);
        }

        T distanceSquared(const Vector2& v) const {
            T dx = v.x - x;
            T dy = v.y - y;
            return (dx * dx + dy * dy);
        }

        inline T dot(const Vector2& v) const {
            return (x * v.x + y * v.y);
        }

        T length() const {
            return (T)sqrt(x * x + y * y);
        }

        T lengthSquared() const {
            return (x * x + y * y);
        }

        inline void negate() {
            x = -x;
            y = -y;
        }

        void normalize() {
            T n = x * x + y * y;
            if (MATHEQUALS<T>(n, MATHONE<T>()))
                return;

            n = (T)sqrt(n);
            // Too close to zero.
            if (n < std::numeric_limits<T>::epsilon())
                return;

            n = 1.0f / n;
            x *= n;
            y *= n;
        }

        Vector2 getNormalized() const {
            Vector2 v(*this);
            v.normalize();
            return v;
        }

        inline void scale(const T &scalar) {
            x *= scalar;
            y *= scalar;
        }

        inline void scale(const Vector2& scale) {
            x *= scale.x;
            y *= scale.y;
        }

        /**
         * Rotates this vector by angle (specified in radians) around the given point.
         *
         * @param point The point to rotate around.
         * @param angle The angle to rotate by (in radians).
         */
        Vector2 rotate(const Vector2& pivot, double angle) {
            return pivot + (*this - pivot).rotate(Vector2::forAngle(angle));
        }

        /** Complex multiplication of two points ("rotates" two points).
         @return Vec2 vector with an angle of this.getAngle() + other.getAngle(),
         and a length of this.getLength() * other.getLength().
         @since v2.1.4
         * @js NA
         * @lua NA
         */
        inline Vector2 rotate(const Vector2& other) const {
            return Vector2(x*other.x - y*other.y, x*other.y + y*other.x);
        }

        /** Unrotates two points.
         @return Vec2 vector with an angle of this.getAngle() - other.getAngle(),
         and a length of this.getLength() * other.getLength().
         @since v2.1.4
         * @js NA
         * @lua NA
         */
        inline Vector2 unrotate(const Vector2& other) const {
            return Vector2(x*other.x + y*other.y, y*other.x - x*other.y);
        }

        inline void set(const T &_x, const T &_y) {
            this->x = _x;
            this->y = _y;
        }

        void set(const T* array) {
            x = array[0];
            y = array[1];
        }

        inline void set(const Vector2& v) {
            this->x = v.x;
            this->y = v.y;
        }

        inline void set(const Vector2& p1, const Vector2& p2) {
            x = p2.x - p1.x;
            y = p2.y - p1.y;
        }

        inline void setZero() {
            x = y = MATHZERO<T>();
        }

        inline void subtract(const Vector2& v) {
            x -= v.x;
            y -= v.y;
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
        inline void smooth(const Vector2& target, const T &elapsedTime, const T &responseTime) {
            if (elapsedTime > 0) {
                *this += (target - *this) * (elapsedTime / (double)(elapsedTime + responseTime));
            }
        }

        inline const Vector2 operator+(const Vector2& v) const {
            Vector2 result(*this);
            result.add(v);
            return result;
        }

        inline Vector2& operator+=(const Vector2& v) {
            add(v);
            return *this;
        }

        inline const Vector2 operator-(const Vector2& v) const {
            Vector2 result(*this);
            result.subtract(v);
            return result;
        }

        inline Vector2& operator-=(const Vector2& v) {
            subtract(v);
            return *this;
        }

        inline const Vector2 operator-() const {
            Vector2 result(*this);
            result.negate();
            return result;
        }

        inline const Vector2 operator*(T s) const {
            Vector2 result(*this);
            result.scale(s);
            return result;
        }

        inline Vector2& operator*=(T s) {
            scale(s);
            return *this;
        }

        inline const Vector2 operator/(T s) const {
            return Vector2(this->x / double(s), this->y / double(s));
        }

        inline bool operator<(const Vector2& v) const {
            if (x < v.x && y < v.y)
                return true;
            return false;
        }

        inline bool operator>(const Vector2& v) const {
            if (x > v.x && y > v.y)
                return true;
            return false;
        }

        inline bool operator==(const Vector2& v) const {
            return MATHEQUALS<T>(x, v.x)
                    && MATHEQUALS<T>(y, v.y);
        }

        inline bool equals(const Vector2 &p) const {
            return *this == p;
        }

        inline bool operator!=(const Vector2& v) const {
            return !(*this == v);
        }

        bool fuzzyEquals(const Vector2& b, T var) const {
            if(x - var <= b.x && b.x <= x + var)
                if(y - var <= b.y && b.y <= y + var)
                    return true;
            return false;
        }

        inline T cross(const Vector2& other) const {
            return x*other.y - y*other.x;
        }

        inline Vector2 midPoint(const Vector2& other) const {
            return Vector2((x + other.x) / 2.0f, (y + other.y) / 2.0f);
        }

        /** Calculates the projection of this over other.
         @return Vec2
         @since v2.1.4
         * @js NA
         * @lua NA
         */
        inline Vector2 project(const Vector2& other) const {
            return other * (dot(other)/other.dot(other));
        }

        /** Linear Interpolation between two points a and b
         @returns
            alpha == 0 ? a
            alpha == 1 ? b
            otherwise a value between a..b
         @since v2.1.4
         * @js NA
         * @lua NA
         */
        inline Vector2 lerp(const Vector2& other, double alpha) const {
            return *this * (1.0 - alpha) + other * alpha;
        }

        static inline Vector2 forAngle(const double &a) {
            return Vector2((T)cos(a), (T)sin(a));
        }

    public:
        T x;
        T y;
    };

    typedef Vector2<float> Vector2f;
    static const Vector2f Vec2fZERO(0.0f, 0.0f);
    static const Vector2f Vec2fONE(1.0f, 1.0f);
    static const Vector2f Vec2fUNITX(1.0f, 0.0f);
    static const Vector2f Vec2fUNITY(0.0f, 1.0f);
}

#endif // VECTOR2_H

