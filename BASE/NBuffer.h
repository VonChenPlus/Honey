#ifndef NBUFFER_H
#define NBUFFER_H

#include <vector>
#include <string>

#include "BASE/Native.h"

// Acts as a queue. Intended to be as fast as possible for most uses.
// Does not do synchronization, must use external mutexes.
class NBuffer
{
public:
    NBuffer();
    virtual ~NBuffer();

    // These work pretty much like you'd expect.
    virtual void append(Size len, const NBYTE *data, bool wait = true);
    void append(const NBuffer &other);
    void appendFormat(const NBYTE *fmt, ...);
    void appendValue(int value);

    virtual void take(Size length, NBYTE *dest, bool wait = true);
    virtual void take(Size length, NBuffer &other, bool wait = true);
    virtual void peek(Size length, NBYTE *dest, bool wait = true);
    virtual void peek(Size length, NBuffer &other, bool wait = true);
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

    DISALLOW_COPY_AND_ASSIGN(NBuffer)
};

class NInBuffer : public NBuffer
{
protected:
    void checkBuffer(Size length, bool wait = true, bool throwException = true) {
        if (length > size()) {
            fillBuffer(length - size(), wait);
            if (throwException && wait && length > size()) {
                throw _NException_Normal("truncating length");
            }
        }
    }

    virtual void fillBuffer(Size, bool = true) {}
};

class NOutBuffer: public NBuffer
{
protected:
    virtual void flushBuffer(Size, bool = true) {}
};

#endif // NBUFFER_H
