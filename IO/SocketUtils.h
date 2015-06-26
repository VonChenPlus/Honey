#ifndef SOCKETUTILS_H
#define SOCKETUTILS_H

#include <string>

#include "BASE/Honey.h"
#include "BASE/HBuffer.h"

namespace IO
{
    void ReadWithProgress(int fd, Size length, HBuffer &buffer, float *progress);
    void WriteWithProgress(int fd, Size length, HBuffer &buffer, float *progress);

    // Returns true if the fd became ready, false if it didn't or
    // if there was another error.
    void WaitUntilReady(int fd, int timeoutms, bool write = false);

    void SetNonBlocking(int fd, bool non_blocking);
}

#endif // SOCKETUTILS_H
