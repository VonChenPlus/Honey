#ifndef IOBUFFER_H
#define IOBUFFER_H

#include "BASE/NBuffer.h"

namespace IO
{
class IOBuffer : public NBuffer
{
    public:
        IOBuffer();

        // Simple I/O.

        // Writes the entire buffer to the file descriptor. Also resets the
        // size to zero. On failure, data remains in buffer and nothing is
        // written.
        void flush(int fd);
        void flushToFile(const char *filename);
        void flushSocket(UIntPtr sock);  // Windows portability

        void readAll(int fd, int hintSize = 0);
        void readAllWithProgress(int fd, int knownSize, float *progress);

        // < 0: error
        // >= 0: number of bytes read
        int read(int fd, Size sz);
    };
}

#endif // IOBUFFER_H
