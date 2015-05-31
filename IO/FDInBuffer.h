#ifndef FDINBUFFER_H
#define FDINBUFFER_H

#include "BASE/NBuffer.h"

namespace IO
{
    class FDInBuffer final: protected NBuffer
    {
    public:
        FDInBuffer(int fd);
        ~FDInBuffer();

        void setTimeout(int timeoutms);

        void take(Size length, NBYTE *dest) override;
        void peek(Size length, NBYTE *dest) override;

    private:
        void check(Size length, bool wait = true);
        void overrun(Size length, bool wait = true);

    private:
        int timeoutms_;
        int fd_;
    };
}

#endif // FDINBUFFER_H
