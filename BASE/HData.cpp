#include "HData.h"

#include <string.h>

const HData HData::Null;

HData::HData()
    : bytes_(NULLPTR)
    , size_(0) {
}

HData::HData(HData&& other)
    : bytes_(NULLPTR)
    , size_(0) {
    move(other);
}

HData::HData(const HData& other)
    : bytes_(NULLPTR)
    , size_(0) {
    copy(other.bytes_, other.size_);
}

HData::~HData() {
    clear();
}

HData& HData::operator= (const HData& other) {
    copy(other.bytes_, other.size_);
    return *this;
}

HData& HData::operator= (HData&& other) {
    move(other);
    return *this;
}

void HData::move(HData& other) {
    bytes_ = other.bytes_;
    size_ = other.size_;

    other.bytes_ = NULLPTR;
    other.size_ = 0;
}

bool HData::isNull() const {
    return (bytes_ == NULLPTR || size_ == 0);
}

HBYTE* HData::getBytes() const {
    return bytes_;
}

size_t HData::getSize() const {
    return size_;
}

void HData::copy(const HBYTE* bytes, const size_t size) {
    clear();

    if (size > 0) {
        size_ = size;
        bytes_ = (HBYTE*)malloc(sizeof(HBYTE) * size_);
        memcpy(bytes_, bytes, size_);
    }
}

void HData::fastSet(HBYTE* bytes, const size_t size) {
    bytes_ = bytes;
    size_ = size;
}

void HData::clear() {
    free(bytes_);
    bytes_ = NULLPTR;
    size_ = 0;
}

