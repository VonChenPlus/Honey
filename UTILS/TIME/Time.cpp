#include "Time.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/time.h>
#include <unistd.h>
#endif
#include <stdio.h>

#include "BASE/BasicTypes.h"

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

        double real_time_now() {
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

        double real_time_now() {
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

        void time_update() {
            curtime = real_time_now();
            curtime_f = (float)curtime;

            //printf("curtime: %f %f\n", curtime, curtime_f);
            // also smooth time.
            //curtime+=float((double) (time-_starttime) / (double) _frequency);
            //curtime*=0.5f;
            //curtime+=1.0f/60.0f;
            //lastTime=curtime;
            //curtime_f = (float)curtime;
        }

        float time_now() {
            return curtime_f;
        }

        double time_now_d() {
            return curtime;
        }

        int time_now_ms() {
            return int(curtime*1000.0);
        }

        void sleep_ms(int ms) {
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
