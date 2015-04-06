#ifndef TIMEUTIL_H
#define TIMEUTIL_H

namespace UTILS
{
    namespace TIME
    {
        // http://linux.die.net/man/3/clock_gettime

        // This time implementation caches the time for max performance (call time_now() as much as you like).
        // You need to call time_update() once per frame (or whenever you need the correct time right now).

        void time_update();

        // Seconds.
        float time_now();
        double time_now_d();

        // Uncached time. Slower than the above cached time functions. Does not update cached time, call time_update for that.
        double real_time_now();

        int time_now_ms();


        // Sleep. Does not necessarily have millisecond granularity, especially on Windows.
        void sleep_ms(int ms);
    }
}

#endif // TIMEUTIL_H
