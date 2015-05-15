#include "SocketUtils.h"

#include <errno.h>
#include <math.h>
#include <stdio.h>
#ifndef _WIN32
#include <unistd.h>
#include <sys/select.h>
#else
#include <io.h>
#include <winsock2.h>
#endif
#include <fcntl.h>
#include <string.h>

namespace IO
{
    // Slow as hell and should only be used for prototyping.
    // Reads from a socket, up to an '\n'. This means that if the line ends
    // with '\r', the '\r' will be returned.
    Size ReadLine(int fd, char *vptr, Size buf_size) {
        char *buffer = vptr;
        Size n;
        for (n = 1; n < buf_size; n++) {
            char c;
            int rc;
            if ((rc = read(fd, &c, 1)) == 1) {
                *buffer++ = c;
                if (c == '\n')
                    break;
            }
            else if (rc == 0) {
                if (n == 1)
                    return 0;
                else
                    break;
            }
            else {
                if (errno == EINTR)
                    continue;
                //FLOG("Error in Readline()");
            }
        }

        *buffer = 0;
        return n;
    }

    // Misnamed, it just writes raw data in a retry loop.
    Size WriteLine(int fd, const char *vptr, Size n) {
        const char *buffer = vptr;
        Size nleft = n;

        while (nleft > 0) {
            int nwritten;
            if ((nwritten = (int)write(fd, buffer, (unsigned int)nleft)) <= 0) {
                if (errno == EINTR)
                    nwritten = 0;
              //else
              //  FLOG("Error in Writeline()");
            }

            nleft  -= nwritten;
            buffer += nwritten;
        }

        return n;
    }

    Size WriteLine(int fd, const char *buffer) {
        return WriteLine(fd, buffer, strlen(buffer));
    }

    Size Write(int fd, const std::string &str) {
        return WriteLine(fd, str.c_str(), str.size());
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
        } else if (rval == 0) {
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
            throw _NException_Normal("Error setting socket nonblocking status");
        #endif
    }
}
