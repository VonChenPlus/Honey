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
    }
}

#endif // TIMEUTIL_H
