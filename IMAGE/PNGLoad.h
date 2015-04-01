#ifndef PNGLOAD_H
#define PNGLOAD_H

#include "BASE/BasicTypes.h"

namespace IMAGE
{
    // *image_data_ptr should be deleted with free()
    // return value of 1 == success.
    int PNGLoad(const char *file, int *pwidth,
                int *pheight, unsigned char **image_data_ptr);

    int PNGLoadPtr(const unsigned  char *input_ptr, size_t input_len, int *pwidth,
                int *pheight, unsigned char **image_data_ptr);
}

#endif // PNGLOAD_H
