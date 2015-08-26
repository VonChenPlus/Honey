#ifndef HBUFFER_H
#define HBUFFER_H

#include <vector>
#include <string>

#include "BASE/Honey.h"

template <typename T>
inline T swap(T *value) {
    switch (sizeof(T)) {
    case 2: return (T)swap16((uint8 *)value);
    case 4: return (T)swap32((uint8 *)value);
    case 8: return (T)swap64((uint8 *)value);
    default:
        throw _HException_Normal("Unhander data bits!");
    }
}

// Acts as a queue. Intended to be as fast as possible for most uses.
// Does not do synchronization, must use external mutexes.
class HBuffer
{
public:
    HBuffer();
    virtual ~HBuffer();

    // These work pretty much like you'd expect.
    virtual void write(size_t len, const HBYTE *data, bool wait = true);
    void write(const HBuffer &other);
    void writeAsFormat(const HBYTE *fmt, ...);

    virtual void read(size_t length, HBYTE *dest, bool wait = true);
    virtual void read(size_t length, HBuffer &other, bool wait = true);
    virtual void peek(size_t length, HBYTE *dest, bool wait = true);
    virtual void peek(size_t length, HBuffer &other, bool wait = true);
    virtual void skip(size_t length, bool wait = true);

    // Utilities. Try to avoid checking for size.
    size_t size() const { return data_.size(); }
    bool empty() const { return size() == 0; }
    void clear() { data_.resize(0); }
    HBYTE *data() { return &data_[0]; }

protected:
    // Write max [length] bytes to the returned pointer.
    // Any other operation on this Buffer invalidates the pointer.
    HBYTE *appendBufferSize(size_t length);

    // TODO: Find a better internal representation, like a cord.
    std::vector<HBYTE> data_;

    DISALLOW_COPY_AND_ASSIGN(HBuffer)
};

class HInBuffer : protected HBuffer
{
public:
    template <typename T>
    void readAny(size_t length, T *dest, bool wait = true) {
        if (length >= sizeof(T) && bigEndian() && sizeof(T) != 1) {
            if (length % sizeof(T) != 0)
                throw _HException_Normal("Unaligned data size!");

            size_t count = length / sizeof(T);
            for (size_t index = 0; index < count; ++index) {
                read(sizeof(T), (HBYTE *)&dest[index], wait);
                if (dest) {
                    dest[index] = swap(&dest[index]);
                }
            }
        }
        else
            read(length, (HBYTE *)dest, wait);
    }

    template <typename T>
    void readOne(T *dest, bool wait = true) {
        readAny(sizeof(T), dest, wait);
    }

    template <typename T>
    T readOne(bool wait = true) {
        T temp;
        readOne(&temp, wait);
        return temp;
    }

    void skip(size_t length, bool wait = true) {
        readAny(length, (uint8 *)NULLPTR, wait);
    }

    template <typename T>
    void readAny(size_t length, T &dest, bool wait = true) {
        read(length, dest, wait);
    }

protected:
    void checkBuffer(size_t length, bool wait = true, bool throwException = true) {
        if (length > size()) {
            fillBuffer(length - size(), wait);
            if (throwException && wait && length > size()) {
                throw _HException_Normal("truncating length");
            }
        }
    }

    virtual void fillBuffer(size_t, bool = true) {}

    virtual bool bigEndian() { return false; }
};

class HOutBuffer: protected HBuffer
{
public:
    template <typename T>
    void writeAny(size_t length, T *dest, bool wait = true) {
        if (length >= sizeof(T) && bigEndian() && sizeof(T) != 1) {
            if (length % sizeof(T) != 0)
                throw _HException_Normal("Unaligned data size!");

            size_t count = length / sizeof(T);
            for (size_t index = 0; index < count; ++index) {
                T temp = swap(&dest[index]);
                write(sizeof(T), (HBYTE *)&temp, wait);
            }
        }
        else
            write(length, (HBYTE *)dest, wait);
    }

    template <typename T>
    void writeOne(T *dest, bool wait = true) {
        writeAny(sizeof(T), dest, wait);
    }

    template <typename T>
    void writeOne(T dest, bool wait = true) {
        writeAny(sizeof(T), &dest, wait);
    }

    void pad(size_t length) {
        uint8 temp = 0;
        for (size_t index = 0; index < length; ++index) {
            writeOne(&temp);
        }
    }

    virtual void flushBuffer(size_t = 0, bool = true) {}

protected:
    virtual bool bigEndian() { return false; }
};

#endif // HBUFFER_H
