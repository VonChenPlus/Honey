#ifndef IMAGECONVERT_H
#define IMAGECONVERT_H

#include "BASE/HObject.h"
#include "IMAGE/ImageDefine.h"

namespace IMAGE
{
    /**convert functions*/
    ImageFormat convertDataToFormat(const unsigned char* data, uint64 dataLen, ImageFormat originFormat, ImageFormat format, unsigned char** outData, uint64* outDataLen);
    ImageFormat convertI8ToFormat(const unsigned char* data, uint64 dataLen, ImageFormat format, unsigned char** outData, uint64* outDataLen);
    ImageFormat convertAI88ToFormat(const unsigned char* data, uint64 dataLen, ImageFormat format, unsigned char** outData, uint64* outDataLen);
    ImageFormat convertRGB888ToFormat(const unsigned char* data, uint64 dataLen, ImageFormat format, unsigned char** outData, uint64* outDataLen);
    ImageFormat convertRGBA8888ToFormat(const unsigned char* data, uint64 dataLen, ImageFormat format, unsigned char** outData, uint64* outDataLen);

    //I8 to XXX
    void convertI8ToRGB888(const unsigned char* data, uint64 dataLen, unsigned char* outData);
    void convertI8ToRGBA8888(const unsigned char* data, uint64 dataLen, unsigned char* outData);
    void convertI8ToRGB565(const unsigned char* data, uint64 dataLen, unsigned char* outData);
    void convertI8ToRGBA4444(const unsigned char* data, uint64 dataLen, unsigned char* outData);
    void convertI8ToRGB5A1(const unsigned char* data, uint64 dataLen, unsigned char* outData);
    void convertI8ToAI88(const unsigned char* data, uint64 dataLen, unsigned char* outData);

    //AI88 to XXX
    void convertAI88ToRGB888(const unsigned char* data, uint64 dataLen, unsigned char* outData);
    void convertAI88ToRGBA8888(const unsigned char* data, uint64 dataLen, unsigned char* outData);
    void convertAI88ToRGB565(const unsigned char* data, uint64 dataLen, unsigned char* outData);
    void convertAI88ToRGBA4444(const unsigned char* data, uint64 dataLen, unsigned char* outData);
    void convertAI88ToRGB5A1(const unsigned char* data, uint64 dataLen, unsigned char* outData);
    void convertAI88ToA8(const unsigned char* data, uint64 dataLen, unsigned char* outData);
    void convertAI88ToI8(const unsigned char* data, uint64 dataLen, unsigned char* outData);

    //RGB888 to XXX
    void convertRGB888ToRGBA8888(const unsigned char* data, uint64 dataLen, unsigned char* outData);
    void convertRGB888ToRGB565(const unsigned char* data, uint64 dataLen, unsigned char* outData);
    void convertRGB888ToI8(const unsigned char* data, uint64 dataLen, unsigned char* outData);
    void convertRGB888ToAI88(const unsigned char* data, uint64 dataLen, unsigned char* outData);
    void convertRGB888ToRGBA4444(const unsigned char* data, uint64 dataLen, unsigned char* outData);
    void convertRGB888ToRGB5A1(const unsigned char* data, uint64 dataLen, unsigned char* outData);

    //RGBA8888 to XXX
    void convertRGBA8888ToRGB888(const unsigned char* data, uint64 dataLen, unsigned char* outData);
    void convertRGBA8888ToRGB565(const unsigned char* data, uint64 dataLen, unsigned char* outData);
    void convertRGBA8888ToI8(const unsigned char* data, uint64 dataLen, unsigned char* outData);
    void convertRGBA8888ToA8(const unsigned char* data, uint64 dataLen, unsigned char* outData);
    void convertRGBA8888ToAI88(const unsigned char* data, uint64 dataLen, unsigned char* outData);
    void convertRGBA8888ToRGBA4444(const unsigned char* data, uint64 dataLen, unsigned char* outData);
    void convertRGBA8888ToRGB5A1(const unsigned char* data, uint64 dataLen, unsigned char* outData);
}

#endif // IMAGECONVERT_H
