#include "Buffer.h"

#include <stdarg.h>
#include <string.h>

#include "UTILS/STRING/String.h"
using UTILS::STRING::StringFromFormat;

Buffer::Buffer() {
}

Buffer::~Buffer() {
}

char *Buffer::appendBufferSize(Size length) {
    Size old_size = data_.size();
    data_.resize(old_size + length);
    return &data_[0] + old_size;
}

void Buffer::append(const char *data, Size len) {
    char *dest = appendBufferSize(len);
    memcpy(dest, data, len);
}

void Buffer::append(const Buffer &other) {
    Size len = other.size();
    char *dest = appendBufferSize(len);
    memcpy(dest, &other.data_[0], len);
}

void Buffer::appendFormat(const char *fmt, ...) {
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
    char *ptr = appendBufferSize(retval);
    memcpy(ptr, buffer, retval);
}

void Buffer::appendValue(int value) {
    std::string temp = StringFromFormat("%i", value);
    append(temp.c_str(), temp.size());
}

void Buffer::take(Size length, char *dest, bool peek) {
    if (length > data_.size())
        throw _NException_Normal("truncating length");

    memcpy(dest, &data_[0], length);
    if (!peek)
        data_.erase(data_.begin(), data_.begin() + length);
}

void Buffer::skip(Size length) {
    if (length > data_.size()) {
        throw _NException_Normal("truncating length");
    }
    data_.erase(data_.begin(), data_.begin() + length);
}
