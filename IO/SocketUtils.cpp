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

#include "UTILS/STRING/StringUtils.h"
using UTILS::STRING::StringFromFormat;
#include "MATH/MathDef.h"

namespace IO
{
    void ReadWithProgress(int fd, size_t length, HBuffer &buffer, float *progress) {
        std::vector<HBYTE> buf;
        if (length >= 1024 * 64 * 16) {
            buf.resize(1024 * 64);
        }
        else if (length >= 1024 * 16) {
            buf.resize(length / 16);
        }
        else {
            buf.resize(1024);
        }

        size_t total = 0;
        do {
            int retval = recv(fd, (char *)&buf[0], (int)buf.size(), 0);
            if (retval == 0) {
                return;
            }
            else if (retval < 0) {
                throw _HException_(StringFromFormat("error recv from buffer: %i", retval), HException::IO);
            }

            buffer.write(retval, &buf[0]);
            total += retval;
            if (progress)
                *progress = (float)total / (float)length;
        } while (errorNumber == SOCKEINTR && total < length);
    }

    void WriteWithProgress(int fd, size_t length, HBuffer &buffer, float *progress) {
        std::vector<HBYTE> buf;
        if (length >= 1024 * 64 * 16) {
            buf.resize(1024 * 64);
        }
        else if (length >= 1024 * 16) {
            buf.resize(length / 16);
        }
        else {
            buf.resize(1024);
        }

        size_t total = 0;
        do {
            size_t bufSize = MATH::MATH_MIN(buf.size(), buffer.size());
            buffer.read(bufSize, &buf[0]);
            int retval = send(fd, (char *)&buf[0], (int)bufSize, 0);
            if (retval == 0) {
                return;
            }
            else if (retval < 0) {
                throw _HException_(StringFromFormat("error send to buffer: %i", retval), HException::IO);
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
            throw _HException_("Error calling select", HException::IO);
        }
        else if (rval == 0) {
            throw _HException_("Timeout", HException::IO);
        }
    }

    void SetNonBlocking(int sock, bool non_blocking) {
        #ifndef _WIN32
        int opts = fcntl(sock, F_GETFL);
        if (opts < 0) {
            throw _HException_Normal("Error getting socket status while changing nonblocking status");
        }
        if (non_blocking) {
            opts = (opts | O_NONBLOCK);
        }
        else {
            opts = (opts & ~O_NONBLOCK);
        }

        if (fcntl(sock, F_SETFL, opts) < 0) {
            throw _HException_Normal("Error setting socket nonblocking status");
        }
        #else
        if (ioctlsocket(sock, FIONBIO, (unsigned long *)&non_blocking) < 0)
            throw _HException_("Error setting socket nonblocking status", HException::IO);
        #endif
    }
}
