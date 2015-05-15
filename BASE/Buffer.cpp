#include "Buffer.h"

#include <stdarg.h>
#include <string.h>

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
    append(StringFromFormat("%i", value).c_str());
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
        throw _NException_Normal("vsnprintf truncated output");
    }
    if (retval < 0) {
        throw _NException_Normal("vsnprintf failed");
    }
    va_end(vl);
    char *ptr = append(retval);
    memcpy(ptr, buffer, retval);
}

void Buffer::peekAll(std::string *dest) {
    dest->resize(data_.size());
    memcpy(&(*dest)[0], &data_[0], data_.size());
}

