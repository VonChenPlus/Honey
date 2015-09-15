#ifndef TINYIMAGE_H
#define TINYIMAGE_H

#include <map>
#include "BASE/HObject.h"
#include "EXTERNALS/glew/GL/glew.h"

namespace IMAGE
{
    enum class PixelFormat
    {
        //! auto detect the type
        AUTO,
        //! 32-bit texture: BGRA8888
        BGRA8888,
        //! 32-bit texture: RGBA8888
        RGBA8888,
        //! 24-bit texture: RGBA888
        RGB888,
        //! 16-bit texture without Alpha channel
        RGB565,
        //! 8-bit textures used as masks
        A8,
        //! 8-bit intensity texture
        I8,
        //! 16-bit textures used as masks
        AI88,
        //! 16-bit textures: RGBA4444
        RGBA4444,
        //! 16-bit textures: RGB5A1
        RGB5A1,
        //! 4-bit PVRTC-compressed texture: PVRTC4
        //! ETC-compressed texture: ETC
        ETC,
        //! Default texture format: AUTO
        DEFAULT = AUTO,

        NONE = -1
    };

    /**convert functions*/
    IMAGE::PixelFormat convertDataToFormat(const unsigned char* data, ssize_t dataLen, IMAGE::PixelFormat originFormat, IMAGE::PixelFormat format, unsigned char** outData, ssize_t* outDataLen);
    IMAGE::PixelFormat convertI8ToFormat(const unsigned char* data, ssize_t dataLen, IMAGE::PixelFormat format, unsigned char** outData, ssize_t* outDataLen);
    IMAGE::PixelFormat convertAI88ToFormat(const unsigned char* data, ssize_t dataLen, IMAGE::PixelFormat format, unsigned char** outData, ssize_t* outDataLen);
    IMAGE::PixelFormat convertRGB888ToFormat(const unsigned char* data, ssize_t dataLen, IMAGE::PixelFormat format, unsigned char** outData, ssize_t* outDataLen);
    IMAGE::PixelFormat convertRGBA8888ToFormat(const unsigned char* data, ssize_t dataLen, IMAGE::PixelFormat format, unsigned char** outData, ssize_t* outDataLen);

    //I8 to XXX
    static void convertI8ToRGB888(const unsigned char* data, ssize_t dataLen, unsigned char* outData);
    static void convertI8ToRGBA8888(const unsigned char* data, ssize_t dataLen, unsigned char* outData);
    static void convertI8ToRGB565(const unsigned char* data, ssize_t dataLen, unsigned char* outData);
    static void convertI8ToRGBA4444(const unsigned char* data, ssize_t dataLen, unsigned char* outData);
    static void convertI8ToRGB5A1(const unsigned char* data, ssize_t dataLen, unsigned char* outData);
    static void convertI8ToAI88(const unsigned char* data, ssize_t dataLen, unsigned char* outData);

    //AI88 to XXX
    static void convertAI88ToRGB888(const unsigned char* data, ssize_t dataLen, unsigned char* outData);
    static void convertAI88ToRGBA8888(const unsigned char* data, ssize_t dataLen, unsigned char* outData);
    static void convertAI88ToRGB565(const unsigned char* data, ssize_t dataLen, unsigned char* outData);
    static void convertAI88ToRGBA4444(const unsigned char* data, ssize_t dataLen, unsigned char* outData);
    static void convertAI88ToRGB5A1(const unsigned char* data, ssize_t dataLen, unsigned char* outData);
    static void convertAI88ToA8(const unsigned char* data, ssize_t dataLen, unsigned char* outData);
    static void convertAI88ToI8(const unsigned char* data, ssize_t dataLen, unsigned char* outData);

    //RGB888 to XXX
    static void convertRGB888ToRGBA8888(const unsigned char* data, ssize_t dataLen, unsigned char* outData);
    static void convertRGB888ToRGB565(const unsigned char* data, ssize_t dataLen, unsigned char* outData);
    static void convertRGB888ToI8(const unsigned char* data, ssize_t dataLen, unsigned char* outData);
    static void convertRGB888ToAI88(const unsigned char* data, ssize_t dataLen, unsigned char* outData);
    static void convertRGB888ToRGBA4444(const unsigned char* data, ssize_t dataLen, unsigned char* outData);
    static void convertRGB888ToRGB5A1(const unsigned char* data, ssize_t dataLen, unsigned char* outData);

    //RGBA8888 to XXX
    static void convertRGBA8888ToRGB888(const unsigned char* data, ssize_t dataLen, unsigned char* outData);
    static void convertRGBA8888ToRGB565(const unsigned char* data, ssize_t dataLen, unsigned char* outData);
    static void convertRGBA8888ToI8(const unsigned char* data, ssize_t dataLen, unsigned char* outData);
    static void convertRGBA8888ToA8(const unsigned char* data, ssize_t dataLen, unsigned char* outData);
    static void convertRGBA8888ToAI88(const unsigned char* data, ssize_t dataLen, unsigned char* outData);
    static void convertRGBA8888ToRGBA4444(const unsigned char* data, ssize_t dataLen, unsigned char* outData);
    static void convertRGBA8888ToRGB5A1(const unsigned char* data, ssize_t dataLen, unsigned char* outData);

