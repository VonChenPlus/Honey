#include "ImageConvert.h"

namespace IMAGE
{
    ImageFormat convertI8ToFormat(const unsigned char* data, uint64 dataLen, ImageFormat format, unsigned char** outData, uint64* outDataLen)
    {
        switch (format)
        {
        case ImageFormat::RGBA8888:
            *outDataLen = dataLen*4;
            *outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
            convertI8ToRGBA8888(data, dataLen, *outData);
            break;
        case ImageFormat::RGB888:
            *outDataLen = dataLen*3;
            *outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
            convertI8ToRGB888(data, dataLen, *outData);
            break;
        case ImageFormat::RGB565:
            *outDataLen = dataLen*2;
            *outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
            convertI8ToRGB565(data, dataLen, *outData);
            break;
        case ImageFormat::AI88:
            *outDataLen = dataLen*2;
            *outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
            convertI8ToAI88(data, dataLen, *outData);
            break;
        case ImageFormat::RGBA4444:
            *outDataLen = dataLen*2;
            *outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
            convertI8ToRGBA4444(data, dataLen, *outData);
            break;
        case ImageFormat::RGB5A1:
            *outDataLen = dataLen*2;
            *outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
            convertI8ToRGB5A1(data, dataLen, *outData);
            break;
        default:
            *outData = (unsigned char*)data;
            *outDataLen = dataLen;
            return ImageFormat::I8;
        }

        return format;
    }

    ImageFormat convertAI88ToFormat(const unsigned char* data, uint64 dataLen, ImageFormat format, unsigned char** outData, uint64* outDataLen)
    {
        switch (format)
        {
        case ImageFormat::RGBA8888:
            *outDataLen = dataLen*2;
            *outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
            convertAI88ToRGBA8888(data, dataLen, *outData);
            break;
        case ImageFormat::RGB888:
            *outDataLen = dataLen/2*3;
            *outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
            convertAI88ToRGB888(data, dataLen, *outData);
            break;
        case ImageFormat::RGB565:
            *outDataLen = dataLen;
            *outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
            convertAI88ToRGB565(data, dataLen, *outData);
            break;
        case ImageFormat::A8:
            *outDataLen = dataLen/2;
            *outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
            convertAI88ToA8(data, dataLen, *outData);
            break;
        case ImageFormat::I8:
            *outDataLen = dataLen/2;
            *outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
            convertAI88ToI8(data, dataLen, *outData);
            break;
        case ImageFormat::RGBA4444:
            *outDataLen = dataLen;
            *outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
            convertAI88ToRGBA4444(data, dataLen, *outData);
            break;
        case ImageFormat::RGB5A1:
            *outDataLen = dataLen;
            *outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
            convertAI88ToRGB5A1(data, dataLen, *outData);
            break;
        default:
            // unsupport convertion or don't need to convert
            *outData = (unsigned char*)data;
            *outDataLen = dataLen;
            return ImageFormat::AI88;
            break;
        }

        return format;
    }

    ImageFormat convertRGB888ToFormat(const unsigned char* data, uint64 dataLen, ImageFormat format, unsigned char** outData, uint64* outDataLen)
    {
        switch (format)
        {
        case ImageFormat::RGBA8888:
            *outDataLen = dataLen/3*4;
            *outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
            convertRGB888ToRGBA8888(data, dataLen, *outData);
            break;
        case ImageFormat::RGB565:
            *outDataLen = dataLen/3*2;
            *outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
            convertRGB888ToRGB565(data, dataLen, *outData);
            break;
        case ImageFormat::I8:
            *outDataLen = dataLen/3;
            *outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
            convertRGB888ToI8(data, dataLen, *outData);
            break;
        case ImageFormat::AI88:
            *outDataLen = dataLen/3*2;
            *outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
            convertRGB888ToAI88(data, dataLen, *outData);
            break;
        case ImageFormat::RGBA4444:
            *outDataLen = dataLen/3*2;
            *outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
            convertRGB888ToRGBA4444(data, dataLen, *outData);
            break;
        case ImageFormat::RGB5A1:
            *outDataLen = dataLen;
            *outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
            convertRGB888ToRGB5A1(data, dataLen, *outData);
            break;
        default:
            *outData = (unsigned char*)data;
            *outDataLen = dataLen;
            return ImageFormat::RGB888;
        }
        return format;
    }

