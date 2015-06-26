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

    protected:
        void read(Size length, NBYTE *dest, bool wait = true) override;

    private:
        void fillBuffer(Size length, bool wait = true) override;
        bool swapBuffer() override { return true;  }

    private:
        int fd_;
        int timeoutms_;
    };
}

#endif // FDINBUFFER_H
