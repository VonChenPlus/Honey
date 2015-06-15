#include "FDOutBuffer.h"
#include "IO/TCPSocket.h"
#include "IO/SocketUtils.h"

namespace IO
{
    #define MIN_BULK_SIZE 1024
    FDOutBuffer::FDOutBuffer(int fd)
        : fd_(fd)
        , timeoutms_(0) {
        if (!TCPSocket::isSocket(fd_))
            throw _NException_Normal("Invalid Sock object!");
    }

    FDOutBuffer::~FDOutBuffer() {
    }

    void FDOutBuffer::setTimeout(int timeoutms) {
        timeoutms_ = timeoutms;
    }

    void FDOutBuffer::append(Size len, const NBYTE *data, bool wait) {
        if (len < MIN_BULK_SIZE) {
            NBuffer::append(len, data, wait);
            return;
        }

        NBuffer::append(len, data, wait);
        flushBuffer(size(), wait);
    }

    void FDOutBuffer::flushBuffer(Size len, bool wait) {
        WaitUntilReady(fd_, wait ? timeoutms_ : 0);
        WriteWithProgress(fd_, len, *this, NULLPTR);
    }
}
