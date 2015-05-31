#include "FDInBuffer.h"
#include "IO/TCPSocket.h"
#include "IO/SocketUtils.h"

namespace IO
{
    FDInBuffer::FDInBuffer(int fd)
        : fd_(fd) {
        if (!TCPSocket::isSocket(fd_))
            throw _NException_Normal("Invalid Sock object!");
    }

    FDInBuffer::~FDInBuffer() {
    }

    void FDInBuffer::setTimeout(int timeoutms) {
        timeoutms_ = timeoutms;
    }

    void FDInBuffer::take(Size length, NBYTE *dest) {
        check(length);
        NBuffer::take(length, dest);
    }

    void FDInBuffer::peek(Size length, NBYTE *dest) {
        check(length);
        NBuffer::peek(length, dest);
    }

    void FDInBuffer::check(Size length, bool wait) {
        if (length > size()) {
            overrun(length - size(), wait);
            if (wait && length > size()) {
                throw _NException_Normal("truncating length");
            }
        }
    }

    void FDInBuffer::overrun(Size length, bool wait) {
        WaitUntilReady(fd_, wait ? timeoutms_ : 0);
        ReadWithProgress(fd_, length, *this, NULLPTR);
    }
}

