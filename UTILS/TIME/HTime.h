#ifndef TIMEUTIL_H
#define TIMEUTIL_H

#ifdef _WIN32
#include <WinSock2.h>
#else
#include <sys/time.h>
#include <unistd.h>
#endif

namespace UTILS
{
    namespace TIME
    {
        // http://linux.die.net/man/3/clock_gettime

        // Uncached time. Slower than the above cached time functions. Does not update cached time, call TimeUpdate for that.
        double FetchCurrentTime();

        // This time implementation caches the time for max performance (call TimeNow() as much as you like).
        // You need to call TimeUpdate() once per frame (or whenever you need the correct time right now).

        void TimeUpdate();

        // Seconds.
        double TimeNow();

        int TimeNow_MS();

        // Sleep. Does not necessarily have millisecond granularity, especially on Windows.
        void SleepThread(int ms);
    }
}

#endif // TIMEUTIL_H
