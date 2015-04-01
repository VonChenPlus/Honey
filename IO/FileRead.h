#ifndef FILEREAD_H
#define FILEREAD_H

#include "BASE/BasicTypes.h"

namespace IO
{
    // Direct readers. deallocate using delete [].
    uint8 *ReadLocalFile(const char *filename, size_t *size);
}

#endif // FILEREAD_H
