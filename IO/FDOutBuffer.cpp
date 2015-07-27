#include "FDOutBuffer.h"

#include "IO/TCPSocket.h"
#include "IO/SocketUtils.h"
#include "UTILS/STRING/StringUtils.h"

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

    void FDOutBuffer::write(size_t length, const HBYTE *data, bool wait) {
        if (length < MIN_BULK_SIZE) {
            HBuffer::write(length, data, wait);
            return;
        }

        HBuffer::write(length, data, wait);
        flushBuffer(size(), wait);
    }

    void FDOutBuffer::flushBuffer(size_t length, bool wait) {
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

        if (length == 0)
            length = this->size();
        WriteWithProgress(fd_, length, *this, NULLPTR);
    }
}
