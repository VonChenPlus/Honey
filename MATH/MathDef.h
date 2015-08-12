#ifndef MATHDEF_H
#define MATHDEF_H

#include <limits>
#include <cstdlib>
#include <cmath>
#include <algorithm>

#include "BASE/Honey.h"
#undef min
#undef max

namespace MATH
{
    /**Util macro for conversion from degrees to radians.*/
    #define MATH_DEG_TO_RAD(x)          ((x) * 0.0174532925f)
    /**Util macro for conversion from radians to degrees.*/
    #define MATH_RAD_TO_DEG(x)          ((x)* 57.29577951f)

    #define MATH_PI             3.141592653589793f
    #define MATH_PIOVER2        1.57079632679489661923f
    #define MATH_1_PI           0.31830988618379067154
    #define MATH_E              2.71828182845904523536f
    #define MATH_TOLERANCE      2e-37f
    #define MATH_DEGREES_TO_RADIANS(__ANGLE__) ((__ANGLE__) * 0.01745329252f) // PI / 180
    #define MATH_RADIANS_TO_DEGREES(__ANGLE__) ((__ANGLE__) * 57.29577951f) // PI * 180

    #define MATH_RANDOM_MINUS1_1()  ((2.0f*((float)rand()/RAND_MAX))-1.0f)  // Returns a random float between -1 and 1.
    #define MATH_RANDOM_0_1()       ((float)rand()/RAND_MAX)                // Returns a random float between 0 and 1.

    inline int8 MATH_INT8_MAX()       { return std::numeric_limits<int8>::max(); }
    inline int8 MATH_INT8_MIN()       { return std::numeric_limits<int8>::min(); }
    inline int8 MATH_UINT8_MAX()      { return std::numeric_limits<uint8>::max(); }
    inline int8 MATH_UINT8_MIN()      { return std::numeric_limits<uint8>::min(); }

    inline int16 MATH_INT16_MAX()      { return std::numeric_limits<int16>::max(); }
    inline int16 MATH_INT16_MIN()      { return std::numeric_limits<int16>::min(); }
    inline int16 MATH_UINT16_MAX()     { return std::numeric_limits<uint16>::max(); }
    inline int16 MATH_UINT16_MIN()     { return std::numeric_limits<uint16>::min(); }

    inline int32 MATH_INT32_MAX()      { return std::numeric_limits<int32>::max(); }
    inline int32 MATH_INT32_MIN()      { return std::numeric_limits<int32>::min(); }
    inline int32 MATH_UINT32_MAX()     { return std::numeric_limits<uint32>::max(); }
    inline int32 MATH_UINT32_MIN()     { return std::numeric_limits<uint32>::min(); }

    inline int64 MATH_INT64_MAX()      { return std::numeric_limits<int64>::max(); }
    inline int64 MATH_INT64_MIN()      { return std::numeric_limits<int64>::min(); }
    inline int64 MATH_UINT64_MAX()     { return std::numeric_limits<uint64>::max(); }
    inline int64 MATH_UINT64_MIN()     { return std::numeric_limits<uint64>::min(); }

    inline float MATH_FLOAT_MAX()      { return std::numeric_limits<float>::max(); }
    inline float MATH_FLOAT_MIN()      { return std::numeric_limits<float>::min(); }
    inline float MATH_FLOAT_EPSILON()  { return std::numeric_limits<float>::epsilon(); }

    inline double MATH_DOUBLE_MAX()      { return std::numeric_limits<double>::max(); }
    inline double MATH_DOUBLE_MIN()      { return std::numeric_limits<double>::min(); }
    inline double MATH_DOUBLE_EPSILON()  { return std::numeric_limits<double>::epsilon(); }

    #define MATH_CLAMP(x, lo, hi) ((x < lo) ? lo : ((x > hi) ? hi : x))
    #define MATH_CLAMP16(x) MATH_CLAMP(x, 0, 16)
    #define MATH_CLAMP32(x) MATH_CLAMP(x, 0, 32)
    #define MATH_CLAMP64(x) MATH_CLAMP(x, 0, 64)

    template <typename T>
    inline T MATHZERO() { return 0;}
    template <>
    inline float MATHZERO() { return 0.0f; }
    template <>
    inline double MATHZERO() { return 0.0; }

    template <typename T>
    inline T MATHONE() { return 1;}
    template <>
    inline float MATHONE() { return 1.0f; }
    template <>
    inline double MATHONE() { return 1.0; }

    template <typename T>
    inline bool MATHEQUALS(const T &v1, const T &v2) { return v1 == v2; }
    template <>
    inline bool MATHEQUALS(const float &v1, const float &v2) { return std::abs(v1 - v2) < std::numeric_limits<float>::epsilon(); }
    template <>
    inline bool MATHEQUALS(const double &v1, const double &v2) { return std::abs(v1 - v2) < std::numeric_limits<double>::epsilon(); }

    template <typename T>
    inline T MATH_MAX(T v1, T v2) { return std::max(v1, v2); }
    template <typename T>
    inline T MATH_MIN(T v1, T v2) { return std::min(v1, v2); }

    inline bool IsPowerOf2(int n) { return n == 1 || (n & (n - 1)) == 0; }

    inline uint32 Log2I(uint32 value) { uint32 ret = -1; while (value != 0) { value >>= 1; ret++; } return ret; }
}

#endif // MATHDEF_H

