#ifndef FDINBUFFER_H
#define FDINBUFFER_H

#include "BASE/NBuffer.h"

namespace IO
{
    class FDInBuffer final: protected NInBuffer
    {
    public:
        FDInBuffer(int fd);
        ~FDInBuffer();

        void setTimeout(int timeoutms);

        void take(Size length, NBYTE *dest, bool wait = true) override;
        void take(Size length, NBuffer &other, bool wait = true) override;

    private:
        void fillBuffer(Size length, bool wait = true) override;

    private:
        int fd_;
        int timeoutms_;
    };
}

#endif // FDINBUFFER_H