    ImageFormat convertRGBA8888ToFormat(const unsigned char* data, uint64 dataLen, ImageFormat format, unsigned char** outData, uint64* outDataLen)
    {
        switch (format)
        {
        case ImageFormat::RGB888:
            *outDataLen = dataLen/4*3;
            *outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
            convertRGBA8888ToRGB888(data, dataLen, *outData);
            break;
        case ImageFormat::RGB565:
            *outDataLen = dataLen/2;
            *outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
            convertRGBA8888ToRGB565(data, dataLen, *outData);
            break;
        case ImageFormat::A8:
            *outDataLen = dataLen/4;
            *outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
            convertRGBA8888ToA8(data, dataLen, *outData);
            break;
        case ImageFormat::I8:
            *outDataLen = dataLen/4;
            *outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
            convertRGBA8888ToI8(data, dataLen, *outData);
            break;
        case ImageFormat::AI88:
            *outDataLen = dataLen/2;
            *outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
            convertRGBA8888ToAI88(data, dataLen, *outData);
            break;
        case ImageFormat::RGBA4444:
            *outDataLen = dataLen/2;
            *outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
            convertRGBA8888ToRGBA4444(data, dataLen, *outData);
            break;
        case ImageFormat::RGB5A1:
            *outDataLen = dataLen/2;
            *outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
            convertRGBA8888ToRGB5A1(data, dataLen, *outData);
            break;
        default:
            *outData = (unsigned char*)data;
            *outDataLen = dataLen;
            return ImageFormat::RGBA8888;
        }

        return format;
    }

    ImageFormat convertDataToFormat(const unsigned char* data, uint64 dataLen, ImageFormat originFormat, ImageFormat format, unsigned char** outData, uint64* outDataLen)
    {
        // don't need to convert
        if (format == originFormat || format == ImageFormat::AUTO)
        {
            *outData = (unsigned char*)data;
            *outDataLen = dataLen;
            return originFormat;
        }

        switch (originFormat)
        {
        case ImageFormat::I8:
            return convertI8ToFormat(data, dataLen, format, outData, outDataLen);
        case ImageFormat::AI88:
            return convertAI88ToFormat(data, dataLen, format, outData, outDataLen);
        case ImageFormat::RGB888:
            return convertRGB888ToFormat(data, dataLen, format, outData, outDataLen);
        case ImageFormat::RGBA8888:
            return convertRGBA8888ToFormat(data, dataLen, format, outData, outDataLen);
        default:
            *outData = (unsigned char*)data;
            *outDataLen = dataLen;
            return originFormat;
        }
    }

    // IIIIIIII -> RRRRRRRRGGGGGGGGGBBBBBBBB
    void convertI8ToRGB888(const unsigned char* data, uint64 dataLen, unsigned char* outData)
    {
        for (uint64 i=0; i < dataLen; ++i)
        {
            *outData++ = data[i];     //R
            *outData++ = data[i];     //G
            *outData++ = data[i];     //B
        }
    }

    // IIIIIIIIAAAAAAAA -> RRRRRRRRGGGGGGGGBBBBBBBB
    void convertAI88ToRGB888(const unsigned char* data, uint64 dataLen, unsigned char* outData)
    {
        for (uint64 i = 0, l = dataLen - 1; i < l; i += 2)
        {
            *outData++ = data[i];     //R
            *outData++ = data[i];     //G
            *outData++ = data[i];     //B
        }
    }

    // IIIIIIII -> RRRRRRRRGGGGGGGGGBBBBBBBBAAAAAAAA
    void convertI8ToRGBA8888(const unsigned char* data, uint64 dataLen, unsigned char* outData)
    {
        for (uint64 i = 0; i < dataLen; ++i)
        {
            *outData++ = data[i];     //R
            *outData++ = data[i];     //G
            *outData++ = data[i];     //B
            *outData++ = 0xFF;        //A
        }
    }

