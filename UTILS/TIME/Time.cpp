#include "Time.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/time.h>
#include <unistd.h>
#endif
#include <stdio.h>

#include "BASE/Native.h"

namespace UTILS
{
    namespace TIME
    {
        static double curtime = 0;
        static float curtime_f = 0;

        #ifdef _WIN32

        LARGE_INTEGER frequency;
        double frequencyMult;
        LARGE_INTEGER startTime;

        double FetchCurrentTime() {
            if (frequency.QuadPart == 0) {
                QueryPerformanceFrequency(&frequency);
                QueryPerformanceCounter(&startTime);
                curtime = 0.0;
                frequencyMult = 1.0 / static_cast<double>(frequency.QuadPart);
            }
            LARGE_INTEGER time;
            QueryPerformanceCounter(&time);
            double elapsed = static_cast<double>(time.QuadPart - startTime.QuadPart);
            return elapsed * frequencyMult;
        }

        #else

        uint64 _frequency = 0;
        uint64 _starttime = 0;

        double FetchCurrentTime() {
            static time_t start;
            struct timeval tv;
            gettimeofday(&tv, NULLPTR);
            if (start == 0) {
                start = tv.tv_sec;
            }
            tv.tv_sec -= start;
            return (double)tv.tv_sec + (double)tv.tv_usec / 1000000.0;
        }

        #endif

        void TimeUpdate() {
            curtime = FetchCurrentTime();
            curtime_f = (float)curtime;
        }

        float TimeNow() {
            return curtime_f;
        }

        double TimeNowD() {
            return curtime;
        }

        int TimeNow_MS() {
            return int(curtime*1000.0);
        }

        void Sleep_MS(int ms) {
            #ifdef _WIN32
            #ifndef METRO
                Sleep(ms);
            #endif
            #else
                usleep(ms * 1000);
            #endif
        }
    }
}
