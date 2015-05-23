#ifndef SOCKINBUFFER_H
#define SOCKINBUFFER_H

#include "BASE/NBuffer.h"

namespace IO
{
    class SockInBuffer final: protected NBuffer
    {
    public:
        SockInBuffer(int sock);
        ~SockInBuffer();

        void setTimeout(int timeoutms);

        void take(Size length, NBYTE *dest) override;
        void peek(Size length, NBYTE *dest) override;

        // Skippers
        void skip(Size length) override;

    private:
        void check(Size length, bool wait = true);
        void overrun(Size length, bool wait = true);

    private:
        int timeoutms_;
        int sock_;
    };
}

#endif // SOCKINBUFFER_H
