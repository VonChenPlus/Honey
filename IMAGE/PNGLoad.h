#ifndef PNGLOAD_H
#define PNGLOAD_H

#include "BASE/BasicTypes.h"

namespace IMAGE
{
    // *image_data_ptr should be deleted with free()
    // return value of 1 == success.
    void PNGLoad(const char *file, int *pwidth,
                int *pheight, unsigned char **image_data_ptr);

    void PNGLoadPtr(const unsigned  char *input_ptr, Size input_len, int *pwidth,
                int *pheight, unsigned char **image_data_ptr);
}

#endif // PNGLOAD_H
