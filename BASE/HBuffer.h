#ifndef HBUFFER_H
#define HBUFFER_H

#include <vector>
#include <string>

#include "BASE/Honey.h"

// Acts as a queue. Intended to be as fast as possible for most uses.
// Does not do synchronization, must use external mutexes.
class HBuffer
{
public:
    HBuffer();
    virtual ~HBuffer();

    // These work pretty much like you'd expect.
    virtual void write(Size len, const NBYTE *data, bool wait = true);
    void write(const HBuffer &other);
    void writeAsFormat(const NBYTE *fmt, ...);

    virtual void read(Size length, NBYTE *dest, bool wait = true);
    virtual void read(Size length, HBuffer &other, bool wait = true);
    virtual void peek(Size length, NBYTE *dest, bool wait = true);
    virtual void peek(Size length, HBuffer &other, bool wait = true);
    virtual void skip(Size length, bool wait = true);

    // Utilities. Try to avoid checking for size.
    Size size() const { return data_.size(); }
    bool empty() const { return size() == 0; }
    void clear() { data_.resize(0); }
    NBYTE *data() { return &data_[0]; }

protected:
    // Write max [length] bytes to the returned pointer.
    // Any other operation on this Buffer invalidates the pointer.
    NBYTE *appendBufferSize(Size length);

    // TODO: Find a better internal representation, like a cord.
    std::vector<NBYTE> data_;

    DISALLOW_COPY_AND_ASSIGN(HBuffer)
};

class HInBuffer : protected HBuffer
{
public:
    template <typename T>
    void readAny(Size length, T *dest, bool wait = true) {
        read(length, (NBYTE *)dest, wait);
    }

    void read(Size length, NBYTE *dest, bool wait = true) override {
        HBuffer::read(length, dest, wait);
    }

    void read(Size length, HBuffer &other, bool wait = true) override {
        HBuffer::read(length, other, wait);
    }

protected:
    void checkBuffer(Size length, bool wait = true, bool throwException = true) {
        if (length > size()) {
            fillBuffer(length - size(), wait);
            if (throwException && wait && length > size()) {
                throw _HException_Normal("truncating length");
            }
        }
    }

    virtual void fillBuffer(Size, bool = true) {}

    virtual bool swapBuffer() { return false;  }
};

class HOutBuffer: protected HBuffer
{
public:
    template <typename T>
    void writeAny(Size length, T *dest, bool wait = true) {
        write(length, (NBYTE *)dest, wait);
    }

    virtual void flushBuffer(Size, bool = true) {}
};

#endif // HBUFFER_H
