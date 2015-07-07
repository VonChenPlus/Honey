#ifndef MATHDEF_H
#define MATHDEF_H

#include <climits>
#include <cstdlib>

#include "BASE/Honey.h"

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

    #define MATH_RANDOM_MINUS1_1()  ((2.0f*((float)rand()/RAND_MAX))-1.0f)  // Returns a random float between -1 and 1.
    #define MATH_RANDOM_0_1()       ((float)rand()/RAND_MAX)                // Returns a random float between 0 and 1.

    #define MATH_FLOAT_MAX      std::numeric_limits<float>::max()
    #define MATH_FLOAT_MIN      std::numeric_limits<float>::min()

    #define MATH_DOUBLE_MAX     std::numeric_limits<double>::max()
    #define MATH_DOUBLE_MIN     std::numeric_limits<double>::min()

    #define MATH_INT8_MAX       std::numeric_limits<int8>::max()
    #define MATH_INT8_MIN       std::numeric_limits<int8>::min()
    #define MATH_UINT8_MAX      std::numeric_limits<uint8>::max()
    #define MATH_UINT8_MIN      std::numeric_limits<uint8>::min()

    #define MATH_INT16_MAX      std::numeric_limits<int16>::max()
    #define MATH_INT16_MIN      std::numeric_limits<int16>::min()
    #define MATH_UINT16_MAX     std::numeric_limits<uint16>::max()
    #define MATH_UINT16_MIN     std::numeric_limits<uint16>::min()

    #define MATH_INT32_MAX      std::numeric_limits<int32>::max()
    #define MATH_INT32_MIN      std::numeric_limits<int32>::min()
    #define MATH_UINT32_MAX     std::numeric_limits<uint32>::max()
    #define MATH_UINT32_MIN     std::numeric_limits<uint32>::min()

    #define MATH_INT64_MAX      std::numeric_limits<int64>::max()
    #define MATH_INT64_MIN      std::numeric_limits<int64>::min()
    #define MATH_UINT64_MAX     std::numeric_limits<uint64>::max()
    #define MATH_UINT64_MIN     std::numeric_limits<uint64>::min()

    #define MATH_FLOAT_MAX      std::numeric_limits<float>::max()
    #define MATH_FLOAT_MIN      std::numeric_limits<float>::min()
    #define MATH_FLOAT_EPSILON  std::numeric_limits<float>::epsilon()

    #define MATH_DOUBLE_MAX      std::numeric_limits<double>::max()
    #define MATH_DOUBLE_MIN      std::numeric_limits<double>::min()
    #define MATH_DOUBLE_EPSILON  std::numeric_limits<double>::epsilon()

    #define MATH_CLAMP(x, lo, hi) ((x < lo) ? lo : ((x > hi) ? hi : x))
    #define MATH_CLAMP16(x) MATH_CLAMP(x, 0, 16)
    #define MATH_CLAMP32(x) MATH_CLAMP(x, 0, 32)
    #define MATH_CLAMP64(x) MATH_CLAMP(x, 0, 64)

    inline bool IsPowerOf2(int n) {
        return n == 1 || (n & (n - 1)) == 0;
    }

    inline uint32 Log2I(uint32 value) {
        uint32 ret = -1;
        while (value != 0) {
            value >>= 1; ret++;
        }
        return ret;
    }
}

#endif // MATHDEF_H

