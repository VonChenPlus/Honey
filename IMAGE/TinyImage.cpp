#include "IMAGE/TinyImage.h"

#include <string.h>

#include "BASE/HData.h"
#include "IO/FileUtils.h"
#include "IMAGE/TinyPNG.h"
#include "EXTERNALS/rg_etc1/etc1.h"
#include "EXTERNALS/jpge/jpgd.h"

#define CC_GL_ATC_RGB_AMD                                          0x8C92
#define CC_GL_ATC_RGBA_EXPLICIT_ALPHA_AMD                          0x8C93
#define CC_GL_ATC_RGBA_INTERPOLATED_ALPHA_AMD                      0x87EE

namespace IMAGE
{
    IMAGE::PixelFormat convertI8ToFormat(const unsigned char* data, int64 dataLen, IMAGE::PixelFormat format, unsigned char** outData, int64* outDataLen)
    {
        switch (format)
        {
        case IMAGE::PixelFormat::RGBA8888:
            *outDataLen = dataLen*4;
            *outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
            convertI8ToRGBA8888(data, dataLen, *outData);
            break;
        case IMAGE::PixelFormat::RGB888:
            *outDataLen = dataLen*3;
            *outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
            convertI8ToRGB888(data, dataLen, *outData);
            break;
        case IMAGE::PixelFormat::RGB565:
            *outDataLen = dataLen*2;
            *outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
            convertI8ToRGB565(data, dataLen, *outData);
            break;
        case IMAGE::PixelFormat::AI88:
            *outDataLen = dataLen*2;
            *outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
            convertI8ToAI88(data, dataLen, *outData);
            break;
        case IMAGE::PixelFormat::RGBA4444:
            *outDataLen = dataLen*2;
            *outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
            convertI8ToRGBA4444(data, dataLen, *outData);
            break;
        case IMAGE::PixelFormat::RGB5A1:
            *outDataLen = dataLen*2;
            *outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
            convertI8ToRGB5A1(data, dataLen, *outData);
            break;
        default:
            *outData = (unsigned char*)data;
            *outDataLen = dataLen;
            return IMAGE::PixelFormat::I8;
        }

        return format;
    }

    IMAGE::PixelFormat convertAI88ToFormat(const unsigned char* data, int64 dataLen, IMAGE::PixelFormat format, unsigned char** outData, int64* outDataLen)
    {
        switch (format)
        {
        case IMAGE::PixelFormat::RGBA8888:
            *outDataLen = dataLen*2;
            *outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
            convertAI88ToRGBA8888(data, dataLen, *outData);
            break;
        case IMAGE::PixelFormat::RGB888:
            *outDataLen = dataLen/2*3;
            *outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
            convertAI88ToRGB888(data, dataLen, *outData);
            break;
        case IMAGE::PixelFormat::RGB565:
            *outDataLen = dataLen;
            *outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
            convertAI88ToRGB565(data, dataLen, *outData);
            break;
        case IMAGE::PixelFormat::A8:
            *outDataLen = dataLen/2;
            *outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
            convertAI88ToA8(data, dataLen, *outData);
            break;
        case IMAGE::PixelFormat::I8:
            *outDataLen = dataLen/2;
            *outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
            convertAI88ToI8(data, dataLen, *outData);
            break;
        case IMAGE::PixelFormat::RGBA4444:
            *outDataLen = dataLen;
            *outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
            convertAI88ToRGBA4444(data, dataLen, *outData);
            break;
        case IMAGE::PixelFormat::RGB5A1:
            *outDataLen = dataLen;
            *outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
            convertAI88ToRGB5A1(data, dataLen, *outData);
            break;
        default:
            // unsupport convertion or don't need to convert
            *outData = (unsigned char*)data;
            *outDataLen = dataLen;
            return IMAGE::PixelFormat::AI88;
            break;
        }

        return format;
    }

