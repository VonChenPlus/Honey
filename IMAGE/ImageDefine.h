#ifndef IMAGEDEFINE_H
#define IMAGEDEFINE_H

#include <map>
#include "BASE/Honey.h"

namespace IMAGE
{
    enum class ImageFormat : int8
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
        //!
        NONE = -1
    };

    enum class ImageType : uint8
    {
        JPG,
        PNG,
        ETC,
        RAW_DATA,
        UNKNOWN
    };

    struct ImageFormatInfo {
        ImageFormatInfo(uint32 anInternalFormat, uint32 aFormat, uint32 aType, int aBpp, bool aCompressed, bool anAlpha)
            : internalFormat(anInternalFormat)
            , format(aFormat)
            , type(aType)
            , bpp(aBpp)
            , compressed(aCompressed)
            , alpha(anAlpha) {

        }

        uint32 internalFormat;
        uint32 format;
        uint32 type;
        int bpp;
        bool compressed;
        bool alpha;
    };

    typedef std::map<ImageFormat, const ImageFormatInfo> PixelFormatInfoMap;
    typedef PixelFormatInfoMap::value_type PixelFormatInfoMapValue;
}

#endif // IMAGEDEFINE_H

