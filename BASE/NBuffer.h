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
    void append(const char *data, Size len);
    void append(const NBuffer &other);
    void appendFormat(const char *fmt, ...);
    void appendValue(int value);

    // Takers
    void take(Size length, char *dest, bool peek = false);

    // Skippers
    void skip(Size length);

    // Utilities. Try to avoid checking for size.
    Size size() const { return data_.size(); }
    bool empty() const { return size() == 0; }
    void clear() { data_.resize(0); }

protected:
    // Write max [length] bytes to the returned pointer.
    // Any other operation on this Buffer invalidates the pointer.
    char *appendBufferSize(Size length);

    // TODO: Find a better internal representation, like a cord.
    std::vector<char> data_;

    DISALLOW_COPY_AND_ASSIGN(NBuffer)
};

#endif // NBUFFER_H
