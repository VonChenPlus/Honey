#ifndef FDINBUFFER_H
#define FDINBUFFER_H

#include "BASE/HBuffer.h"

namespace IO
{
    class FDInBuffer final: public HInBuffer
    {
    public:
        FDInBuffer(int fd);
        ~FDInBuffer();

        void setTimeout(int timeoutms);

    private:
        void read(size_t length, HBYTE *dest, bool wait = true) override;
        void fillBuffer(size_t length, bool wait = true) override;
        bool bigEndian() override { return true; }

    private:
        int fd_;
        int timeoutms_;
    };
}

#endif // FDINBUFFER_H
