#include "TinyZim.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <QImage>
#include "EXTERNALS/zlib/zlib.h"
#include "EXTERNALS/rg_etc1/rg_etc1.h"
#include "IO/FileUtils.h"
using IO::ReadLocalFile;
using IO::OpenFile;
#include "UTILS/STRING/NString.h"
using UTILS::STRING::StringFromFormat;
#include "MATH/Utils.h"
using MATH::IsPowerOf2;
using MATH::Clamp16;
using MATH::Clamp32;
using MATH::Clamp64;

namespace IMAGE
{
    static unsigned int log2i(unsigned int val) {
        unsigned int ret = -1;
        while (val != 0) {
            val >>= 1; ret++;
        }
        return ret;
    }

    int ezuncompress(unsigned char* pDest, long* pnDestLen, const unsigned char* pSrc, long nSrcLen) {
        z_stream stream;
        stream.next_in = (Bytef*)pSrc;
        stream.avail_in = (uInt)nSrcLen;
        /* Check for source > 64K on 16-bit machine: */
        if ((uLong)stream.avail_in != (uLong)nSrcLen) return Z_BUF_ERROR;

        uInt destlen = (uInt)*pnDestLen;
        if ((uLong)destlen != (uLong)*pnDestLen) return Z_BUF_ERROR;
        stream.zalloc = (alloc_func)0;
        stream.zfree = (free_func)0;

        int err = inflateInit(&stream);
        if (err != Z_OK) return err;

        int nExtraChunks = 0;
        do {
            stream.next_out = pDest;
            stream.avail_out = destlen;
            err = inflate(&stream, Z_FINISH);
            if (err == Z_STREAM_END )
                break;
            if (err == Z_NEED_DICT || (err == Z_BUF_ERROR && stream.avail_in == 0))
                err = Z_DATA_ERROR;
            if (err != Z_BUF_ERROR) {
                inflateEnd(&stream);
                return err;
            }
            nExtraChunks += 1;
        } while (stream.avail_out == 0);

        *pnDestLen = stream.total_out;

        err = inflateEnd(&stream);
        if (err != Z_OK) return err;

        return nExtraChunks ? Z_BUF_ERROR : Z_OK;
    }

    int LoadZIMPtr(const uint8 *zim, Size datasize, int *width, int *height, int *flags, uint8 **image) {
        if (zim[0] != 'Z' || zim[1] != 'I' || zim[2] != 'M' || zim[3] != 'G') {
            throw _NException_Normal("Not a ZIM file");
        }
        memcpy(width, zim + 4, 4);
        memcpy(height, zim + 8, 4);
        memcpy(flags, zim + 12, 4);

        int num_levels = 1;
        int image_data_size[ZIM_MAX_MIP_LEVELS];
        if (*flags & ZIM_HAS_MIPS) {
            num_levels = log2i(*width < *height ? *width : *height) + 1;
        }
        int total_data_size = 0;
        for (int i = 0; i < num_levels; i++) {
            if (i > 0) {
                width[i] = width[i-1] / 2;
                height[i] = height[i-1] / 2;
            }
            switch (*flags & ZIM_FORMAT_MASK) {
            case ZIM_RGBA8888:
                image_data_size[i] = width[i] * height[i] * 4;
                break;
            case ZIM_RGBA4444:
            case ZIM_RGB565:
                image_data_size[i] = width[i] * height[i] * 2;
                break;
            case ZIM_ETC1: {
                    int data_width = width[i];
                    int data_height = height[i];
                    if (data_width < 4) data_width = 4;
                    if (data_height < 4) data_height = 4;
                    image_data_size[i] = data_width * data_height / 2;
                    break;
                }
            default:
                throw _NException_Normal(StringFromFormat("Invalid ZIM format %i", *flags & ZIM_FORMAT_MASK));
            }
            total_data_size += image_data_size[i];
        }

        if (total_data_size == 0) {
            throw _NException_Normal("Invalid ZIM data size 0");
        }

        image[0] = (uint8 *)malloc(total_data_size);
        for (int i = 1; i < num_levels; i++) {
            image[i] = image[i-1] + image_data_size[i-1];
        }

        if (*flags & ZIM_ZLIB_COMPRESSED) {
            long outlen = (long)total_data_size;
            if (Z_OK != ezuncompress(*image, &outlen, (unsigned char *)(zim + 16), (long)datasize - 16)) {
                free(*image);
                *image = 0;
                return 0;
            }
            if (outlen != total_data_size) {
                throw _NException_Normal(StringFromFormat("Wrong size data in ZIM: %i vs %i", (int)outlen, (int)total_data_size));
            }
        }
        else {
            memcpy(*image, zim + 16, datasize - 16);
            if (datasize - 16 != (Size)total_data_size) {
                throw _NException_Normal(StringFromFormat("Wrong size data in ZIM: %i vs %i", (int)(datasize-16), (int)total_data_size));
            }
        }
        return num_levels;
    }

