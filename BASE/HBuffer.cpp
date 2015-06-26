#include "HBuffer.h"

#include <stdarg.h>
#include <string.h>

#include "UTILS/STRING/HString.h"
using UTILS::STRING::StringFromFormat;

HBuffer::HBuffer() {
}

HBuffer::~HBuffer() {
}

NBYTE *HBuffer::appendBufferSize(Size length) {
    Size old_size = data_.size();
    data_.resize(old_size + length);
    return &data_[0] + old_size;
}

void HBuffer::write(Size len, const NBYTE *data, bool) {
    NBYTE *dest = appendBufferSize(len);
    memcpy(dest, data, len);
}

void HBuffer::write(const HBuffer &other) {
    Size len = other.size();
    NBYTE *dest = appendBufferSize(len);
    memcpy(dest, &other.data_[0], len);
}

void HBuffer::writeAsFormat(const NBYTE *fmt, ...) {
    NBYTE buffer[2048];
    va_list vl;
    va_start(vl, fmt);
    int retval = vsnprintf(buffer, sizeof(buffer), fmt, vl);
    if (retval >= (int)sizeof(buffer)) {
        throw _HException_Normal("vsnprintf truncated output");
    }
    if (retval < 0) {
        throw _HException_Normal("vsnprintf failed");
    }
    va_end(vl);
    NBYTE *ptr = appendBufferSize(retval);
    memcpy(ptr, buffer, retval);
}

void HBuffer::read(Size length, NBYTE *dest, bool) {
    if (length > data_.size()) {
        throw _HException_Normal("truncating length");
    }
    memcpy(dest, &data_[0], length);
    data_.erase(data_.begin(), data_.begin() + length);
}

void HBuffer::read(Size length, HBuffer &other, bool) {
    if (length > data_.size()) {
        throw _HException_Normal("truncating length");
    }

    other.appendBufferSize(length);
    memcpy(&other.data_[0], &data_[0], length);
    data_.erase(data_.begin(), data_.begin() + length);
}

void HBuffer::peek(Size length, NBYTE *dest, bool) {
    if (length > data_.size()) {
        throw _HException_Normal("truncating length");
    }
    memcpy(dest, &data_[0], length);
}

void HBuffer::peek(Size length, HBuffer &other, bool) {
    if (length > data_.size()) {
        throw _HException_Normal("truncating length");
    }

    other.appendBufferSize(length);
    memcpy(&other.data_[0], &data_[0], length);
}

void HBuffer::skip(Size length, bool) {
    if (length > data_.size()) {
        throw _HException_Normal("truncating length");
    }
    data_.erase(data_.begin(), data_.begin() + length);
}