    IMAGE::PixelFormat convertRGB888ToFormat(const unsigned char* data, int64 dataLen, IMAGE::PixelFormat format, unsigned char** outData, int64* outDataLen)
    {
        switch (format)
        {
        case IMAGE::PixelFormat::RGBA8888:
            *outDataLen = dataLen/3*4;
            *outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
            convertRGB888ToRGBA8888(data, dataLen, *outData);
            break;
        case IMAGE::PixelFormat::RGB565:
            *outDataLen = dataLen/3*2;
            *outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
            convertRGB888ToRGB565(data, dataLen, *outData);
            break;
        case IMAGE::PixelFormat::I8:
            *outDataLen = dataLen/3;
            *outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
            convertRGB888ToI8(data, dataLen, *outData);
            break;
        case IMAGE::PixelFormat::AI88:
            *outDataLen = dataLen/3*2;
            *outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
            convertRGB888ToAI88(data, dataLen, *outData);
            break;
        case IMAGE::PixelFormat::RGBA4444:
            *outDataLen = dataLen/3*2;
            *outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
            convertRGB888ToRGBA4444(data, dataLen, *outData);
            break;
        case IMAGE::PixelFormat::RGB5A1:
            *outDataLen = dataLen;
            *outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
            convertRGB888ToRGB5A1(data, dataLen, *outData);
            break;
        default:
            *outData = (unsigned char*)data;
            *outDataLen = dataLen;
            return IMAGE::PixelFormat::RGB888;
        }
        return format;
    }

    IMAGE::PixelFormat convertRGBA8888ToFormat(const unsigned char* data, int64 dataLen, IMAGE::PixelFormat format, unsigned char** outData, int64* outDataLen)
    {
        switch (format)
        {
        case IMAGE::PixelFormat::RGB888:
            *outDataLen = dataLen/4*3;
            *outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
            convertRGBA8888ToRGB888(data, dataLen, *outData);
            break;
        case IMAGE::PixelFormat::RGB565:
            *outDataLen = dataLen/2;
            *outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
            convertRGBA8888ToRGB565(data, dataLen, *outData);
            break;
        case IMAGE::PixelFormat::A8:
            *outDataLen = dataLen/4;
            *outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
            convertRGBA8888ToA8(data, dataLen, *outData);
            break;
        case IMAGE::PixelFormat::I8:
            *outDataLen = dataLen/4;
            *outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
            convertRGBA8888ToI8(data, dataLen, *outData);
            break;
        case IMAGE::PixelFormat::AI88:
            *outDataLen = dataLen/2;
            *outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
            convertRGBA8888ToAI88(data, dataLen, *outData);
            break;
        case IMAGE::PixelFormat::RGBA4444:
            *outDataLen = dataLen/2;
            *outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
            convertRGBA8888ToRGBA4444(data, dataLen, *outData);
            break;
        case IMAGE::PixelFormat::RGB5A1:
            *outDataLen = dataLen/2;
            *outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
            convertRGBA8888ToRGB5A1(data, dataLen, *outData);
            break;
        default:
            *outData = (unsigned char*)data;
            *outDataLen = dataLen;
            return IMAGE::PixelFormat::RGBA8888;
        }

        return format;
    }

    IMAGE::PixelFormat convertDataToFormat(const unsigned char* data, int64 dataLen, IMAGE::PixelFormat originFormat, IMAGE::PixelFormat format, unsigned char** outData, int64* outDataLen)
    {
        // don't need to convert
        if (format == originFormat || format == IMAGE::PixelFormat::AUTO)
        {
            *outData = (unsigned char*)data;
            *outDataLen = dataLen;
            return originFormat;
        }

        switch (originFormat)
        {
        case IMAGE::PixelFormat::I8:
            return convertI8ToFormat(data, dataLen, format, outData, outDataLen);
        case IMAGE::PixelFormat::AI88:
            return convertAI88ToFormat(data, dataLen, format, outData, outDataLen);
        case IMAGE::PixelFormat::RGB888:
            return convertRGB888ToFormat(data, dataLen, format, outData, outDataLen);
        case IMAGE::PixelFormat::RGBA8888:
            return convertRGBA8888ToFormat(data, dataLen, format, outData, outDataLen);
        default:
            *outData = (unsigned char*)data;
            *outDataLen = dataLen;
            return originFormat;
        }
    }

    // IIIIIIII -> RRRRRRRRGGGGGGGGGBBBBBBBB
    void convertI8ToRGB888(const unsigned char* data, int64 dataLen, unsigned char* outData)
    {
        for (int64 i=0; i < dataLen; ++i)
        {
            *outData++ = data[i];     //R
            *outData++ = data[i];     //G
            *outData++ = data[i];     //B
        }
    }

