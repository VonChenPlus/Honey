#include "FDInBuffer.h"
#include "IO/TCPSocket.h"
#include "IO/SocketUtils.h"

namespace IO
{
    FDInBuffer::FDInBuffer(int fd)
        : fd_(fd)
        , timeoutms_(0) {
        if (!TCPSocket::isSocket(fd_))
            throw _NException_Normal("Invalid Sock object!");
    }

    FDInBuffer::~FDInBuffer() {
    }

    void FDInBuffer::setTimeout(int timeoutms) {
        timeoutms_ = timeoutms;
    }

    void FDInBuffer::read(Size length, NBYTE *dest, bool wait) {
        // do not know total buffer size sometimes, so we do not throw exception
        checkBuffer(length, wait, false);
        NBuffer::read(length, dest, wait);
    }

    void FDInBuffer::fillBuffer(Size length, bool wait) {
        WaitUntilReady(fd_, wait ? timeoutms_ : 0);
        ReadWithProgress(fd_, length, *this, NULLPTR);
    }
}

