#ifndef NTHREADDEF_H
#define NTHREADDEF_H

#include <thread>

#ifdef _WIN32
#include <process.h>
#define USE_BEGINTHREADEX
#ifdef USE_BEGINTHREADEX
#define THREAD_ID unsigned
#define THREAD_RETURN unsigned __stdcall
#else
#define THREAD_ID DWORD
#define THREAD_RETURN DWORD WINAPI
#endif
#define THREAD_HANDLE HANDLE

#else

#define THREAD_ID pthread_t
#define THREAD_HANDLE pthread_t
#define THREAD_RETURN void*

#endif

#endif // NTHREADDEF_H

