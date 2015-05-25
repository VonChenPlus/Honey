#include "SocketUtils.h"

#include <errno.h>
#include <math.h>
#include <stdio.h>
#ifndef _WIN32
#include <unistd.h>
#include <sys/select.h>
#include <sys/socket.h>
#else
#include <io.h>
#include <winsock2.h>
#endif
#include <fcntl.h>
#include <string.h>

#include "UTILS/STRING/NString.h"
using UTILS::STRING::StringFromFormat;

namespace IO
{
    void ReadWithProgress(int fd, size_t length, NBuffer &buffer, float *progress) {
        std::vector<NBYTE> buf;
        if (length >= 1024 * 64 * 16) {
            buf.resize(1024 * 64);
        }
        else if (length >= 1024 * 16) {
            buf.resize(length / 16);
        }
        else {
            buf.resize(1024);
        }

        int total = 0;
        while (true) {
            int retval = recv(fd, &buf[0], (int)buf.size(), 0);
            if (retval == 0) {
                return;
            }
            else if (retval < 0) {
                throw _NException_(StringFromFormat("error reading from buffer: %i", retval), NException::IO);
            }

            buffer.append(&buf[0], retval);
            total += retval;
            if (progress)
                *progress = (float)total / (float)length;
        }
    }

    void WaitUntilReady(int fd, double timeout) {
        struct timeval tv;
        tv.tv_sec = floor(timeout);
        tv.tv_usec = (timeout - floor(timeout)) * 1000000.0;

        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(fd, &fds);
        // First argument to select is the highest socket in the set + 1.
        int rval = select(fd + 1, &fds, NULLPTR, NULLPTR, &tv);
        if (rval < 0) {
            throw _NException_("Error calling select", NException::IO);
        }
        else if (rval == 0) {
            throw _NException_("Timeout", NException::IO);
        }
    }

    void SetNonBlocking(int sock, bool non_blocking) {
        #ifndef _WIN32
        int opts = fcntl(sock, F_GETFL);
        if (opts < 0) {
            throw _NException_Normal("Error getting socket status while changing nonblocking status");
        }
        if (non_blocking) {
            opts = (opts | O_NONBLOCK);
        }
        else {
            opts = (opts & ~O_NONBLOCK);
        }

        if (fcntl(sock, F_SETFL, opts) < 0) {
            throw _NException_Normal("Error setting socket nonblocking status");
        }
        #else
        if (ioctlsocket(sock, FIONBIO, (unsigned long *)&non_blocking) < 0)
            throw _NException_("Error setting socket nonblocking status", NException::IO);
        #endif
    }
}
