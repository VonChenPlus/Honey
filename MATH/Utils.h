#ifndef UTILS_H
#define UTILS_H

#include <cmath>

namespace MATH
{
    #define PI 3.141592653589793f
    #ifndef M_PI
    #define M_PI 3.141592653589793f
    #endif

    inline bool IsPowerOf2(int n)
    {
        return n == 1 || (n & (n - 1)) == 0;
    }
}

#endif // UTILS_H

