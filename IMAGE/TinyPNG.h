#ifndef THINPNG_H
#define THINPNG_H

#include "BASE/Honey.h"

namespace IMAGE
{
    // *image_data_ptr should be deleted with free()
    // return value of 1 == success.
    void PNGLoad(const char *file, int *pwidth,
                int *pheight, unsigned char **image_data_ptr);

    void PNGLoadPtr(const unsigned  char *input_ptr, size_t input_len, int *pwidth,
                int *pheight, unsigned char **image_data_ptr);
}

#endif // THINPNG_H
