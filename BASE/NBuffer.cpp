#include "NBuffer.h"

#include <stdarg.h>
#include <string.h>

#include "UTILS/STRING/NString.h"
using UTILS::STRING::StringFromFormat;

NBuffer::NBuffer() {
    data_.resize(10);
}

NBuffer::~NBuffer() {
}

NBYTE *NBuffer::appendBufferSize(Size length) {
    Size old_size = data_.size();
    data_.resize(old_size + length);
    return &data_[0] + old_size;
}

void NBuffer::append(const NBYTE *data, Size len) {
    NBYTE *dest = appendBufferSize(len);
    memcpy(dest, data, len);
}

void NBuffer::append(const NBuffer &other) {
    Size len = other.size();
    NBYTE *dest = appendBufferSize(len);
    memcpy(dest, &other.data_[0], len);
}

void NBuffer::appendFormat(const NBYTE *fmt, ...) {
    NBYTE buffer[2048];
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
    NBYTE *ptr = appendBufferSize(retval);
    memcpy(ptr, buffer, retval);
}

void NBuffer::appendValue(int value) {
    std::string temp = StringFromFormat("%i", value);
    append(temp.c_str(), temp.size());
}

void NBuffer::take(Size length, NBYTE *dest) {
    if (length > data_.size()) {
        throw _NException_Normal("truncating length");
    }
    memcpy(dest, &data_[0], length);
    data_.erase(data_.begin(), data_.begin() + length);
}

void NBuffer::peek(Size length, NBYTE *dest) {
    if (length > data_.size()) {
        throw _NException_Normal("truncating length");
    }
    memcpy(dest, &data_[0], length);
}

void NBuffer::skip(Size length) {
    if (length > data_.size()) {
        throw _NException_Normal("truncating length");
    }
    data_.erase(data_.begin(), data_.begin() + length);
}