    int LoadZIM(const char *filename, int *width, int *height, int *format, uint8 **image) {
        Size size;
        uint8 *buffer = ReadLocalFile(filename, &size);
        int retval = LoadZIMPtr(buffer, (int)size, width, height, format, image);
        delete[] buffer;
        return retval;
    }

    int ezcompress(unsigned char* pDest, long* pnDestLen, const unsigned char* pSrc, long nSrcLen) {
        z_stream stream;
        int err;

        int nExtraChunks;
        uInt destlen;

        stream.next_in = (Bytef*)pSrc;
        stream.avail_in = (uInt)nSrcLen;
        destlen = (uInt)*pnDestLen;
        if ((uLong)destlen != (uLong)*pnDestLen) return Z_BUF_ERROR;
        stream.zalloc = (alloc_func)0;
        stream.zfree = (free_func)0;
        stream.opaque = (voidpf)0;

        err = deflateInit(&stream, Z_DEFAULT_COMPRESSION);
        if (err != Z_OK) return err;
        nExtraChunks = 0;
        do {
            stream.next_out = pDest;
            stream.avail_out = destlen;
            err = deflate(&stream, Z_FINISH);
            if (err == Z_STREAM_END )
                break;
            if (err != Z_OK) {
                deflateEnd(&stream);
                return err;
            }
            nExtraChunks += 1;
        } while (stream.avail_out == 0);

        *pnDestLen = stream.total_out;

        err = deflateEnd(&stream);
        if (err != Z_OK) return err;

        return nExtraChunks ? Z_BUF_ERROR : Z_OK;
    }

    void Convert(const uint8_t *image_data, int width, int height, int pitch, int flags,
        uint8_t **data, int *data_size) {
        // For 4444 and 565. Ordered dither matrix. looks really surprisingly good on cell phone screens at 4444.
        int dith[16] = {
            1, 9, 3, 11,
            13, 5, 15, 7,
            4, 12, 2, 10,
            16, 8, 14, 6
        };
        if ((flags & ZIM_DITHER) == 0) {
            for (int i = 0; i < 16; i++) { dith[i] = 8; }
        }
        switch (flags & ZIM_FORMAT_MASK) {
        case ZIM_RGBA8888: {
                *data_size = width * height * 4;
                *data = new uint8_t[width * height * 4];
                for (int y = 0; y < height; y++) {
                    memcpy((*data) + y * width * 4, image_data + y * pitch, width * 4);
                }
                break;
            }
        case ZIM_ETC1: {
            rg_etc1::pack_etc1_block_init();
            rg_etc1::etc1_pack_params params;
            params.m_dithering = false; //(flags & ZIM_ETC1_DITHER) != 0;
            if (flags & ZIM_ETC1_LOW) {
                params.m_quality = rg_etc1::cLowQuality;
            } else if (flags & ZIM_ETC1_MEDIUM) {
                params.m_quality = rg_etc1::cMediumQuality;
            } else {
                params.m_quality = rg_etc1::cHighQuality;
            }

            // Check for power of 2
            if (!IsPowerOf2(width) || !IsPowerOf2(height))
                throw _NException_Normal("Image must have power of 2 dimensions");

            // Convert RGBX to ETC1 before saving.
            int blockw = width/4;
            int blockh = height/4;
            *data_size = blockw * blockh * 8;
            *data = new uint8_t[*data_size];
            for (int y = 0; y < blockh; y++) {
                for (int x = 0; x < blockw; x++) {
                    uint32_t block[16];
                    for (int iy = 0; iy < 4; iy++) {
                        memcpy(block + 4 * iy, image_data + ((y * 4 + iy) * (pitch/4) + x * 4) * 4, 16);
                    }
                    rg_etc1::pack_etc1_block((*data) + (blockw * y + x) * 8, block, params);
                }
            }
            width = blockw * 4;
            height = blockh * 4;
            break;
            }
        case ZIM_RGBA4444: {
                *data_size = width * height * 2;
                *data = new uint8_t[*data_size];
                uint16_t *dst = (uint16_t *)(*data);
                int i = 0;
                for (int y = 0; y < height; y++) {
                    for (int x = 0; x < width; x++) {
                        int dithval = dith[(x&3)+((y&0x3)<<2)] - 8;
                        int r = Clamp16((image_data[i * 4] + dithval) >> 4);
                        int g = Clamp16((image_data[i * 4 + 1] + dithval) >> 4);
                        int b = Clamp16((image_data[i * 4 + 2] + dithval) >> 4);
                        int a = Clamp16((image_data[i * 4 + 3] + dithval) >> 4);	// really dither alpha?
                        // Note: GL_UNSIGNED_SHORT_4_4_4_4, not GL_UNSIGNED_SHORT_4_4_4_4_REV
                        *dst++ = (r << 12) | (g << 8) | (b << 4) | (a << 0);
                        i++;
                    }
                }
                break;
            }
        case ZIM_RGB565:
            {
                *data_size = width * height * 2;
                *data = new uint8_t[*data_size];
                uint16_t *dst = (uint16_t *)(*data);
                int i = 0;
                for (int y = 0; y < height; y++) {
                    for (int x = 0; x < width; x++) {
                        int dithval = dith[(x&3)+((y&0x3)<<2)] - 8;
                        dithval = 0;
                        int r = Clamp32((image_data[i * 4] + dithval/2) >> 3);
                        int g = Clamp64((image_data[i * 4 + 1] + dithval/4) >> 2);
                        int b = Clamp32((image_data[i * 4 + 2] + dithval/2) >> 3);
                        // Note: GL_UNSIGNED_SHORT_5_6_5, not GL_UNSIGNED_SHORT_5_6_5_REV
                        *dst++ = (r << 11) | (g << 5) | (b);
                        i++;
                    }
                }
            }
            break;

        default:
            throw _NException_Normal("Unhandled ZIM format");
        }
    }

