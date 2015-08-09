#include "HTime.h"
#include <stdio.h>
#include "BASE/Honey.h"

namespace UTILS
{
    namespace TIME
    {
        static double curtime = 0;

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

        int gettimeofday(struct timeval * val, struct timezone *) {
            if (val) {
                LARGE_INTEGER liTime, liFreq;
                QueryPerformanceFrequency( &liFreq );
                QueryPerformanceCounter( &liTime );
                val->tv_sec     = (long)( liTime.QuadPart / liFreq.QuadPart );
                val->tv_usec    = (long)( liTime.QuadPart * 1000000.0 / liFreq.QuadPart - val->tv_sec * 1000000.0 );
            }
            return 0;
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
        }

        double TimeNow() {
            return curtime;
        }

        void SleepThread(int ms) {
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