    // IIIIIIIIAAAAAAAA -> RRRRRRRRGGGGGGGGBBBBBBBBAAAAAAAA
    void convertAI88ToRGBA8888(const unsigned char* data, uint64 dataLen, unsigned char* outData)
    {
        for (uint64 i = 0, l = dataLen - 1; i < l; i += 2)
        {
            *outData++ = data[i];     //R
            *outData++ = data[i];     //G
            *outData++ = data[i];     //B
            *outData++ = data[i + 1]; //A
        }
    }

    // IIIIIIII -> RRRRRGGGGGGBBBBB
    void convertI8ToRGB565(const unsigned char* data, uint64 dataLen, unsigned char* outData)
    {
        unsigned short* out16 = (unsigned short*)outData;
        for (uint64 i = 0; i < dataLen; ++i)
        {
            *out16++ = (data[i] & 0x00F8) << 8    //R
                | (data[i] & 0x00FC) << 3         //G
                | (data[i] & 0x00F8) >> 3;        //B
        }
    }

    // IIIIIIIIAAAAAAAA -> RRRRRGGGGGGBBBBB
    void convertAI88ToRGB565(const unsigned char* data, uint64 dataLen, unsigned char* outData)
    {
        unsigned short* out16 = (unsigned short*)outData;
        for (uint64 i = 0, l = dataLen - 1; i < l; i += 2)
        {
            *out16++ = (data[i] & 0x00F8) << 8    //R
                | (data[i] & 0x00FC) << 3         //G
                | (data[i] & 0x00F8) >> 3;        //B
        }
    }

    // IIIIIIII -> RRRRGGGGBBBBAAAA
    void convertI8ToRGBA4444(const unsigned char* data, uint64 dataLen, unsigned char* outData)
    {
        unsigned short* out16 = (unsigned short*)outData;
        for (uint64 i = 0; i < dataLen; ++i)
        {
            *out16++ = (data[i] & 0x00F0) << 8    //R
            | (data[i] & 0x00F0) << 4             //G
            | (data[i] & 0x00F0)                  //B
            | 0x000F;                             //A
        }
    }

    // IIIIIIIIAAAAAAAA -> RRRRGGGGBBBBAAAA
    void convertAI88ToRGBA4444(const unsigned char* data, uint64 dataLen, unsigned char* outData)
    {
        unsigned short* out16 = (unsigned short*)outData;
        for (uint64 i = 0, l = dataLen - 1; i < l; i += 2)
        {
            *out16++ = (data[i] & 0x00F0) << 8    //R
            | (data[i] & 0x00F0) << 4             //G
            | (data[i] & 0x00F0)                  //B
            | (data[i+1] & 0x00F0) >> 4;          //A
        }
    }

    // IIIIIIII -> RRRRRGGGGGBBBBBA
    void convertI8ToRGB5A1(const unsigned char* data, uint64 dataLen, unsigned char* outData)
    {
        unsigned short* out16 = (unsigned short*)outData;
        for (uint64 i = 0; i < dataLen; ++i)
        {
            *out16++ = (data[i] & 0x00F8) << 8    //R
                | (data[i] & 0x00F8) << 3         //G
                | (data[i] & 0x00F8) >> 2         //B
                | 0x0001;                         //A
        }
    }

    // IIIIIIIIAAAAAAAA -> RRRRRGGGGGBBBBBA
    void convertAI88ToRGB5A1(const unsigned char* data, uint64 dataLen, unsigned char* outData)
    {
        unsigned short* out16 = (unsigned short*)outData;
        for (uint64 i = 0, l = dataLen - 1; i < l; i += 2)
        {
            *out16++ = (data[i] & 0x00F8) << 8    //R
                | (data[i] & 0x00F8) << 3         //G
                | (data[i] & 0x00F8) >> 2         //B
                | (data[i + 1] & 0x0080) >> 7;    //A
        }
    }

    // IIIIIIII -> IIIIIIIIAAAAAAAA
    void convertI8ToAI88(const unsigned char* data, uint64 dataLen, unsigned char* outData)
    {
        unsigned short* out16 = (unsigned short*)outData;
        for (uint64 i = 0; i < dataLen; ++i)
        {
            *out16++ = 0xFF00     //A
            | data[i];            //I
        }
    }

