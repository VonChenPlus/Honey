#ifndef PLATFORM_H
#define PLATFORM_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>  // for byte swapping

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

#ifdef _WIN32
#pragma warning(disable:4244)
#pragma warning(disable:4996)
#pragma warning(disable:4305)  // truncation from double to float
#endif

// Thread local storage
#ifdef _WIN32
#define __THREAD __declspec( thread )
#else
#define __THREAD __thread
#endif

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

inline uint8_t swap8(uint8_t _data) {return _data;}

// Just in case this has been defined by platform
#undef swap16
#undef swap32
#undef swap64

#ifdef _WIN32
inline uint16_t swap16(uint16_t _data) {return _byteswap_ushort(_data);}
inline uint32_t swap32(uint32_t _data) {return _byteswap_ulong (_data);}
inline uint64_t swap64(uint64_t _data) {return _byteswap_uint64(_data);}
#elif defined(ARM)
inline uint16_t swap16 (uint16_t _data) { uint32_t data = _data; __asm__ ("rev16 %0, %1\n" : "=l" (data) : "l" (data)); return (uint16_t)data;}
inline uint32_t swap32 (uint32_t _data) {__asm__ ("rev %0, %1\n" : "=l" (_data) : "l" (_data)); return _data;}
inline uint64_t swap64(uint64_t _data) {return ((uint64_t)swap32(_data) << 32) | swap32(_data >> 32);}
#elif __linux__ && !defined(ANDROID)
#include <byteswap.h>
inline uint16_t swap16(uint16_t _data) {return bswap_16(_data);}
inline uint32_t swap32(uint32_t _data) {return bswap_32(_data);}
inline uint64_t swap64(uint64_t _data) {return bswap_64(_data);}
#elif defined(__FreeBSD__)
#include <sys/endian.h>
inline uint16_t swap16(uint16_t _data) {return bswap16(_data);}
inline uint32_t swap32(uint32_t _data) {return bswap32(_data);}
inline uint64_t swap64(uint64_t _data) {return bswap64(_data);}
#elif defined(__GNUC__)
inline uint16_t swap16(uint16_t _data) {return (_data >> 8) | (_data << 8);}
inline uint32_t swap32(uint32_t _data) {return __builtin_bswap32(_data);}
inline uint64_t swap64(uint64_t _data) {return __builtin_bswap64(_data);}
#else
// Slow generic implementation. Hopefully this never hits
inline uint16_t swap16(uint16_t data) {return (data >> 8) | (data << 8);}
inline uint32_t swap32(uint32_t data) {return (swap16(data) << 16) | swap16(data >> 16);}
inline uint64_t swap64(uint64_t data) {return ((uint64_t)swap32(data) << 32) | swap32(data >> 32);}
#endif

inline uint16_t swap16(const uint8_t* _pData) {return swap16(*(const uint16_t*)_pData);}
inline uint32_t swap32(const uint8_t* _pData) {return swap32(*(const uint32_t*)_pData);}
inline uint64_t swap64(const uint8_t* _pData) {return swap64(*(const uint64_t*)_pData);}

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

#ifdef _MSC_VER
    #pragma warning (disable:4996)
    #ifndef strncasecmp
        #define strncasecmp _strnicmp
    #endif

    #ifndef strcasecmp
        #define strcasecmp _strcmpi
    #endif
#endif

#endif // PLATFORM_H

