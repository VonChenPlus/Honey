#ifndef IMAGE_H
#define IMAGE_H

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
        PVRTC4,
        //! 4-bit PVRTC-compressed texture: PVRTC4 (has alpha channel)
        PVRTC4A,
        //! 2-bit PVRTC-compressed texture: PVRTC2
        PVRTC2,
        //! 2-bit PVRTC-compressed texture: PVRTC2 (has alpha channel)
        PVRTC2A,
        //! ETC-compressed texture: ETC
        ETC,
        //! S3TC-compressed texture: S3TC_Dxt1
        S3TC_DXT1,
        //! S3TC-compressed texture: S3TC_Dxt3
        S3TC_DXT3,
        //! S3TC-compressed texture: S3TC_Dxt5
        S3TC_DXT5,
        //! ATITC-compressed texture: ATC_RGB
        ATC_RGB,
        //! ATITC-compressed texture: ATC_EXPLICIT_ALPHA
        ATC_EXPLICIT_ALPHA,
        //! ATITC-compresed texture: ATC_INTERPOLATED_ALPHA
        ATC_INTERPOLATED_ALPHA,
        //! Default texture format: AUTO
        DEFAULT = AUTO,

        NONE = -1
    };

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

    class Image : public HObject
    {
    public:
        Image();
        virtual ~Image();

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

        // Getters
        inline unsigned char *   getData()               { return _data; }
        inline ssize_t           getDataLen()            { return _dataLen; }
        inline Format            getFileType()           {return _fileType; }
        inline PixelFormat       getRenderFormat()       { return _renderFormat; }
        inline int               getWidth()              { return _width; }
        inline int               getHeight()             { return _height; }
        inline bool              hasPremultipliedAlpha() { return _hasPremultipliedAlpha; }

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
        static const int MIPMAP_MAX = 16;
        unsigned char *_data;
        ssize_t _dataLen;
        int _width;
        int _height;
        Format _fileType;
        PixelFormat _renderFormat;
        // false if we cann't auto detect the image is premultiplied or not.
        bool _hasPremultipliedAlpha;
        std::string _filePath;

    protected:
        // noncopyable
        Image(const Image&    rImg);
        Image & operator=(const Image&);

        bool initWithImageFileThreadSafe(const std::string& fullpath);

        Format detectFormat(const unsigned char * data, ssize_t dataLen);
        bool isPng(const unsigned char * data, ssize_t dataLen);
        bool isJpg(const unsigned char * data, ssize_t dataLen);
        bool isEtc(const unsigned char * data, ssize_t dataLen);
    };
}

#endif // IMAGE_H