    // Deletes the old buffer.
    uint8_t *DownsampleBy2(const uint8_t *image, int width, int height, int pitch) {
        uint8_t *out = new uint8_t[(width/2) * (height/2) * 4];

        int degamma[256];
        int gamma[32768];
        for (int i =0; i < 256; i++) {
            degamma[i] = powf((float)i / 255.0f, 1.0f/2.2f) * 8191.0f;
        }
        for (int i = 0; i < 32768; i++) {
            gamma[i] = powf((float)i / 32764.0f, 2.2f) * 255.0f;
        }

        // Really stupid mipmap downsampling - at least it does gamma though.
        for (int y = 0; y < height; y+=2) {
            for (int x = 0; x < width; x+=2) {
                const uint8_t *topLeft = image + pitch * y + x*4;
                const uint8_t *tr = topLeft + 4;
                const uint8_t *bl = topLeft + pitch;
                const uint8_t *bottomRight = bl + 4;
                uint8_t *d = out + ((y/2) * ((width/2)) + x / 2) * 4;
                for (int c = 0; c < 4; c++) {
                    d[c] = gamma[degamma[topLeft[c]] + degamma[tr[c]] + degamma[bl[c]] + degamma[bottomRight[c]]];
                }
            }
        }
        return out;
    }

    void SaveZIM(const char *filename, int width, int height, int pitch, int flags, const uint8_t *image_data) {
        FILE *f = OpenFile(filename, "wb");
        static const char magic[5] = "ZIMG";
        fwrite(magic, 1, 4, f);
        fwrite(&width, 1, 4, f);
        fwrite(&height, 1, 4, f);
        fwrite(&flags, 1, 4, f);

        int num_levels = 1;
        if (flags & ZIM_HAS_MIPS) {
            num_levels = log2i(width > height ? height : width) + 1;
        }
        for (int i = 0; i < num_levels; i++) {
            uint8_t *data = 0;
            int data_size;
            Convert(image_data, width, height, pitch, flags, &data, &data_size);
            if (flags & ZIM_ZLIB_COMPRESSED) {
                long dest_len = data_size * 2;
                uint8_t *dest = new uint8_t[dest_len];
                if (Z_OK == ezcompress(dest, &dest_len, data, data_size)) {
                    fwrite(dest, 1, dest_len, f);
                }
                else {
                    throw _NException_Normal("Zlib compression failed.");
                }
                delete [] dest;
            } else {
                fwrite(data, 1, data_size, f);
            }
            delete [] data;

            if (i != num_levels - 1) {
                uint8_t *smaller = DownsampleBy2(image_data, width, height, pitch);
                if (i != 0) {
                    delete [] image_data;
                }
                image_data = smaller;
                width /= 2;
                height /= 2;
                if ((flags & ZIM_FORMAT_MASK) == ZIM_ETC1) {
                    if (width < 4) width = 4;
                    if (height < 4) height = 4;
                }
                pitch = width * 4;
            }
        }
        delete [] image_data;
        fclose(f);
    }
}
