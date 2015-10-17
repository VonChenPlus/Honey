#include "TinyPNG.h"

#include <string.h>
#include <stdlib.h>
#include "EXTERNALS/libpng17/pngpriv.h"
#include "UTILS/STRING/StringUtils.h"
using UTILS::STRING::StringFromFormat;

namespace IMAGE
{
    // *image_data_ptr should be deleted with free()
    // return value of 1 == success.
    void PNGLoad(const char *file, int *pwidth, int *pheight, int *pcolor, unsigned char **data, int *datalen) {
        png_image png;
        memset(&png, 0, sizeof(png));
        png.version = PNG_IMAGE_VERSION;

        png_image_begin_read_from_file(&png, file);

        if (PNG_IMAGE_FAILED(png)) {
            throw _HException_(StringFromFormat("pngLoad: %s", png.message), HException::IO);
        }
        *pwidth = png.width;
        *pheight = png.height;
        *pcolor = png.opaque->png_ptr->color_type;

        int stride = PNG_IMAGE_ROW_STRIDE(png);
        *datalen = PNG_IMAGE_SIZE(png);
        *data = (unsigned char *)malloc(*datalen);
        png_image_finish_read(&png, nullptr, *data, stride, nullptr);
    }

    void PNGLoadPtr(const unsigned char *input_ptr, uint64 input_len, int *pwidth, int *pheight, int *pcolor, unsigned char **data, int *datalen) {
        png_image png;
        memset(&png, 0, sizeof(png));
        png.version = PNG_IMAGE_VERSION;

        png_image_begin_read_from_memory(&png, input_ptr, input_len);

        if (PNG_IMAGE_FAILED(png)) {
            throw _HException_(StringFromFormat("pngLoad: %s", png.message), HException::IO);
        }
        *pwidth = png.width;
        *pheight = png.height;
        *pcolor = png.opaque->png_ptr->color_type;

        int stride = PNG_IMAGE_ROW_STRIDE(png);
        *datalen = PNG_IMAGE_SIZE(png);
        *data = (unsigned char *)malloc(*datalen);
        png_image_finish_read(&png, nullptr, *data, stride, nullptr);
    }
}
