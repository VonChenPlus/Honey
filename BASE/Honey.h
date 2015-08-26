#ifndef HONEY_H
#define HONEY_H

#include "BASE/Platform.h"

#define GCC_VER(x,y,z)	((x) * 10000 + (y) * 100 + (z))
#define GCC_VERSION GCC_VER(__GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__)

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))
#endif

#ifndef UNUSED
    #define UNUSED(x) (void)x
#endif

#define DISALLOW_COPY_AND_ASSIGN(t) \
private: \
    t(const t &other);  \
    void operator =(const t &other);

#define SAFE_DELETE(p)           do { delete (p); (p) = nullptr; } while(0)
#define SAFE_DELETE_ARRAY(p)     do { if(p) { delete[] (p); (p) = nullptr; } } while(0)
#define SAFE_FREE(p)             do { if(p) { free(p); (p) = nullptr; } } while(0)

#endif // HONEY_H