    // IIIIIIIIAAAAAAAA -> RRRRRRRRGGGGGGGGBBBBBBBB
    void convertAI88ToRGB888(const unsigned char* data, int64 dataLen, unsigned char* outData)
    {
        for (int64 i = 0, l = dataLen - 1; i < l; i += 2)
        {
            *outData++ = data[i];     //R
            *outData++ = data[i];     //G
            *outData++ = data[i];     //B
        }
    }

    // IIIIIIII -> RRRRRRRRGGGGGGGGGBBBBBBBBAAAAAAAA
    void convertI8ToRGBA8888(const unsigned char* data, int64 dataLen, unsigned char* outData)
    {
        for (int64 i = 0; i < dataLen; ++i)
        {
            *outData++ = data[i];     //R
            *outData++ = data[i];     //G
            *outData++ = data[i];     //B
            *outData++ = 0xFF;        //A
        }
    }

    // IIIIIIIIAAAAAAAA -> RRRRRRRRGGGGGGGGBBBBBBBBAAAAAAAA
    void convertAI88ToRGBA8888(const unsigned char* data, int64 dataLen, unsigned char* outData)
    {
        for (int64 i = 0, l = dataLen - 1; i < l; i += 2)
        {
            *outData++ = data[i];     //R
            *outData++ = data[i];     //G
            *outData++ = data[i];     //B
            *outData++ = data[i + 1]; //A
        }
    }

    // IIIIIIII -> RRRRRGGGGGGBBBBB
    void convertI8ToRGB565(const unsigned char* data, int64 dataLen, unsigned char* outData)
    {
        unsigned short* out16 = (unsigned short*)outData;
        for (int i = 0; i < dataLen; ++i)
        {
            *out16++ = (data[i] & 0x00F8) << 8    //R
                | (data[i] & 0x00FC) << 3         //G
                | (data[i] & 0x00F8) >> 3;        //B
        }
    }

    // IIIIIIIIAAAAAAAA -> RRRRRGGGGGGBBBBB
    void convertAI88ToRGB565(const unsigned char* data, int64 dataLen, unsigned char* outData)
    {
        unsigned short* out16 = (unsigned short*)outData;
        for (int64 i = 0, l = dataLen - 1; i < l; i += 2)
        {
            *out16++ = (data[i] & 0x00F8) << 8    //R
                | (data[i] & 0x00FC) << 3         //G
                | (data[i] & 0x00F8) >> 3;        //B
        }
    }

    // IIIIIIII -> RRRRGGGGBBBBAAAA
    void convertI8ToRGBA4444(const unsigned char* data, int64 dataLen, unsigned char* outData)
    {
        unsigned short* out16 = (unsigned short*)outData;
        for (int64 i = 0; i < dataLen; ++i)
        {
            *out16++ = (data[i] & 0x00F0) << 8    //R
            | (data[i] & 0x00F0) << 4             //G
            | (data[i] & 0x00F0)                  //B
            | 0x000F;                             //A
        }
    }

    // IIIIIIIIAAAAAAAA -> RRRRGGGGBBBBAAAA
    void convertAI88ToRGBA4444(const unsigned char* data, int64 dataLen, unsigned char* outData)
    {
        unsigned short* out16 = (unsigned short*)outData;
        for (int64 i = 0, l = dataLen - 1; i < l; i += 2)
        {
            *out16++ = (data[i] & 0x00F0) << 8    //R
            | (data[i] & 0x00F0) << 4             //G
            | (data[i] & 0x00F0)                  //B
            | (data[i+1] & 0x00F0) >> 4;          //A
        }
    }

    // IIIIIIII -> RRRRRGGGGGBBBBBA
    void convertI8ToRGB5A1(const unsigned char* data, int64 dataLen, unsigned char* outData)
    {
        unsigned short* out16 = (unsigned short*)outData;
        for (int i = 0; i < dataLen; ++i)
        {
            *out16++ = (data[i] & 0x00F8) << 8    //R
                | (data[i] & 0x00F8) << 3         //G
                | (data[i] & 0x00F8) >> 2         //B
                | 0x0001;                         //A
        }
    }

