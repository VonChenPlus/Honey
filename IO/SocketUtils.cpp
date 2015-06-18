#include "SocketUtils.h"

#include <errno.h>
#include <math.h>
#include <stdio.h>
#ifndef _WIN32
#include <unistd.h>
#include <sys/select.h>
#include <sys/socket.h>
#define errorNumber errno
#define SOCKEINTR EINTR
#else
#include <io.h>
#include <winsock2.h>
#define errorNumber WSAGetLastError()
#define SOCKEINTR WSAEINTR
#endif
#include <fcntl.h>
#include <string.h>
#include <algorithm>
#undef min

#include "UTILS/STRING/NString.h"
using UTILS::STRING::StringFromFormat;

namespace IO
{
    void ReadWithProgress(int fd, Size length, NBuffer &buffer, float *progress) {
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

        Size total = 0;
        do {
            int retval = recv(fd, &buf[0], (int)buf.size(), 0);
            if (retval == 0) {
                return;
            }
            else if (retval < 0) {
                throw _NException_(StringFromFormat("error recv from buffer: %i", retval), NException::IO);
            }

            buffer.write(retval, &buf[0]);
            total += retval;
            if (progress)
                *progress = (float)total / (float)length;
        } while (errorNumber == SOCKEINTR && total < length);
    }

    void WriteWithProgress(int fd, Size length, NBuffer &buffer, float *progress) {
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

        Size total = 0;
        do {
            Size bufSize = std::min(buf.size(), buffer.size());
            buffer.read(bufSize, &buf[0]);
            int retval = send(fd, &buf[0], (int)buf.size(), 0);
            if (retval == 0) {
                return;
            }
            else if (retval < 0) {
                throw _NException_(StringFromFormat("error send to buffer: %i", retval), NException::IO);
            }

            total += retval;
            if (progress)
                *progress = (float)total / (float)length;
        } while (errorNumber == SOCKEINTR && total < length);
    }

    void WaitUntilReady(int fd, int timeoutms, bool write) {
        struct timeval tv;
        struct timeval* tvp = &tv;
        if (timeoutms != -1) {
            tv.tv_sec = timeoutms / 1000;
            tv.tv_usec = (timeoutms % 1000) * 1000;
        }
        else {
            tvp = 0;
        }

        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(fd, &fds);
        // First argument to select is the highest socket in the set + 1.
        int rval = 0;
        if (!write)
            rval = select(fd + 1, &fds, NULLPTR, NULLPTR, tvp);
        else
            rval = select(fd + 1, NULLPTR, &fds, NULLPTR, tvp);

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
