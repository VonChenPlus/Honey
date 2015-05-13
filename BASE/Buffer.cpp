#include "Buffer.h"

#include <stdarg.h>
#include <stdlib.h>
#include <algorithm>
#include <string.h>

#ifdef _WIN32
#include <winsock2.h>
#undef min
#undef max
#else
#include <sys/socket.h>
#include <unistd.h>
#endif

#include "IO/Socket.h"
using IO::WriteLine;
#include "UTILS/TIME/Time.h"
using UTILS::TIME::sleep_ms;
#include "UTILS/STRING/String.h"
using UTILS::STRING::StringFromFormat;

Buffer::Buffer() {
}

Buffer::~Buffer() {
}

char *Buffer::append(Size length) {
    Size old_size = data_.size();
    data_.resize(old_size + length);
    return &data_[0] + old_size;
}

void Buffer::append(const std::string &str) {
    char *ptr = append(str.size());
    memcpy(ptr, str.data(), str.size());
}

void Buffer::append(const char *str) {
    Size len = strlen(str);
    char *dest = append(len);
    memcpy(dest, str, len);
}

void Buffer::append(const Buffer &other) {
    Size len = other.size();
    char *dest = append(len);
    memcpy(dest, &other.data_[0], len);
}

void Buffer::appendValue(int value) {
    char buf[16];
    // This is slow.
    sprintf(buf, "%i", value);
    append(buf);
}

void Buffer::take(Size length, std::string *dest) {
    if (length > data_.size()) {
        throw _NException_Normal("truncating length");
    }
    dest->resize(length);
    if (length > 0) {
        take(length, &(*dest)[0]);
    }
}

void Buffer::take(Size length, char *dest) {
    memcpy(dest, &data_[0], length);
    data_.erase(data_.begin(), data_.begin() + length);
}

int Buffer::takeLineCRLF(std::string *dest) {
    int after_next_line = offsetToAfterNextCRLF();
    if (after_next_line < 0)
        return after_next_line;
    else {
        take(after_next_line - 2, dest);
        skip(2);  // Skip the CRLF
        return after_next_line - 2;
    }
}

void Buffer::skip(Size length) {
    if (length > data_.size()) {
        throw _NException_Normal("truncating length");
    }
    data_.erase(data_.begin(), data_.begin() + length);
}

int Buffer::skipLineCRLF()
{
    int after_next_line = offsetToAfterNextCRLF();
    if (after_next_line < 0)
        return after_next_line;
    else {
        skip(after_next_line);
        return after_next_line - 2;
    }
}

int Buffer::offsetToAfterNextCRLF()
{
    for (int i = 0; i < (int)data_.size() - 1; i++) {
        if (data_[i] == '\r' && data_[i + 1] == '\n') {
          return i + 2;
        }
    }
    return -1;
}

void Buffer::printf(const char *fmt, ...) {
    char buffer[2048];
    va_list vl;
    va_start(vl, fmt);
    int retval = vsnprintf(buffer, sizeof(buffer), fmt, vl);
    if (retval >= (int)sizeof(buffer)) {
        // Output was truncated. TODO: Do something.
        throw _NException_Normal("vsnprintf truncated output");
    }
    if (retval < 0) {
        throw _NException_Normal("vsnprintf failed");
    }
    va_end(vl);
    char *ptr = append(retval);
    memcpy(ptr, buffer, retval);
}

void Buffer::flush(int fd) {
    // Look into using send() directly.
    if (data_.size() == WriteLine(fd, &data_[0], data_.size())) {
        data_.resize(0);
        throw _NException_("WriteLine failed", NException::IO);
    }
}

void Buffer::flushToFile(const char *filename) {
    FILE *f = fopen(filename, "wb");
    if (!f)
        throw _NException_("fopen failed", NException::IO);
    if (data_.size()) {
        fwrite(&data_[0], 1, data_.size(), f);
    }
    fclose(f);
}

void Buffer::flushSocket(uintptr_t sock) {
    for (Size pos = 0, end = data_.size(); pos < end; ) {
        int sent = send(sock, &data_[pos], (int)(end - pos), 0);
        if (sent < 0) {
            throw _NException_("send failed", NException::IO);
        }
        pos += sent;

        // Buffer full, don't spin.
        if (sent == 0) {
            sleep_ms(1);
        }
    }
    data_.resize(0);
}

void Buffer::readAll(int fd, int hintSize) {
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

void Buffer::readAllWithProgress(int fd, int knownSize, float *progress) {
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

int Buffer::read(int fd, Size sz) {
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

void Buffer::peekAll(std::string *dest) {
    dest->resize(data_.size());
    memcpy(&(*dest)[0], &data_[0], data_.size());
}