    // IIIIIIIIAAAAAAAA -> AAAAAAAA
    void convertAI88ToA8(const unsigned char* data, uint64 dataLen, unsigned char* outData)
    {
        for (uint64 i = 1; i < dataLen; i += 2)
        {
            *outData++ = data[i]; //A
        }
    }

    // IIIIIIIIAAAAAAAA -> IIIIIIII
    void convertAI88ToI8(const unsigned char* data, uint64 dataLen, unsigned char* outData)
    {
        for (uint64 i = 0, l = dataLen - 1; i < l; i += 2)
        {
            *outData++ = data[i]; //R
        }
    }

    // RRRRRRRRGGGGGGGGBBBBBBBB -> RRRRRRRRGGGGGGGGBBBBBBBBAAAAAAAA
    void convertRGB888ToRGBA8888(const unsigned char* data, uint64 dataLen, unsigned char* outData)
    {
        for (uint64 i = 0, l = dataLen - 2; i < l; i += 3)
        {
            *outData++ = data[i];         //R
            *outData++ = data[i + 1];     //G
            *outData++ = data[i + 2];     //B
            *outData++ = 0xFF;            //A
        }
    }

    // RRRRRRRRGGGGGGGGBBBBBBBBAAAAAAAA -> RRRRRRRRGGGGGGGGBBBBBBBB
    void convertRGBA8888ToRGB888(const unsigned char* data, uint64 dataLen, unsigned char* outData)
    {
        for (uint64 i = 0, l = dataLen - 3; i < l; i += 4)
        {
            *outData++ = data[i];         //R
            *outData++ = data[i + 1];     //G
            *outData++ = data[i + 2];     //B
        }
    }

    // RRRRRRRRGGGGGGGGBBBBBBBB -> RRRRRGGGGGGBBBBB
    void convertRGB888ToRGB565(const unsigned char* data, uint64 dataLen, unsigned char* outData)
    {
        unsigned short* out16 = (unsigned short*)outData;
        for (uint64 i = 0, l = dataLen - 2; i < l; i += 3)
        {
            *out16++ = (data[i] & 0x00F8) << 8    //R
                | (data[i + 1] & 0x00FC) << 3     //G
                | (data[i + 2] & 0x00F8) >> 3;    //B
        }
    }

    // RRRRRRRRGGGGGGGGBBBBBBBBAAAAAAAA -> RRRRRGGGGGGBBBBB
    void convertRGBA8888ToRGB565(const unsigned char* data, uint64 dataLen, unsigned char* outData)
    {
        unsigned short* out16 = (unsigned short*)outData;
        for (uint64 i = 0, l = dataLen - 3; i < l; i += 4)
        {
            *out16++ = (data[i] & 0x00F8) << 8    //R
                | (data[i + 1] & 0x00FC) << 3     //G
                | (data[i + 2] & 0x00F8) >> 3;    //B
        }
    }

    // RRRRRRRRGGGGGGGGBBBBBBBB -> IIIIIIII
    void convertRGB888ToI8(const unsigned char* data, uint64 dataLen, unsigned char* outData)
    {
        for (uint64 i = 0, l = dataLen - 2; i < l; i += 3)
        {
            *outData++ = (data[i] * 299 + data[i + 1] * 587 + data[i + 2] * 114 + 500) / 1000;  //I =  (R*299 + G*587 + B*114 + 500) / 1000
        }
    }

    // RRRRRRRRGGGGGGGGBBBBBBBBAAAAAAAA -> IIIIIIII
    void convertRGBA8888ToI8(const unsigned char* data, uint64 dataLen, unsigned char* outData)
    {
        for (uint64 i = 0, l = dataLen - 3; i < l; i += 4)
        {
            *outData++ = (data[i] * 299 + data[i + 1] * 587 + data[i + 2] * 114 + 500) / 1000;  //I =  (R*299 + G*587 + B*114 + 500) / 1000
        }
    }

