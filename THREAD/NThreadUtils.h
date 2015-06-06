#ifndef NTHREADUTILS
#define NTHREADUTILS

#ifdef _WIN32
#include <windows.h>
#endif

#include "THREAD/NThreadDef.h"
#include "UTILS/TIME/NTime.h"

namespace THREAD
{
    inline void yield() {
    #ifdef _WIN32
        SwitchToThread();
    #else
        UTILS::TIME::SleepThread(0);
    #endif
    }

    inline THREAD_ID CurrentThreadID() {
    #ifdef _WIN32
        return GetCurrentThreadId();
    #else
        return pthread_self();
    #endif
    }

    inline void SetCurrentThreadName(const char *threadName) {
    #ifdef _WIN32
        // Set the debugger-visible threadname through an unholy magic hack
        static const DWORD MS_VC_EXCEPTION = 0x406D1388;
        #pragma pack(push,8)
        struct THREADNAME_INFO {
            DWORD dwType; // must be 0x1000
            LPCSTR szName; // pointer to name (in user addr space)
            DWORD dwThreadID; // thread ID (-1=caller thread)
            DWORD dwFlags; // reserved for future use, must be zero
        } info;
        #pragma pack(pop)

        info.dwType = 0x1000;
        info.szName = threadName;
        info.dwThreadID = -1; //dwThreadID;
        info.dwFlags = 0;

        __try
        {
            RaiseException(MS_VC_EXCEPTION, 0, sizeof(info)/sizeof(ULONG_PTR), (ULONG_PTR*)&info);
        }
        __except(EXCEPTION_CONTINUE_EXECUTION)
        {}
    #else
        // Do nothing
    #endif
    }
}

#endif // NTHREADUTILS

