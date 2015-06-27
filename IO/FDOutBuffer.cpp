#include "FDOutBuffer.h"

#include "IO/TCPSocket.h"
#include "IO/SocketUtils.h"
#include "UTILS/STRING/HString.h"

namespace IO
{
    #define MIN_BULK_SIZE 1024
    FDOutBuffer::FDOutBuffer(int fd)
        : fd_(fd)
        , timeoutms_(-1) {
        if (!TCPSocket::isSocket(fd_))
            throw _HException_Normal("Invalid Sock object!");
    }

    FDOutBuffer::~FDOutBuffer() {
    }

    void FDOutBuffer::setTimeout(int timeoutms) {
        timeoutms_ = timeoutms;
    }

    void FDOutBuffer::write(Size len, const HBYTE *data, bool wait) {
        if (len < MIN_BULK_SIZE) {
            HBuffer::write(len, data, wait);
            return;
        }

        HBuffer::write(len, data, wait);
        flushBuffer(size(), wait);
    }

    void FDOutBuffer::flushBuffer(Size len, bool wait) {
        while (true) {
            try {
                WaitUntilReady(fd_, wait ? timeoutms_ : 0, true);
                break;
            }
            catch (HException e) {
                if (strcasecmp(e.reason().c_str(), "Timeout") != 0) {
                    break;
                }
            }
        }
        WriteWithProgress(fd_, len, *this, NULLPTR);
    }
}
