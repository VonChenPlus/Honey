#include "SockInBuffer.h"
#include "IO/TCPSocket.h"
#include "IO/SocketUtils.h"

namespace IO
{
    SockInBuffer::SockInBuffer(int sock)
        : sock_(sock) {
        if (!TCPSocket::isSocket(sock_))
            throw _NException_Normal("Invalid Sock object!");
    }

    SockInBuffer::~SockInBuffer() {
    }

    void SockInBuffer::setTimeout(int timeoutms) {
        timeoutms_ = timeoutms;
    }

    void SockInBuffer::take(Size length, NBYTE *dest) {
        check(length);
        NBuffer::take(length, dest);
    }

    void SockInBuffer::peek(Size length, NBYTE *dest) {
        check(length);
        NBuffer::peek(length, dest);
    }

    void SockInBuffer::check(Size length, bool wait) {
        if (length > size()) {
            overrun(length - size(), wait);
            if (wait && length > size()) {
                throw _NException_Normal("truncating length");
            }
        }
    }

    void SockInBuffer::overrun(Size length, bool wait) {
        WaitUntilReady(sock_, wait ? timeoutms_ : 0);
        ReadWithProgress(sock_, length, *this, NULLPTR);
    }
}

