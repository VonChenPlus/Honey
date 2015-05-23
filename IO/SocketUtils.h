#ifndef SOCKETUTILS_H
#define SOCKETUTILS_H

#include <string>

#include "BASE/Native.h"
#include "BASE/NBuffer.h"

namespace IO
{
    void ReadWithProgress(int fd, int length, NBuffer &buffer, float *progress);

    // Returns true if the fd became ready, false if it didn't or
    // if there was another error.
    void WaitUntilReady(int fd, double timeout);

    void SetNonBlocking(int fd, bool non_blocking);
}

#endif // SOCKETUTILS_H
