#ifndef HASH_H
#define HASH_H

#include "BASE/Native.h"

namespace UTILS
{
    namespace HASH
    {
        uint32 Fletcher(const uint8 *data_u8, Size length);  // FAST. Length & 1 == 0.
        uint32 Adler32(const uint8 *data, Size len);         // Fairly accurate, slightly slower
    }
}

#endif // HASH_H
