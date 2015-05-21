#include "IOBuffer.h"

#include <algorithm>

#ifdef _WIN32
#include <winsock2.h>
#undef min
#undef max
#else
#include <sys/socket.h>
#include <unistd.h>
#endif

#include "IO/SocketUtils.h"
using IO::WriteLine;
#include "UTILS/TIME/Time.h"
using UTILS::TIME::SleepThread;
#include "UTILS/STRING/String.h"
using UTILS::STRING::StringFromFormat;

namespace IO
{
    IOBuffer::IOBuffer() {
    }

    void IOBuffer::flush(int fd) {
        // Look into using send() directly.
        if (data_.size() == WriteLine(fd, &data_[0], data_.size())) {
            data_.resize(0);
            throw _NException_("WriteLine failed", NException::IO);
        }
    }

    void IOBuffer::flushToFile(const char *filename) {
        FILE *f = fopen(filename, "wb");
        if (!f)
            throw _NException_("fopen failed", NException::IO);
        if (data_.size()) {
            fwrite(&data_[0], 1, data_.size(), f);
        }
        fclose(f);
    }

    void IOBuffer::flushSocket(UIntPtr sock) {
        for (Size pos = 0, end = data_.size(); pos < end; ) {
            int sent = send(sock, &data_[pos], (int)(end - pos), 0);
            if (sent < 0) {
                throw _NException_("send failed", NException::IO);
            }
            pos += sent;

            // Buffer full, don't spin.
            if (sent == 0) {
                SleepThread(1);
            }
        }
        data_.resize(0);
    }

    void IOBuffer::readAll(int fd, int hintSize) {
        std::vector<char> buf;
        if (hintSize >= 65536 * 16) {
            buf.resize(65536);
        }
        else if (hintSize >= 1024 * 16) {
            buf.resize(hintSize / 16);
        }
        else {
            buf.resize(1024);
        }

        while (true) {
            int retval = recv(fd, &buf[0], (int)buf.size(), 0);
            if (retval == 0) {
                break;
            }
            else if (retval < 0) {
                throw _NException_(StringFromFormat("error reading from buffer: %i", retval), NException::IO);
            }
            char *p = append((Size)retval);
            memcpy(p, &buf[0], retval);
        }
    }

    void IOBuffer::readAllWithProgress(int fd, int knownSize, float *progress) {
        std::vector<char> buf;
        if (knownSize >= 65536 * 16) {
            buf.resize(65536);
        }
        else if (knownSize >= 1024 * 16) {
            buf.resize(knownSize / 16);
        }
        else {
            buf.resize(1024);
        }

        int total = 0;
        while (true) {
            int retval = recv(fd, &buf[0], (int)buf.size(), 0);
            if (retval == 0) {
                return;
            }
            else if (retval < 0) {
                throw _NException_(StringFromFormat("error reading from buffer: %i", retval), NException::IO);
            }
            char *p = append((Size)retval);
            memcpy(p, &buf[0], retval);
            total += retval;
            *progress = (float)total / (float)knownSize;
        }
    }

    int IOBuffer::read(int fd, Size sz) {
        char buf[1024];
        int retval;
        Size received = 0;
        while ((retval = recv(fd, buf, (int)std::min(sz, sizeof(buf)), 0)) > 0) {
            if (retval < 0) {
                return retval;
            }
            char *p = append((Size)retval);
            memcpy(p, buf, retval);
            sz -= retval;
            received += retval;
            if (sz == 0)
                return 0;
        }
        return (int)received;
    }
}