    // RRRRRRRRGGGGGGGGBBBBBBBBAAAAAAAA -> AAAAAAAA
    void convertRGBA8888ToA8(const unsigned char* data, uint64 dataLen, unsigned char* outData)
    {
        for (uint64 i = 0, l = dataLen -3; i < l; i += 4)
        {
            *outData++ = data[i + 3]; //A
        }
    }

    // RRRRRRRRGGGGGGGGBBBBBBBB -> IIIIIIIIAAAAAAAA
    void convertRGB888ToAI88(const unsigned char* data, uint64 dataLen, unsigned char* outData)
    {
        for (uint64 i = 0, l = dataLen - 2; i < l; i += 3)
        {
            *outData++ = (data[i] * 299 + data[i + 1] * 587 + data[i + 2] * 114 + 500) / 1000;  //I =  (R*299 + G*587 + B*114 + 500) / 1000
            *outData++ = 0xFF;
        }
    }


    // RRRRRRRRGGGGGGGGBBBBBBBBAAAAAAAA -> IIIIIIIIAAAAAAAA
    void convertRGBA8888ToAI88(const unsigned char* data, uint64 dataLen, unsigned char* outData)
    {
        for (uint64 i = 0, l = dataLen - 3; i < l; i += 4)
        {
            *outData++ = (data[i] * 299 + data[i + 1] * 587 + data[i + 2] * 114 + 500) / 1000;  //I =  (R*299 + G*587 + B*114 + 500) / 1000
            *outData++ = data[i + 3];
        }
    }

    // RRRRRRRRGGGGGGGGBBBBBBBB -> RRRRGGGGBBBBAAAA
    void convertRGB888ToRGBA4444(const unsigned char* data, uint64 dataLen, unsigned char* outData)
    {
        unsigned short* out16 = (unsigned short*)outData;
        for (uint64 i = 0, l = dataLen - 2; i < l; i += 3)
        {
            *out16++ = ((data[i] & 0x00F0) << 8           //R
                        | (data[i + 1] & 0x00F0) << 4     //G
                        | (data[i + 2] & 0xF0)            //B
                        |  0x0F);                         //A
        }
    }

    // RRRRRRRRGGGGGGGGBBBBBBBBAAAAAAAA -> RRRRGGGGBBBBAAAA
    void convertRGBA8888ToRGBA4444(const unsigned char* data, uint64 dataLen, unsigned char* outData)
    {
        unsigned short* out16 = (unsigned short*)outData;
        for (uint64 i = 0, l = dataLen - 3; i < l; i += 4)
        {
            *out16++ = (data[i] & 0x00F0) << 8    //R
            | (data[i + 1] & 0x00F0) << 4         //G
            | (data[i + 2] & 0xF0)                //B
            |  (data[i + 3] & 0xF0) >> 4;         //A
        }
    }

    // RRRRRRRRGGGGGGGGBBBBBBBB -> RRRRRGGGGGBBBBBA
    void convertRGB888ToRGB5A1(const unsigned char* data, uint64 dataLen, unsigned char* outData)
    {
        unsigned short* out16 = (unsigned short*)outData;
        for (uint64 i = 0, l = dataLen - 2; i < l; i += 3)
        {
            *out16++ = (data[i] & 0x00F8) << 8    //R
                | (data[i + 1] & 0x00F8) << 3     //G
                | (data[i + 2] & 0x00F8) >> 2     //B
                |  0x01;                          //A
        }
    }

    // RRRRRRRRGGGGGGGGBBBBBBBB -> RRRRRGGGGGBBBBBA
    void convertRGBA8888ToRGB5A1(const unsigned char* data, uint64 dataLen, unsigned char* outData)
    {
        unsigned short* out16 = (unsigned short*)outData;
        for (uint64 i = 0, l = dataLen - 2; i < l; i += 4)
        {
            *out16++ = (data[i] & 0x00F8) << 8    //R
                | (data[i + 1] & 0x00F8) << 3     //G
                | (data[i + 2] & 0x00F8) >> 2     //B
                |  (data[i + 3] & 0x0080) >> 7;   //A
        }
    }
}