    // IIIIIIIIAAAAAAAA -> RRRRRGGGGGBBBBBA
    void convertAI88ToRGB5A1(const unsigned char* data, int64 dataLen, unsigned char* outData)
    {
        unsigned short* out16 = (unsigned short*)outData;
        for (int64 i = 0, l = dataLen - 1; i < l; i += 2)
        {
            *out16++ = (data[i] & 0x00F8) << 8    //R
                | (data[i] & 0x00F8) << 3         //G
                | (data[i] & 0x00F8) >> 2         //B
                | (data[i + 1] & 0x0080) >> 7;    //A
        }
    }

    // IIIIIIII -> IIIIIIIIAAAAAAAA
    void convertI8ToAI88(const unsigned char* data, int64 dataLen, unsigned char* outData)
    {
        unsigned short* out16 = (unsigned short*)outData;
        for (int64 i = 0; i < dataLen; ++i)
        {
            *out16++ = 0xFF00     //A
            | data[i];            //I
        }
    }

    // IIIIIIIIAAAAAAAA -> AAAAAAAA
    void convertAI88ToA8(const unsigned char* data, int64 dataLen, unsigned char* outData)
    {
        for (int64 i = 1; i < dataLen; i += 2)
        {
            *outData++ = data[i]; //A
        }
    }

    // IIIIIIIIAAAAAAAA -> IIIIIIII
    void convertAI88ToI8(const unsigned char* data, int64 dataLen, unsigned char* outData)
    {
        for (int64 i = 0, l = dataLen - 1; i < l; i += 2)
        {
            *outData++ = data[i]; //R
        }
    }

    // RRRRRRRRGGGGGGGGBBBBBBBB -> RRRRRRRRGGGGGGGGBBBBBBBBAAAAAAAA
    void convertRGB888ToRGBA8888(const unsigned char* data, int64 dataLen, unsigned char* outData)
    {
        for (int64 i = 0, l = dataLen - 2; i < l; i += 3)
        {
            *outData++ = data[i];         //R
            *outData++ = data[i + 1];     //G
            *outData++ = data[i + 2];     //B
            *outData++ = 0xFF;            //A
        }
    }

    // RRRRRRRRGGGGGGGGBBBBBBBBAAAAAAAA -> RRRRRRRRGGGGGGGGBBBBBBBB
    void convertRGBA8888ToRGB888(const unsigned char* data, int64 dataLen, unsigned char* outData)
    {
        for (int64 i = 0, l = dataLen - 3; i < l; i += 4)
        {
            *outData++ = data[i];         //R
            *outData++ = data[i + 1];     //G
            *outData++ = data[i + 2];     //B
        }
    }

    // RRRRRRRRGGGGGGGGBBBBBBBB -> RRRRRGGGGGGBBBBB
    void convertRGB888ToRGB565(const unsigned char* data, int64 dataLen, unsigned char* outData)
    {
        unsigned short* out16 = (unsigned short*)outData;
        for (int64 i = 0, l = dataLen - 2; i < l; i += 3)
        {
            *out16++ = (data[i] & 0x00F8) << 8    //R
                | (data[i + 1] & 0x00FC) << 3     //G
                | (data[i + 2] & 0x00F8) >> 3;    //B
        }
    }

    // RRRRRRRRGGGGGGGGBBBBBBBBAAAAAAAA -> RRRRRGGGGGGBBBBB
    void convertRGBA8888ToRGB565(const unsigned char* data, int64 dataLen, unsigned char* outData)
    {
        unsigned short* out16 = (unsigned short*)outData;
        for (int64 i = 0, l = dataLen - 3; i < l; i += 4)
        {
            *out16++ = (data[i] & 0x00F8) << 8    //R
                | (data[i + 1] & 0x00FC) << 3     //G
                | (data[i + 2] & 0x00F8) >> 3;    //B
        }
    }

    // RRRRRRRRGGGGGGGGBBBBBBBB -> IIIIIIII
    void convertRGB888ToI8(const unsigned char* data, int64 dataLen, unsigned char* outData)
    {
        for (int64 i = 0, l = dataLen - 2; i < l; i += 3)
        {
            *outData++ = (data[i] * 299 + data[i + 1] * 587 + data[i + 2] * 114 + 500) / 1000;  //I =  (R*299 + G*587 + B*114 + 500) / 1000
        }
    }

    // RRRRRRRRGGGGGGGGBBBBBBBBAAAAAAAA -> IIIIIIII
    void convertRGBA8888ToI8(const unsigned char* data, int64 dataLen, unsigned char* outData)
    {
        for (int64 i = 0, l = dataLen - 3; i < l; i += 4)
        {
            *outData++ = (data[i] * 299 + data[i + 1] * 587 + data[i + 2] * 114 + 500) / 1000;  //I =  (R*299 + G*587 + B*114 + 500) / 1000
        }
    }

