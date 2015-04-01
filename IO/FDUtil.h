#ifndef FDUTIL_H
#define FDUTIL_H

#include "BASE/BasicTypes.h"
#include <string>

namespace IO
{
    // Slow as hell and should only be used for prototyping.
    ssize_t ReadLine(int fd, char *buffer, size_t buf_size);

    // Decently fast.
    ssize_t WriteLine(int fd, const char *buffer, size_t buf_size);
    ssize_t WriteLine(int fd, const char *buffer);
    ssize_t Write(int fd, const std::string &str);

    // Returns true if the fd became ready, false if it didn't or
    // if there was another error.
    bool WaitUntilReady(int fd, double timeout);

    void SetNonBlocking(int fd, bool non_blocking);
}

#endif // FDUTIL_H
