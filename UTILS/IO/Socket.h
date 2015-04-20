#ifndef SOCKET_H
#define SOCKET_H

#include "BASE/Native.h"
#include <string>

namespace UTILS
{
    namespace IO
    {
        // Slow as hell and should only be used for prototyping.
        Size ReadLine(int fd, char *buffer, Size buf_size);

        // Decently fast.
        Size WriteLine(int fd, const char *buffer, Size buf_size);
        Size WriteLine(int fd, const char *buffer);
        Size Write(int fd, const std::string &str);

        // Returns true if the fd became ready, false if it didn't or
        // if there was another error.
        void WaitUntilReady(int fd, double timeout);

        void SetNonBlocking(int fd, bool non_blocking);
    }
}

#endif // SOCKET_H