    // RRRRRRRRGGGGGGGGBBBBBBBBAAAAAAAA -> AAAAAAAA
    void convertRGBA8888ToA8(const unsigned char* data, int64 dataLen, unsigned char* outData)
    {
        for (int64 i = 0, l = dataLen -3; i < l; i += 4)
        {
            *outData++ = data[i + 3]; //A
        }
    }

    // RRRRRRRRGGGGGGGGBBBBBBBB -> IIIIIIIIAAAAAAAA
    void convertRGB888ToAI88(const unsigned char* data, int64 dataLen, unsigned char* outData)
    {
        for (int64 i = 0, l = dataLen - 2; i < l; i += 3)
        {
            *outData++ = (data[i] * 299 + data[i + 1] * 587 + data[i + 2] * 114 + 500) / 1000;  //I =  (R*299 + G*587 + B*114 + 500) / 1000
            *outData++ = 0xFF;
        }
    }


    // RRRRRRRRGGGGGGGGBBBBBBBBAAAAAAAA -> IIIIIIIIAAAAAAAA
    void convertRGBA8888ToAI88(const unsigned char* data, int64 dataLen, unsigned char* outData)
    {
        for (int64 i = 0, l = dataLen - 3; i < l; i += 4)
        {
            *outData++ = (data[i] * 299 + data[i + 1] * 587 + data[i + 2] * 114 + 500) / 1000;  //I =  (R*299 + G*587 + B*114 + 500) / 1000
            *outData++ = data[i + 3];
        }
    }

    // RRRRRRRRGGGGGGGGBBBBBBBB -> RRRRGGGGBBBBAAAA
    void convertRGB888ToRGBA4444(const unsigned char* data, int64 dataLen, unsigned char* outData)
    {
        unsigned short* out16 = (unsigned short*)outData;
        for (int64 i = 0, l = dataLen - 2; i < l; i += 3)
        {
            *out16++ = ((data[i] & 0x00F0) << 8           //R
                        | (data[i + 1] & 0x00F0) << 4     //G
                        | (data[i + 2] & 0xF0)            //B
                        |  0x0F);                         //A
        }
    }

    // RRRRRRRRGGGGGGGGBBBBBBBBAAAAAAAA -> RRRRGGGGBBBBAAAA
    void convertRGBA8888ToRGBA4444(const unsigned char* data, int64 dataLen, unsigned char* outData)
    {
        unsigned short* out16 = (unsigned short*)outData;
        for (int64 i = 0, l = dataLen - 3; i < l; i += 4)
        {
            *out16++ = (data[i] & 0x00F0) << 8    //R
            | (data[i + 1] & 0x00F0) << 4         //G
            | (data[i + 2] & 0xF0)                //B
            |  (data[i + 3] & 0xF0) >> 4;         //A
        }
    }

    // RRRRRRRRGGGGGGGGBBBBBBBB -> RRRRRGGGGGBBBBBA
    void convertRGB888ToRGB5A1(const unsigned char* data, int64 dataLen, unsigned char* outData)
    {
        unsigned short* out16 = (unsigned short*)outData;
        for (int64 i = 0, l = dataLen - 2; i < l; i += 3)
        {
            *out16++ = (data[i] & 0x00F8) << 8    //R
                | (data[i + 1] & 0x00F8) << 3     //G
                | (data[i + 2] & 0x00F8) >> 2     //B
                |  0x01;                          //A
        }
    }

    // RRRRRRRRGGGGGGGGBBBBBBBB -> RRRRRGGGGGBBBBBA
    void convertRGBA8888ToRGB5A1(const unsigned char* data, int64 dataLen, unsigned char* outData)
    {
        unsigned short* out16 = (unsigned short*)outData;
        for (int64 i = 0, l = dataLen - 2; i < l; i += 4)
        {
            *out16++ = (data[i] & 0x00F8) << 8    //R
                | (data[i + 1] & 0x00F8) << 3     //G
                | (data[i + 2] & 0x00F8) >> 2     //B
                |  (data[i + 3] & 0x0080) >> 7;   //A
        }
    }

