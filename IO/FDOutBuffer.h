#ifndef FDOUTBUFFER_H
#define FDOUTBUFFER_H

#include "BASE/NBuffer.h"

namespace IO
{
    class FDOutBuffer final: public NOutBuffer
    {
    public:
        FDOutBuffer(int fd);
        ~FDOutBuffer();

        void setTimeout(int timeoutms);

        void flushBuffer(Size len, bool wait = true) override;

    protected:
        void write(Size len, const NBYTE *data, bool wait = true) override;

    private:
        int fd_;
        int timeoutms_;
    };
}

#endif // FDOUTBUFFER_H
