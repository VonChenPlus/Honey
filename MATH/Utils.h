#ifndef UTILS_H
#define UTILS_H

#include <cmath>

namespace MATH
{
    #define PI 3.141592653589793f
    #ifndef M_PI
    #define M_PI 3.141592653589793f
    #endif

    inline bool IsPowerOf2(int n) {
        return n == 1 || (n & (n - 1)) == 0;
    }

    inline int Clamp16(int x) {
        if (x < 0) return 0; if (x > 15) return 15; return x;
    }
    inline int Clamp32(int x) {
        if (x < 0) return 0; if (x > 31) return 31; return x;
    }
    inline int Clamp64(int x) {
        if (x < 0) return 0; if (x > 63) return 63; return x;
    }
}

#endif // UTILS_H

