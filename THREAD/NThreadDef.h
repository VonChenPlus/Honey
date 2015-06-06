#ifndef NTHREADDEF_H
#define NTHREADDEF_H

#include <BASE/Native.h>

#if GCC_VERSION >= GCC_VER(4,4,0) && __GXX_EXPERIMENTAL_CXX0X__ && !defined(ANDROID) && !defined(__SYMBIAN32__)
// GCC 4.4 provides <thread>
#ifndef _GLIBCXX_USE_SCHED_YIELD
#define _GLIBCXX_USE_SCHED_YIELD
#endif
#include <thread>
#define THREAD_ID pthread_t
#define THREAD_HANDLE pthread_t
#define THREAD_RETURN void*
#else

#if (_MSC_VER >= 1600) || (GCC_VERSION >= GCC_VER(4,3,0) && __GXX_EXPERIMENTAL_CXX0X__)
#define USE_RVALUE_REFERENCES
#endif

#if defined(_WIN32)
// WIN32

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#if defined(_MSC_VER) && defined(_MT)
// When linking with LIBCMT (the multithreaded C library), Microsoft recommends
// using _beginthreadex instead of CreateThread.
#define USE_BEGINTHREADEX
#include <process.h>
#endif

#ifdef USE_BEGINTHREADEX
#define THREAD_ID unsigned
#define THREAD_RETURN unsigned __stdcall
#else
#define THREAD_ID DWORD
#define THREAD_RETURN DWORD WINAPI
#endif
#define THREAD_HANDLE HANDLE

#else
// PTHREAD

#include <unistd.h>

#ifndef _POSIX_THREADS
#error unsupported platform (no pthreads?)
#endif

#include <pthread.h>

#define THREAD_ID pthread_t
#define THREAD_HANDLE pthread_t
#define THREAD_RETURN void*

#endif

#endif

#endif // NTHREADDEF_H