    static bool _PVRHaveAlphaPremultiplied = false;
    // premultiply alpha, or the effect will wrong when want to use other pixel format in Texture2D,
    // such as RGB888, RGB5A1
    #define RGB_PREMULTIPLY_ALPHA(vr, vg, vb, va) \
        (unsigned)(((unsigned)((unsigned char)(vr) * ((unsigned char)(va) + 1)) >> 8) | \
        ((unsigned)((unsigned char)(vg) * ((unsigned char)(va) + 1) >> 8) << 8) | \
        ((unsigned)((unsigned char)(vb) * ((unsigned char)(va) + 1) >> 8) << 16) | \
        ((unsigned)(unsigned char)(va) << 24))

    //The PixpelFormat corresponding information
    const PixelFormatInfoMap _pixelFormatInfoTables(TexturePixelFormatInfoTablesValue,
                                                                         TexturePixelFormatInfoTablesValue + sizeof(TexturePixelFormatInfoTablesValue) / sizeof(TexturePixelFormatInfoTablesValue[0]));

    TinyImage::TinyImage()
        : data_(nullptr)
        , dataLen_(0)
        , width_(0)
        , height_(0)
        , fileType_(Format::UNKNOWN)
        , renderFormat_(PixelFormat::NONE)
        , hasPremultipliedAlpha_(true) {

    }

    TinyImage::~TinyImage() {
        SAFE_FREE(data_);
    }

    bool TinyImage::initWithImageFile(const std::string& path) {
        bool ret = false;
        filePath_ = IO::FileUtils::getInstance().fullPathForFilename(path);

        HData data = IO::FileUtils::getInstance().getDataFromFile(filePath_);

        if (!data.isNull()) {
            ret = initWithImageData((const unsigned char *)data.getBytes(), data.getSize());
        }

        return ret;
    }

    bool TinyImage::initWithImageFileThreadSafe(const std::string& fullpath) {
        bool ret = false;
        filePath_ = fullpath;

        HData data = IO::FileUtils::getInstance().getDataFromFile(fullpath);

        if (!data.isNull()) {
            ret = initWithImageData((const unsigned char *)data.getBytes(), data.getSize());
        }

        return ret;
    }

    bool TinyImage::initWithImageData(const unsigned char *data, int64 dataLen) {
        bool ret = false;

        do {
            if (! data || dataLen <= 0) break;

            unsigned char* unpackedData = nullptr;
            int64 unpackedLen = 0;

            unpackedData = const_cast<unsigned char*>(data);
            unpackedLen = dataLen;

            fileType_ = detectFormat(unpackedData, unpackedLen);

            switch (fileType_) {
            case Format::PNG:
                ret = initWithPngData(unpackedData, unpackedLen);
                break;
            case Format::JPG:
                ret = initWithJpgData(unpackedData, unpackedLen);
                break;
            case Format::ETC:
                ret = initWithETCData(unpackedData, unpackedLen);
                break;

            default:
                ;
            }

            if(unpackedData != data) {
                free(unpackedData);
            }
        } while (0);

        return ret;
    }

    bool TinyImage::isPng(const unsigned char *data, int64 dataLen) {
        if (dataLen <= 8) {
            return false;
        }

        static unsigned char PNG_SIGNATURE[] = {0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a};

        return memcmp(PNG_SIGNATURE, data, sizeof(PNG_SIGNATURE)) == 0;
    }

    bool TinyImage::isEtc(const unsigned char * data, int64) {
        return etc1_pkm_is_valid((etc1_byte*)data) ? true : false;
    }

    bool TinyImage::isJpg(const unsigned char * data, int64 dataLen) {
        if (dataLen <= 4) {
            return false;
        }

        static unsigned char JPG_SOI[] = {0xFF, 0xD8};

        return memcmp(data, JPG_SOI, 2) == 0;
    }

    TinyImage::Format TinyImage::detectFormat(const unsigned char * data, int64 dataLen) {
        if (isPng(data, dataLen)) {
            return Format::PNG;
        }
        else if (isJpg(data, dataLen)) {
            return Format::JPG;
        }
        else if (isEtc(data, dataLen)) {
            return Format::ETC;
        }
        else {
            return Format::UNKNOWN;
        }
    }

    int TinyImage::getBitPerPixel() {
        return getPixelFormatInfoMap().at(renderFormat_).bpp;
    }

