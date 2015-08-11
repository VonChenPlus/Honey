#ifndef TIMEUTIL_H
#define TIMEUTIL_H

#ifdef _WIN32
#include <Windows.h>
#else
#include <sys/time.h>
#include <unistd.h>
#endif

namespace UTILS
{
    namespace TIME
    {
        double FetchCurrentTime();
        int GetTimeOfDay(struct timeval * val, struct timezone *zone);

        void TimeUpdate();
        double TimeNow();
        int TimeNow_MS();

        void SleepThread(int ms);
    }
}

#endif // TIMEUTIL_H
