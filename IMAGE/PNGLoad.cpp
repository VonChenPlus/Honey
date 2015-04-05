#include "PNGLoad.h"

#include <string.h>
#include <stdlib.h>

#include "EXTERNALS/libpng17/png.h"

namespace IMAGE
{
    // *image_data_ptr should be deleted with free()
    // return value of 1 == success.
    int PNGLoad(const char *file, int *pwidth, int *pheight, unsigned char **image_data_ptr) {
        png_image png;
        memset(&png, 0, sizeof(png));
        png.version = PNG_IMAGE_VERSION;

        png_image_begin_read_from_file(&png, file);

        if (PNG_IMAGE_FAILED(png)) {
            //ELOG("pngLoad: %s", png.message);
            return 0;
        }
        *pwidth = png.width;
        *pheight = png.height;
        png.format = PNG_FORMAT_RGBA;

        int stride = PNG_IMAGE_ROW_STRIDE(png);
        *image_data_ptr = (unsigned char *)malloc(PNG_IMAGE_SIZE(png));
        png_image_finish_read(&png, NULLPTR, *image_data_ptr, stride, NULLPTR);

        return 1;
    }

    int PNGLoadPtr(const unsigned char *input_ptr, Size input_len, int *pwidth, int *pheight, unsigned char **image_data_ptr) {
        png_image png;
        memset(&png, 0, sizeof(png));
        png.version = PNG_IMAGE_VERSION;

        png_image_begin_read_from_memory(&png, input_ptr, input_len);

        if (PNG_IMAGE_FAILED(png)) {
            //ELOG("pngLoad: %s", png.message);
            return 0;
        }
        *pwidth = png.width;
        *pheight = png.height;
        png.format = PNG_FORMAT_RGBA;

        int stride = PNG_IMAGE_ROW_STRIDE(png);
        *image_data_ptr = (unsigned char *)malloc(PNG_IMAGE_SIZE(png));
        png_image_finish_read(&png, NULLPTR, *image_data_ptr, stride, NULLPTR);

        return 1;
    }
}