    bool TinyImage::hasAlpha() {
        return getPixelFormatInfoMap().at(renderFormat_).alpha;
    }

    bool TinyImage::isCompressed() {
        return getPixelFormatInfoMap().at(renderFormat_).compressed;
    }

    bool TinyImage::initWithJpgData(const unsigned char * data, int64 dataLen) {
        int actual_components = 0;
        data_ = (unsigned char*)jpgd::decompress_jpeg_image_from_memory((const unsigned char*)data, (int)dataLen, &width_, &height_, &actual_components, 4);
        renderFormat_ = PixelFormat::RGB888;

        return true;
    }

    bool TinyImage::initWithPngData(const unsigned char *data, int64 dataLen) {
        int color_type;
        PNGLoadPtr((const unsigned char*)data, dataLen, &width_, &height_, &color_type, (unsigned char**)&data_, (int *)&dataLen_);

        switch (color_type)
        {
        case PNG_COLOR_TYPE_GRAY:
            renderFormat_ = PixelFormat::I8;
            break;
        case PNG_COLOR_TYPE_GRAY_ALPHA:
            renderFormat_ = PixelFormat::AI88;
            break;
        case PNG_COLOR_TYPE_RGB:
            renderFormat_ = PixelFormat::RGB888;
            break;
        case PNG_COLOR_TYPE_RGB_ALPHA:
            renderFormat_ = PixelFormat::RGBA8888;
            break;
        default:
            break;
        }

        // premultiplied alpha for RGBA8888
        if (color_type == PNG_COLOR_TYPE_RGB_ALPHA) {
            premultipliedAlpha();
        }
        else {
            hasPremultipliedAlpha_ = false;
        }

        return true;
    }

    bool TinyImage::initWithETCData(const unsigned char * data, int64) {
        const etc1_byte* header = static_cast<const etc1_byte*>(data);

        //check the data
        if (! etc1_pkm_is_valid(header)) {
            return  false;
        }

        width_ = etc1_pkm_get_width(header);
        height_ = etc1_pkm_get_height(header);

        if (0 == width_ || 0 == height_) {
            return false;
        }

         //if it is not gles or device do not support ETC, decode texture by software
        int bytePerPixel = 3;
        unsigned int stride = width_ * bytePerPixel;
        renderFormat_ = PixelFormat::RGB888;

        dataLen_ =  width_ * height_ * bytePerPixel;
        data_ = static_cast<unsigned char*>(malloc(dataLen_ * sizeof(unsigned char)));

        if (etc1_decode_image(static_cast<const unsigned char*>(data) + ETC_PKM_HEADER_SIZE, static_cast<etc1_byte*>(data_), width_, height_, bytePerPixel, stride) != 0) {
            dataLen_ = 0;
            if (data_ != nullptr) {
                free(data_);
            }
            return false;
        }

        return true;
    }

    bool TinyImage::initWithRawData(const unsigned char * data, int64, int width, int height, int, bool preMulti) {
        bool ret = false;
        do
        {
            if(0 == width || 0 == height) break;

            height_   = height;
            width_    = width;
            hasPremultipliedAlpha_ = preMulti;
            renderFormat_ = PixelFormat::RGBA8888;

            // only RGBA8888 supported
            int bytesPerComponent = 4;
            dataLen_ = height * width * bytesPerComponent;
            data_ = static_cast<unsigned char*>(malloc(dataLen_ * sizeof(unsigned char)));
            if(! data_) break;
            memcpy(data_, data, dataLen_);

            ret = true;
        } while (0);

        return ret;
    }

    void TinyImage::premultipliedAlpha() {
        unsigned int* fourBytes = (unsigned int*)data_;
        for(int i = 0; i < width_ * height_; i++) {
            unsigned char* p = data_ + i * 4;
            fourBytes[i] = RGB_PREMULTIPLY_ALPHA(p[0], p[1], p[2], p[3]);
        }

        hasPremultipliedAlpha_ = true;
    }

    void TinyImage::setPVRImagesHavePremultipliedAlpha(bool haveAlphaPremultiplied) {
        _PVRHaveAlphaPremultiplied = haveAlphaPremultiplied;
    }

    const PixelFormatInfoMap& TinyImage::getPixelFormatInfoMap() {
        return _pixelFormatInfoTables;
    }
}