    struct PixelFormatInfo {
        PixelFormatInfo(uint32 anInternalFormat, uint32 aFormat, uint32 aType, int aBpp, bool aCompressed, bool anAlpha)
            : internalFormat(anInternalFormat)
            , format(aFormat)
            , type(aType)
            , bpp(aBpp)
            , compressed(aCompressed)
            , alpha(anAlpha)
        {}

        uint32 internalFormat;
        uint32 format;
        uint32 type;
        int bpp;
        bool compressed;
        bool alpha;
    };

    typedef std::map<PixelFormat, const PixelFormatInfo> PixelFormatInfoMap;
    typedef PixelFormatInfoMap::value_type PixelFormatInfoMapValue;
    static const PixelFormatInfoMapValue TexturePixelFormatInfoTablesValue[] =
    {
        PixelFormatInfoMapValue(PixelFormat::BGRA8888, PixelFormatInfo(GL_BGRA, GL_BGRA, GL_UNSIGNED_BYTE, 32, false, true)),
        PixelFormatInfoMapValue(PixelFormat::RGBA8888, PixelFormatInfo(GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, 32, false, true)),
        PixelFormatInfoMapValue(PixelFormat::RGBA4444, PixelFormatInfo(GL_RGBA, GL_RGBA, GL_UNSIGNED_SHORT_4_4_4_4, 16, false, true)),
        PixelFormatInfoMapValue(PixelFormat::RGB5A1, PixelFormatInfo(GL_RGBA, GL_RGBA, GL_UNSIGNED_SHORT_5_5_5_1, 16, false, true)),
        PixelFormatInfoMapValue(PixelFormat::RGB565, PixelFormatInfo(GL_RGB, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, 16, false, false)),
        PixelFormatInfoMapValue(PixelFormat::RGB888, PixelFormatInfo(GL_RGB, GL_RGB, GL_UNSIGNED_BYTE, 24, false, false)),
        PixelFormatInfoMapValue(PixelFormat::A8, PixelFormatInfo(GL_ALPHA, GL_ALPHA, GL_UNSIGNED_BYTE, 8, false, false)),
        PixelFormatInfoMapValue(PixelFormat::I8, PixelFormatInfo(GL_LUMINANCE, GL_LUMINANCE, GL_UNSIGNED_BYTE, 8, false, false)),
        PixelFormatInfoMapValue(PixelFormat::AI88, PixelFormatInfo(GL_LUMINANCE_ALPHA, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, 16, false, true)),
    };

    class TinyImage : public HObject
    {
    public:
        TinyImage();
        virtual ~TinyImage();

        enum class Format
        {
            JPG,
            PNG,
            ETC,
            RAW_DATA,
            UNKNOWN
        };

        bool initWithImageFile(const std::string& path);
        bool initWithImageData(const unsigned char *data, ssize_t dataLen);
        bool initWithRawData(const unsigned char *data, ssize_t dataLen, int width, int height, int bitsPerComponent, bool preMulti = false);
        bool initWithImageFileThreadSafe(const std::string& fullpath);

        // Getters
        inline unsigned char *   getData()               { return data_; }
        inline ssize_t           getDataLen()            { return dataLen_; }
        inline Format            getFileType()           {return fileType_; }
        inline PixelFormat       getRenderFormat()       { return renderFormat_; }
        inline int               getWidth()              { return width_; }
        inline int               getHeight()             { return height_; }
        inline bool              hasPremultipliedAlpha() { return hasPremultipliedAlpha_; }

        int                      getBitPerPixel();
        bool                     hasAlpha();
        bool                     isCompressed();

        static void setPVRImagesHavePremultipliedAlpha(bool haveAlphaPremultiplied);
        static const PixelFormatInfoMap& getPixelFormatInfoMap();

    protected:
        bool initWithJpgData(const unsigned char *data, ssize_t dataLen);
        bool initWithPngData(const unsigned char *data, ssize_t dataLen);
        bool initWithETCData(const unsigned char *data, ssize_t dataLen);

        void premultipliedAlpha();

    protected:
        // noncopyable
        TinyImage(const TinyImage&    rImg);
        TinyImage & operator=(const TinyImage&);

        Format detectFormat(const unsigned char * data, ssize_t dataLen);
        bool isPng(const unsigned char * data, ssize_t dataLen);
        bool isJpg(const unsigned char * data, ssize_t dataLen);
        bool isEtc(const unsigned char * data, ssize_t dataLen);

    protected:
        unsigned char *data_;
        ssize_t dataLen_;
        int width_;
        int height_;
        Format fileType_;
        PixelFormat renderFormat_;
        // false if we cann't auto detect the image is premultiplied or not.
        bool hasPremultipliedAlpha_;
        std::string filePath_;
    };
}

#endif // TINYIMAGE_H
