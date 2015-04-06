#ifndef BUFFER_H
#define BUFFER_H

#include <vector>
#include <string>

#include "BASE/BasicTypes.h"

// Acts as a queue. Intended to be as fast as possible for most uses.
// Does not do synchronization, must use external mutexes.
class Buffer
{
public:
    Buffer();
    ~Buffer();

    // Write max [length] bytes to the returned pointer.
    // Any other operation on this Buffer invalidates the pointer.
    char *append(Size length);

    // These work pretty much like you'd expect.
    void append(const char *str);  // str null-terminated. The null is not copied.
    void append(const std::string &str);
    void append(const Buffer &other);

    // Various types. Useful for varz etc. Appends a string representation of the
    // value, rather than a binary representation.
    void appendValue(int value);

    // Parsing Helpers

    // Use for easy line skipping. If no CRLF within the buffer, returns -1.
    // If parsing HTML headers, this indicates that you should probably buffer up
    // more data.
    int offsetToAfterNextCRLF();

    // Takers

    void take(Size length, std::string *dest);
    void take(Size length, char *dest);
    void takeAll(std::string *dest) { take(size(), dest); }
    // On failure, return value < 0 and *dest is unchanged.
    // Strips off the actual CRLF from the result.
    int takeLineCRLF(std::string *dest);

    // Skippers
    void skip(Size length);
    // Returns -1 on failure (no CRLF within sight).
    // Otherwise returns the length of the line skipped, not including CRLF. Can be 0.
    int skipLineCRLF();

    // Utility functions.
    void printf(const char *fmt, ...);

    // Dumps the entire buffer to the string, but keeps it around.
    // Only to be used for debugging, since it might not be fast at all.
    void peekAll(std::string *dest);

    // Simple I/O.

    // Writes the entire buffer to the file descriptor. Also resets the
    // size to zero. On failure, data remains in buffer and nothing is
    // written.
    void flush(int fd);
    void flushToFile(const char *filename);
    void flushSocket(uintptr_t sock);  // Windows portability

    void readAll(int fd, int hintSize = 0);
    void readAllWithProgress(int fd, int knownSize, float *progress);

    // < 0: error
    // >= 0: number of bytes read
    int read(int fd, Size sz);

    // Utilities. Try to avoid checking for size.
    Size size() const { return data_.size(); }
    bool empty() const { return size() == 0; }
    void clear() { data_.resize(0); }

private:
    // TODO: Find a better internal representation, like a cord.
    std::vector<char> data_;

    DISALLOW_COPY_AND_ASSIGN(Buffer)
};

#endif // BUFFER_H
