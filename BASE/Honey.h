#ifndef HONEY_H
#define HONEY_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>  // for byte swapping

#include "BASE/HException.h"

#ifdef _WIN32
// We need this to compile without hundreds of std::bind errors in Visual Studio 2012
// since by default VARIADIC_MAX is something low like 3, 4, or 5.
// It's a good idea to include this file wherever std::bind is being used with more than 3, 4 or 5 args.
// Visual Studio 2013 doesn't need this after all, so we can simply check for equality.
#if _MSC_VER == 1700
 #undef _VARIADIC_MAX
 #define _VARIADIC_MAX 10
 #endif

#endif

#define GCC_VER(x,y,z)	((x) * 10000 + (y) * 100 + (z))
#define GCC_VERSION GCC_VER(__GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__)

#ifdef _WIN32
#pragma warning(disable:4244)
#pragma warning(disable:4996)
#pragma warning(disable:4305)  // truncation from double to float
#endif

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;
typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;
typedef intptr_t IntPtr;
typedef uintptr_t UIntPtr;
typedef uint32 Color;
typedef unsigned char HBYTE;
typedef int64_t ssize_t;

#ifdef _WIN32

#include <tchar.h>

#define ALIGNED16(x) __declspec(align(16)) x
#define ALIGNED32(x) __declspec(align(32)) x
#define ALIGNED64(x) __declspec(align(64)) x
#define ALIGNED16_DECL(x) __declspec(align(16)) x
#define ALIGNED64_DECL(x) __declspec(align(64)) x

#else

#define ALIGNED16(x)  __attribute__((aligned(16))) x
#define ALIGNED32(x)  __attribute__((aligned(32))) x
#define ALIGNED64(x)  __attribute__((aligned(64))) x
#define ALIGNED16_DECL(x) __attribute__((aligned(16))) x
#define ALIGNED64_DECL(x) __attribute__((aligned(64))) x

#endif  // _WIN32

// Byteswapping
#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))
#endif

inline uint8 swap8(uint8 _data) {return _data;}

// Just in case this has been defined by platform
#undef swap16
#undef swap32
#undef swap64

#ifdef _WIN32
inline uint16 swap16(uint16 _data) {return _byteswap_ushort(_data);}
inline uint32 swap32(uint32 _data) {return _byteswap_ulong (_data);}
inline uint64 swap64(uint64 _data) {return _byteswap_uint64(_data);}
#elif defined(ARM)
inline uint16 swap16 (uint16 _data) { uint32 data = _data; __asm__ ("rev16 %0, %1\n" : "=l" (data) : "l" (data)); return (uint16)data;} 
inline uint32 swap32 (uint32 _data) {__asm__ ("rev %0, %1\n" : "=l" (_data) : "l" (_data)); return _data;} 
inline uint64 swap64(uint64 _data) {return ((uint64)swap32(_data) << 32) | swap32(_data >> 32);}
#elif __linux__ && !defined(ANDROID)
#include <byteswap.h>
inline uint16 swap16(uint16 _data) {return bswap_16(_data);}
inline uint32 swap32(uint32 _data) {return bswap_32(_data);}
inline uint64 swap64(uint64 _data) {return bswap_64(_data);}
#elif defined(__FreeBSD__)
#include <sys/endian.h>
inline uint16 swap16(uint16 _data) {return bswap16(_data);}
inline uint32 swap32(uint32 _data) {return bswap32(_data);}
inline uint64 swap64(uint64 _data) {return bswap64(_data);}
#elif defined(__GNUC__)
inline uint16 swap16(uint16 _data) {return (_data >> 8) | (_data << 8);}
inline uint32 swap32(uint32 _data) {return __builtin_bswap32(_data);}
inline uint64 swap64(uint64 _data) {return __builtin_bswap64(_data);}
#else
// Slow generic implementation. Hopefully this never hits
inline uint16 swap16(uint16 data) {return (data >> 8) | (data << 8);}
inline uint32 swap32(uint32 data) {return (swap16(data) << 16) | swap16(data >> 16);}
inline uint64 swap64(uint64 data) {return ((uint64)swap32(data) << 32) | swap32(data >> 32);}
#endif

inline uint16 swap16(const uint8* _pData) {return swap16(*(const uint16*)_pData);}
inline uint32 swap32(const uint8* _pData) {return swap32(*(const uint32*)_pData);}
inline uint64 swap64(const uint8* _pData) {return swap64(*(const uint64*)_pData);}


template <typename T>
T swap(T *value) {
    switch (sizeof(T)) {
    case 2: return (T)swap16((uint8 *)value);
    case 4: return (T)swap32((uint8 *)value);
    case 8: return (T)swap64((uint8 *)value);
    default:
        throw _HException_Normal("Unhander data bits!");
    }
}

// Thread local storage
#ifdef _WIN32
#define __THREAD __declspec( thread ) 
#else
#define __THREAD __thread
#endif

// For really basic windows code compat
#ifndef _TCHAR_DEFINED
typedef char TCHAR;
#endif

#ifndef NULL
#ifdef __cplusplus
#define NULL    0
#else
#define NULL    ((void *)0)
#endif /* __cplusplus */
#endif /* NULL */

#ifdef _MSC_VER
    #if _MSC_VER > 1600
        #define NULLPTR nullptr
    #else
        #define NULLPTR NULL
    #endif
#else
    #define NULLPTR NULL
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

// Implement C99 functions and similar that are missing in MSVC.
#if defined(_MSC_VER) && _MSC_VER < 1900
#include <stdio.h>
#include <stdarg.h>
static int c99_vsnprintf(char* str, size_t size, const char* format, va_list ap) {
    int count = -1;
    if (size != 0)
        count = _vsnprintf_s(str, size, _TRUNCATE, format, ap);
    if (count == -1)
        count = _vscprintf(format, ap);
    return count;
}
inline int c99_snprintf(char* str, size_t size, const char* format, ...) {
    int count;
    va_list ap;

    va_start(ap, format);
    count = c99_vsnprintf(str, size, format, ap);
    va_end(ap);

    return count;
}
#define snprintf c99_snprintf
#define vscprintf _vscprintf

#endif

#endif // HONEY_H
