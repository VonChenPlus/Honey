#include "FDInBuffer.h"

#include "IO/TCPSocket.h"
#include "IO/SocketUtils.h"
#include "UTILS/STRING/StringUtils.h"

namespace IO
{
    FDInBuffer::FDInBuffer(int fd)
        : fd_(fd)
        , timeoutms_(0) {
        if (!TCPSocket::isSocket(fd_))
            throw _HException_Normal("Invalid Sock object!");
    }

    FDInBuffer::~FDInBuffer() {
    }

    void FDInBuffer::setTimeout(int timeoutms) {
        timeoutms_ = timeoutms;
    }

    void FDInBuffer::read(size_t length, HBYTE *dest, bool wait) {
        // do not know total buffer size sometimes, so we do not throw exception
        checkBuffer(length, wait, false);
        HBuffer::read(length, dest, wait);
    }

    void FDInBuffer::fillBuffer(size_t length, bool wait) {
        while (true) {
            try {
                WaitUntilReady(fd_, wait ? timeoutms_ : 0);
                break;
            }
            catch (HException e) {
                if (strcasecmp(e.reason().c_str(), "Timeout") != 0) {
                    break;
                }
            }
        }
        ReadWithProgress(fd_, length, *this, NULLPTR);
    }
}

