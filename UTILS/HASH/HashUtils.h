#ifndef HASHUTILS_H
#define HASHUTILS_H

#include "BASE/Honey.h"

namespace UTILS
{
    namespace HASH
    {
        uint32 Fletcher(const uint8 *data_u8, uint64 length);  // FAST. Length & 1 == 0.
        uint32 Adler32(const uint8 *data, uint64 len);         // Fairly accurate, slightly slower
    }
}

#endif // HASHUTILS_H
