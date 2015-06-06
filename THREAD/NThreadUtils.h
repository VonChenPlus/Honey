#ifndef NTHREADUTILS
#define NTHREADUTILS

#include "THREAD/NThreadDef.h"
#include "UTILS/TIME/NTime.h"

namespace THREAD
{
    inline void yield()
    {
    #ifdef _WIN32
        SwitchToThread();
    #else
        UTILS::TIME::SleepThread(0);
    #endif
    }

    inline THREAD_ID CurrentThreadID()
    {
    #ifdef _WIN32
        return GetCurrentThreadId();
    #else
        return pthread_self();
    #endif
    }
}

#endif // NTHREADUTILS

