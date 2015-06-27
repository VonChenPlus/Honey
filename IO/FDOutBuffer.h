#ifndef FDOUTBUFFER_H
#define FDOUTBUFFER_H

#include "BASE/HBuffer.h"

namespace IO
{
    class FDOutBuffer final: public HOutBuffer
    {
    public:
        FDOutBuffer(int fd);
        ~FDOutBuffer();

        void setTimeout(int timeoutms);

        void flushBuffer(Size len, bool wait = true) override;

    protected:
        void write(Size len, const HBYTE *data, bool wait = true) override;
        bool bigEndian() override { return true; }

    private:
        int fd_;
        int timeoutms_;
    };
}

#endif // FDOUTBUFFER_H
