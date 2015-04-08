#ifndef MUTEX_H
#define MUTEX_H

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <limits.h>
#include <intrin.h>

// Zap stupid windows defines
// Should move these somewhere clever.
#undef p
#undef DrawText
#undef itoa

#else
#include <pthread.h>
#include <errno.h>
#include <sys/time.h>
#endif

#include "BASE/BasicTypes.h"

class recursive_mutex
{
#ifdef _WIN32
    typedef CRITICAL_SECTION mutexType;
#else
    typedef pthread_mutex_t mutexType;
#endif
public:
    recursive_mutex() {
#ifdef _WIN32
        InitializeCriticalSection(&mut_);
#else
        // Critical sections are recursive so let's make these recursive too.
        pthread_mutexattr_t attr;
        pthread_mutexattr_init(&attr);
        pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
        pthread_mutex_init(&mut_, &attr);
#endif
    }
    ~recursive_mutex() {
#ifdef _WIN32
        DeleteCriticalSection(&mut_);
#else
        pthread_mutex_destroy(&mut_);
#endif
    }

    bool trylock() {
#ifdef _WIN32
        return TryEnterCriticalSection(&mut_) != FALSE;
#else
        return pthread_mutex_trylock(&mut_) != EBUSY;
#endif
    }

    void lock() {
#ifdef _WIN32
        EnterCriticalSection(&mut_);
#else
        pthread_mutex_lock(&mut_);
#endif
    }

    void unlock() {
#ifdef _WIN32
        LeaveCriticalSection(&mut_);
#else
        pthread_mutex_unlock(&mut_);
#endif
    }

    mutexType &native_handle() {
        return mut_;
    }

private:
    mutexType mut_;
    recursive_mutex(const recursive_mutex &other);
};

class lock_guard {
public:
    lock_guard(recursive_mutex &mtx) : mtx_(mtx) {mtx_.lock();}
    ~lock_guard() {mtx_.unlock();}

private:
    recursive_mutex &mtx_;
};

#endif // MUTEX_H

