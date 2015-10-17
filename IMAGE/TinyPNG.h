#ifndef THINPNG_H
#define THINPNG_H

#include "BASE/Honey.h"
#include "EXTERNALS/libpng17/png.h"

namespace IMAGE
{
    // *image_data_ptr should be deleted with free()
    // return value of 1 == success.
    void PNGLoad(const char *file, int *pwidth, int *pheight, int *pcolor, unsigned char **data, int *datalen);

    void PNGLoadPtr(const unsigned  char *input_ptr, uint64 input_len, int *pwidth,
                int *pheight, int *pcolor, unsigned char **data, int *datalen);
}

#endif // THINPNG_H
