#include "HBuffer.h"

#include <stdarg.h>
#include <string.h>

#include "UTILS/STRING/StringUtils.h"
using UTILS::STRING::StringFromFormat;

HBuffer::HBuffer() {
}

HBuffer::~HBuffer() {
}

HBYTE *HBuffer::appendBufferSize(size_t length) {
    size_t old_size = data_.size();
    data_.resize(old_size + length);
    return &data_[0] + old_size;
}

void HBuffer::write(size_t len, const HBYTE *data, bool) {
    HBYTE *dest = appendBufferSize(len);
    memcpy(dest, data, len);
}

void HBuffer::write(const HBuffer &other) {
    size_t len = other.size();
    HBYTE *dest = appendBufferSize(len);
    memcpy(dest, &other.data_[0], len);
}

void HBuffer::writeAsFormat(const HBYTE *fmt, ...) {
    HBYTE buffer[2048];
    va_list vl;
    va_start(vl, fmt);
    int retval = vsnprintf((char *)buffer, sizeof(buffer), (const char *)fmt, vl);
    if (retval >= (int)sizeof(buffer)) {
        throw _HException_Normal("vsnprintf truncated output");
    }
    if (retval < 0) {
        throw _HException_Normal("vsnprintf failed");
    }
    va_end(vl);
    HBYTE *ptr = appendBufferSize(retval);
    memcpy(ptr, buffer, retval);
}

void HBuffer::read(size_t length, HBYTE *dest, bool) {
    if (length > data_.size()) {
        throw _HException_Normal("truncating length");
    }
    if (dest)
        memcpy(dest, &data_[0], length);
    data_.erase(data_.begin(), data_.begin() + length);
}

void HBuffer::read(size_t length, HBuffer &other, bool) {
    if (length > data_.size()) {
        throw _HException_Normal("truncating length");
    }

    other.appendBufferSize(length);
    memcpy(&other.data_[0], &data_[0], length);
    data_.erase(data_.begin(), data_.begin() + length);
}

void HBuffer::peek(size_t length, HBYTE *dest, bool) {
    if (length > data_.size()) {
        throw _HException_Normal("truncating length");
    }
    memcpy(dest, &data_[0], length);
}

void HBuffer::peek(size_t length, HBuffer &other, bool) {
    if (length > data_.size()) {
        throw _HException_Normal("truncating length");
    }

    other.appendBufferSize(length);
    memcpy(&other.data_[0], &data_[0], length);
}

void HBuffer::skip(size_t length, bool) {
    if (length > data_.size()) {
        throw _HException_Normal("truncating length");
    }
    data_.erase(data_.begin(), data_.begin() + length);
}
