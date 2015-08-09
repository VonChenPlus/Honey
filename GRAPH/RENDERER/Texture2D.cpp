#include "GRAPH/RENDERER/Texture2D.h"
#include "GRAPH/BASE/Configuration.h"
#include "GRAPH/RENDERER/GLProgram.h"
#include "GRAPH/RENDERER/GLProgramCache.h"

namespace GRAPH
{
    // If the image has alpha, you can create RGBA8 (32-bit) or RGBA4 (16-bit) or RGB5A1 (16-bit)
    // Default is: RGBA8888 (32-bit textures)
    static IMAGE::PixelFormat g_defaultAlphaPixelFormat = IMAGE::PixelFormat::DEFAULT;

    //////////////////////////////////////////////////////////////////////////
    //conventer function

    // IIIIIIII -> RRRRRRRRGGGGGGGGGBBBBBBBB
    void Texture2D::convertI8ToRGB888(const unsigned char* data, ssize_t dataLen, unsigned char* outData)
    {
        for (ssize_t i=0; i < dataLen; ++i)
        {
            *outData++ = data[i];     //R
            *outData++ = data[i];     //G
            *outData++ = data[i];     //B
        }
    }

    // IIIIIIIIAAAAAAAA -> RRRRRRRRGGGGGGGGBBBBBBBB
    void Texture2D::convertAI88ToRGB888(const unsigned char* data, ssize_t dataLen, unsigned char* outData)
    {
        for (ssize_t i = 0, l = dataLen - 1; i < l; i += 2)
        {
            *outData++ = data[i];     //R
            *outData++ = data[i];     //G
            *outData++ = data[i];     //B
        }
    }

    // IIIIIIII -> RRRRRRRRGGGGGGGGGBBBBBBBBAAAAAAAA
    void Texture2D::convertI8ToRGBA8888(const unsigned char* data, ssize_t dataLen, unsigned char* outData)
    {
        for (ssize_t i = 0; i < dataLen; ++i)
        {
            *outData++ = data[i];     //R
            *outData++ = data[i];     //G
            *outData++ = data[i];     //B
            *outData++ = 0xFF;        //A
        }
    }

    // IIIIIIIIAAAAAAAA -> RRRRRRRRGGGGGGGGBBBBBBBBAAAAAAAA
    void Texture2D::convertAI88ToRGBA8888(const unsigned char* data, ssize_t dataLen, unsigned char* outData)
    {
        for (ssize_t i = 0, l = dataLen - 1; i < l; i += 2)
        {
            *outData++ = data[i];     //R
            *outData++ = data[i];     //G
            *outData++ = data[i];     //B
            *outData++ = data[i + 1]; //A
        }
    }

    // IIIIIIII -> RRRRRGGGGGGBBBBB
    void Texture2D::convertI8ToRGB565(const unsigned char* data, ssize_t dataLen, unsigned char* outData)
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
    void Texture2D::convertAI88ToRGB565(const unsigned char* data, ssize_t dataLen, unsigned char* outData)
    {
        unsigned short* out16 = (unsigned short*)outData;
        for (ssize_t i = 0, l = dataLen - 1; i < l; i += 2)
        {
            *out16++ = (data[i] & 0x00F8) << 8    //R
                | (data[i] & 0x00FC) << 3         //G
                | (data[i] & 0x00F8) >> 3;        //B
        }
    }

    // IIIIIIII -> RRRRGGGGBBBBAAAA
    void Texture2D::convertI8ToRGBA4444(const unsigned char* data, ssize_t dataLen, unsigned char* outData)
    {
        unsigned short* out16 = (unsigned short*)outData;
        for (ssize_t i = 0; i < dataLen; ++i)
        {
            *out16++ = (data[i] & 0x00F0) << 8    //R
            | (data[i] & 0x00F0) << 4             //G
            | (data[i] & 0x00F0)                  //B
            | 0x000F;                             //A
        }
    }

    // IIIIIIIIAAAAAAAA -> RRRRGGGGBBBBAAAA
    void Texture2D::convertAI88ToRGBA4444(const unsigned char* data, ssize_t dataLen, unsigned char* outData)
    {
        unsigned short* out16 = (unsigned short*)outData;
        for (ssize_t i = 0, l = dataLen - 1; i < l; i += 2)
        {
            *out16++ = (data[i] & 0x00F0) << 8    //R
            | (data[i] & 0x00F0) << 4             //G
            | (data[i] & 0x00F0)                  //B
            | (data[i+1] & 0x00F0) >> 4;          //A
        }
    }

    // IIIIIIII -> RRRRRGGGGGBBBBBA
    void Texture2D::convertI8ToRGB5A1(const unsigned char* data, ssize_t dataLen, unsigned char* outData)
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
    void Texture2D::convertAI88ToRGB5A1(const unsigned char* data, ssize_t dataLen, unsigned char* outData)
    {
        unsigned short* out16 = (unsigned short*)outData;
        for (ssize_t i = 0, l = dataLen - 1; i < l; i += 2)
        {
            *out16++ = (data[i] & 0x00F8) << 8    //R
                | (data[i] & 0x00F8) << 3         //G
                | (data[i] & 0x00F8) >> 2         //B
                | (data[i + 1] & 0x0080) >> 7;    //A
        }
    }

    // IIIIIIII -> IIIIIIIIAAAAAAAA
    void Texture2D::convertI8ToAI88(const unsigned char* data, ssize_t dataLen, unsigned char* outData)
    {
        unsigned short* out16 = (unsigned short*)outData;
        for (ssize_t i = 0; i < dataLen; ++i)
        {
            *out16++ = 0xFF00     //A
            | data[i];            //I
        }
    }

    // IIIIIIIIAAAAAAAA -> AAAAAAAA
    void Texture2D::convertAI88ToA8(const unsigned char* data, ssize_t dataLen, unsigned char* outData)
    {
        for (ssize_t i = 1; i < dataLen; i += 2)
        {
            *outData++ = data[i]; //A
        }
    }

    // IIIIIIIIAAAAAAAA -> IIIIIIII
    void Texture2D::convertAI88ToI8(const unsigned char* data, ssize_t dataLen, unsigned char* outData)
    {
        for (ssize_t i = 0, l = dataLen - 1; i < l; i += 2)
        {
            *outData++ = data[i]; //R
        }
    }

    // RRRRRRRRGGGGGGGGBBBBBBBB -> RRRRRRRRGGGGGGGGBBBBBBBBAAAAAAAA
    void Texture2D::convertRGB888ToRGBA8888(const unsigned char* data, ssize_t dataLen, unsigned char* outData)
    {
        for (ssize_t i = 0, l = dataLen - 2; i < l; i += 3)
        {
            *outData++ = data[i];         //R
            *outData++ = data[i + 1];     //G
            *outData++ = data[i + 2];     //B
            *outData++ = 0xFF;            //A
        }
    }

    // RRRRRRRRGGGGGGGGBBBBBBBBAAAAAAAA -> RRRRRRRRGGGGGGGGBBBBBBBB
    void Texture2D::convertRGBA8888ToRGB888(const unsigned char* data, ssize_t dataLen, unsigned char* outData)
    {
        for (ssize_t i = 0, l = dataLen - 3; i < l; i += 4)
        {
            *outData++ = data[i];         //R
            *outData++ = data[i + 1];     //G
            *outData++ = data[i + 2];     //B
        }
    }

    // RRRRRRRRGGGGGGGGBBBBBBBB -> RRRRRGGGGGGBBBBB
    void Texture2D::convertRGB888ToRGB565(const unsigned char* data, ssize_t dataLen, unsigned char* outData)
    {
        unsigned short* out16 = (unsigned short*)outData;
        for (ssize_t i = 0, l = dataLen - 2; i < l; i += 3)
        {
            *out16++ = (data[i] & 0x00F8) << 8    //R
                | (data[i + 1] & 0x00FC) << 3     //G
                | (data[i + 2] & 0x00F8) >> 3;    //B
        }
    }

    // RRRRRRRRGGGGGGGGBBBBBBBBAAAAAAAA -> RRRRRGGGGGGBBBBB
    void Texture2D::convertRGBA8888ToRGB565(const unsigned char* data, ssize_t dataLen, unsigned char* outData)
    {
        unsigned short* out16 = (unsigned short*)outData;
        for (ssize_t i = 0, l = dataLen - 3; i < l; i += 4)
        {
            *out16++ = (data[i] & 0x00F8) << 8    //R
                | (data[i + 1] & 0x00FC) << 3     //G
                | (data[i + 2] & 0x00F8) >> 3;    //B
        }
    }

    // RRRRRRRRGGGGGGGGBBBBBBBB -> IIIIIIII
    void Texture2D::convertRGB888ToI8(const unsigned char* data, ssize_t dataLen, unsigned char* outData)
    {
        for (ssize_t i = 0, l = dataLen - 2; i < l; i += 3)
        {
            *outData++ = (data[i] * 299 + data[i + 1] * 587 + data[i + 2] * 114 + 500) / 1000;  //I =  (R*299 + G*587 + B*114 + 500) / 1000
        }
    }

    // RRRRRRRRGGGGGGGGBBBBBBBBAAAAAAAA -> IIIIIIII
    void Texture2D::convertRGBA8888ToI8(const unsigned char* data, ssize_t dataLen, unsigned char* outData)
    {
        for (ssize_t i = 0, l = dataLen - 3; i < l; i += 4)
        {
            *outData++ = (data[i] * 299 + data[i + 1] * 587 + data[i + 2] * 114 + 500) / 1000;  //I =  (R*299 + G*587 + B*114 + 500) / 1000
        }
    }

    // RRRRRRRRGGGGGGGGBBBBBBBBAAAAAAAA -> AAAAAAAA
    void Texture2D::convertRGBA8888ToA8(const unsigned char* data, ssize_t dataLen, unsigned char* outData)
    {
        for (ssize_t i = 0, l = dataLen -3; i < l; i += 4)
        {
            *outData++ = data[i + 3]; //A
        }
    }

    // RRRRRRRRGGGGGGGGBBBBBBBB -> IIIIIIIIAAAAAAAA
    void Texture2D::convertRGB888ToAI88(const unsigned char* data, ssize_t dataLen, unsigned char* outData)
    {
        for (ssize_t i = 0, l = dataLen - 2; i < l; i += 3)
        {
            *outData++ = (data[i] * 299 + data[i + 1] * 587 + data[i + 2] * 114 + 500) / 1000;  //I =  (R*299 + G*587 + B*114 + 500) / 1000
            *outData++ = 0xFF;
        }
    }


    // RRRRRRRRGGGGGGGGBBBBBBBBAAAAAAAA -> IIIIIIIIAAAAAAAA
    void Texture2D::convertRGBA8888ToAI88(const unsigned char* data, ssize_t dataLen, unsigned char* outData)
    {
        for (ssize_t i = 0, l = dataLen - 3; i < l; i += 4)
        {
            *outData++ = (data[i] * 299 + data[i + 1] * 587 + data[i + 2] * 114 + 500) / 1000;  //I =  (R*299 + G*587 + B*114 + 500) / 1000
            *outData++ = data[i + 3];
        }
    }

    // RRRRRRRRGGGGGGGGBBBBBBBB -> RRRRGGGGBBBBAAAA
    void Texture2D::convertRGB888ToRGBA4444(const unsigned char* data, ssize_t dataLen, unsigned char* outData)
    {
        unsigned short* out16 = (unsigned short*)outData;
        for (ssize_t i = 0, l = dataLen - 2; i < l; i += 3)
        {
            *out16++ = ((data[i] & 0x00F0) << 8           //R
                        | (data[i + 1] & 0x00F0) << 4     //G
                        | (data[i + 2] & 0xF0)            //B
                        |  0x0F);                         //A
        }
    }

    // RRRRRRRRGGGGGGGGBBBBBBBBAAAAAAAA -> RRRRGGGGBBBBAAAA
    void Texture2D::convertRGBA8888ToRGBA4444(const unsigned char* data, ssize_t dataLen, unsigned char* outData)
    {
        unsigned short* out16 = (unsigned short*)outData;
        for (ssize_t i = 0, l = dataLen - 3; i < l; i += 4)
        {
            *out16++ = (data[i] & 0x00F0) << 8    //R
            | (data[i + 1] & 0x00F0) << 4         //G
            | (data[i + 2] & 0xF0)                //B
            |  (data[i + 3] & 0xF0) >> 4;         //A
        }
    }

    // RRRRRRRRGGGGGGGGBBBBBBBB -> RRRRRGGGGGBBBBBA
    void Texture2D::convertRGB888ToRGB5A1(const unsigned char* data, ssize_t dataLen, unsigned char* outData)
    {
        unsigned short* out16 = (unsigned short*)outData;
        for (ssize_t i = 0, l = dataLen - 2; i < l; i += 3)
        {
            *out16++ = (data[i] & 0x00F8) << 8    //R
                | (data[i + 1] & 0x00F8) << 3     //G
                | (data[i + 2] & 0x00F8) >> 2     //B
                |  0x01;                          //A
        }
    }

    // RRRRRRRRGGGGGGGGBBBBBBBB -> RRRRRGGGGGBBBBBA
    void Texture2D::convertRGBA8888ToRGB5A1(const unsigned char* data, ssize_t dataLen, unsigned char* outData)
    {
        unsigned short* out16 = (unsigned short*)outData;
        for (ssize_t i = 0, l = dataLen - 2; i < l; i += 4)
        {
            *out16++ = (data[i] & 0x00F8) << 8    //R
                | (data[i + 1] & 0x00F8) << 3     //G
                | (data[i + 2] & 0x00F8) >> 2     //B
                |  (data[i + 3] & 0x0080) >> 7;   //A
        }
    }
    // conventer function end
    //////////////////////////////////////////////////////////////////////////

    Texture2D::Texture2D()
    : _pixelFormat(IMAGE::PixelFormat::DEFAULT)
    , _pixelsWide(0)
    , _pixelsHigh(0)
    , _name(0)
    , _maxS(0.0)
    , _maxT(0.0)
    , _hasPremultipliedAlpha(false)
    , _hasMipmaps(false)
    , _shaderProgram(nullptr)
    , _antialiasEnabled(true)
    {
    }

    Texture2D::~Texture2D()
    {
        SAFE_RELEASE(_shaderProgram);

        if(_name)
        {
            deleteTexture(_name);
        }
    }

    void Texture2D::releaseGLTexture()
    {
        if(_name)
        {
            deleteTexture(_name);
        }
        _name = 0;
    }


    IMAGE::PixelFormat Texture2D::getPixelFormat() const
    {
        return _pixelFormat;
    }

    int Texture2D::getPixelsWide() const
    {
        return _pixelsWide;
    }

    int Texture2D::getPixelsHigh() const
    {
        return _pixelsHigh;
    }

    GLuint Texture2D::getName() const
    {
        return _name;
    }

    MATH::Sizef Texture2D::getContentSize() const
    {
        MATH::Sizef ret;
        ret.width = _contentSize.width / CC_CONTENT_SCALE_FACTOR();
        ret.height = _contentSize.height / CC_CONTENT_SCALE_FACTOR();

        return ret;
    }

    const MATH::Sizef& Texture2D::getContentSizeInPixels()
    {
        return _contentSize;
    }

    GLfloat Texture2D::getMaxS() const
    {
        return _maxS;
    }

    void Texture2D::setMaxS(GLfloat maxS)
    {
        _maxS = maxS;
    }

    GLfloat Texture2D::getMaxT() const
    {
        return _maxT;
    }

    void Texture2D::setMaxT(GLfloat maxT)
    {
        _maxT = maxT;
    }

    GLProgram* Texture2D::getGLProgram() const
    {
        return _shaderProgram;
    }

    void Texture2D::setGLProgram(GLProgram* shaderProgram)
    {
        SAFE_RETAIN(shaderProgram);
        SAFE_RELEASE(_shaderProgram);
        _shaderProgram = shaderProgram;
    }

    bool Texture2D::hasPremultipliedAlpha() const
    {
        return _hasPremultipliedAlpha;
    }

    bool Texture2D::initWithData(const void *data, ssize_t dataLen, IMAGE::PixelFormat pixelFormat, int pixelsWide, int pixelsHigh, const MATH::Sizef& contentSize)
    {
        //if data has no mipmaps, we will consider it has only one mipmap
        MipmapInfo mipmap;
        mipmap.address = (unsigned char*)data;
        mipmap.len = static_cast<int>(dataLen);
        return initWithMipmaps(&mipmap, 1, pixelFormat, pixelsWide, pixelsHigh);
    }

    bool Texture2D::initWithMipmaps(MipmapInfo* mipmaps, int mipmapsNum, IMAGE::PixelFormat pixelFormat, int pixelsWide, int pixelsHigh)
    {
        if (mipmapsNum <= 0)
        {
            return false;
        }

        if(_pixelFormatInfoTables.find(pixelFormat) == _pixelFormatInfoTables.end())
        {
            return false;
        }

        const IMAGE::PixelFormatInfo& info = _pixelFormatInfoTables.at(pixelFormat);

        if (info.compressed && !Configuration::getInstance()->supportsPVRTC()
                            && !Configuration::getInstance()->supportsETC()
                            && !Configuration::getInstance()->supportsS3TC()
                            && !Configuration::getInstance()->supportsATITC())
        {
            return false;
        }

        //Set the row align only when mipmapsNum == 1 and the data is uncompressed
        if (mipmapsNum == 1 && !info.compressed)
        {
            unsigned int bytesPerRow = pixelsWide * info.bpp / 8;

            if(bytesPerRow % 8 == 0)
            {
                glPixelStorei(GL_UNPACK_ALIGNMENT, 8);
            }
            else if(bytesPerRow % 4 == 0)
            {
                glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
            }
            else if(bytesPerRow % 2 == 0)
            {
                glPixelStorei(GL_UNPACK_ALIGNMENT, 2);
            }
            else
            {
                glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
            }
        }else
        {
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        }

        if(_name != 0)
        {
            deleteTexture(_name);
            _name = 0;
        }

        glGenTextures(1, &_name);
        bindTexture2D(_name);

        if (mipmapsNum == 1)
        {
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, _antialiasEnabled ? GL_LINEAR : GL_NEAREST);
        }else
        {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, _antialiasEnabled ? GL_LINEAR_MIPMAP_NEAREST : GL_NEAREST_MIPMAP_NEAREST);
        }

        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, _antialiasEnabled ? GL_LINEAR : GL_NEAREST );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );

        // Specify OpenGL texture image
        int width = pixelsWide;
        int height = pixelsHigh;

        for (int i = 0; i < mipmapsNum; ++i)
        {
            unsigned char *data = mipmaps[i].address;
            GLsizei datalen = mipmaps[i].len;

            if (info.compressed)
            {
                glCompressedTexImage2D(GL_TEXTURE_2D, i, info.internalFormat, (GLsizei)width, (GLsizei)height, 0, datalen, data);
            }
            else
            {
                glTexImage2D(GL_TEXTURE_2D, i, info.internalFormat, (GLsizei)width, (GLsizei)height, 0, info.format, info.type, data);
            }

            width = std::max(width >> 1, 1);
            height = std::max(height >> 1, 1);
        }

        _contentSize = MATH::Sizef((float)pixelsWide, (float)pixelsHigh);
        _pixelsWide = pixelsWide;
        _pixelsHigh = pixelsHigh;
        _pixelFormat = pixelFormat;
        _maxS = 1;
        _maxT = 1;

        _hasPremultipliedAlpha = false;
        _hasMipmaps = mipmapsNum > 1;

        // shader
        setGLProgram(GLProgramCache::getInstance()->getGLProgram(GLProgram::SHADER_NAME_POSITION_TEXTURE));
        return true;
    }

    bool Texture2D::updateWithData(const void *data,int offsetX,int offsetY,int width,int height)
    {
        if (_name)
        {
            bindTexture2D(_name);
            const IMAGE::PixelFormatInfo& info = _pixelFormatInfoTables.at(_pixelFormat);
            glTexSubImage2D(GL_TEXTURE_2D,0,offsetX,offsetY,width,height,info.format, info.type,data);

            return true;
        }
        return false;
    }

    std::string Texture2D::getDescription() const
    {
        return StringUtils::format("<Texture2D | Name = %u | Dimensions = %ld x %ld | Coordinates = (%.2f, %.2f)>", _name, (long)_pixelsWide, (long)_pixelsHigh, _maxS, _maxT);
    }

    // implementation Texture2D (Image)
    bool Texture2D::initWithImage(Image *image)
    {
        return initWithImage(image, g_defaultAlphaPixelFormat);
    }

    bool Texture2D::initWithImage(IMAGE::Image *image, IMAGE::PixelFormat format)
    {
        if (image == nullptr)
        {
            return false;
        }

        int imageWidth = image->getWidth();
        int imageHeight = image->getHeight();

        Configuration *conf = Configuration::getInstance();

        int maxTextureSize = conf->getMaxTextureSize();
        if (imageWidth > maxTextureSize || imageHeight > maxTextureSize)
        {
            return false;
        }

        unsigned char*   tempData = image->getData();
        MATH::Sizef      imageSize = MATH::Sizef((float)imageWidth, (float)imageHeight);
        IMAGE::PixelFormat      pixelFormat = ((IMAGE::PixelFormat::NONE == format) || (IMAGE::PixelFormat::AUTO == format)) ? image->getRenderFormat() : format;
        IMAGE::PixelFormat      renderFormat = image->getRenderFormat();
        size_t	         tempDataLen = image->getDataLen();


        if (image->getNumberOfMipmaps() > 1)
        {
            initWithMipmaps(image->getMipmaps(), image->getNumberOfMipmaps(), image->getRenderFormat(), imageWidth, imageHeight);

            return true;
        }
        else if (image->isCompressed())
        {
            initWithData(tempData, tempDataLen, image->getRenderFormat(), imageWidth, imageHeight, imageSize);
            return true;
        }
        else
        {
            unsigned char* outTempData = nullptr;
            ssize_t outTempDataLen = 0;

            pixelFormat = convertDataToFormat(tempData, tempDataLen, renderFormat, pixelFormat, &outTempData, &outTempDataLen);

            initWithData(outTempData, outTempDataLen, pixelFormat, imageWidth, imageHeight, imageSize);


            if (outTempData != nullptr && outTempData != tempData)
            {

                free(outTempData);
            }

            // set the premultiplied tag
            _hasPremultipliedAlpha = image->hasPremultipliedAlpha();

            return true;
        }
    }

    IMAGE::PixelFormat Texture2D::convertI8ToFormat(const unsigned char* data, ssize_t dataLen, IMAGE::PixelFormat format, unsigned char** outData, ssize_t* outDataLen)
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
            return PixelFormat::I8;
        }

        return format;
    }

    IMAGE::PixelFormat Texture2D::convertAI88ToFormat(const unsigned char* data, ssize_t dataLen, IMAGE::PixelFormat format, unsigned char** outData, ssize_t* outDataLen)
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

    IMAGE::PixelFormat Texture2D::convertRGB888ToFormat(const unsigned char* data, ssize_t dataLen, IMAGE::PixelFormat format, unsigned char** outData, ssize_t* outDataLen)
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
            return PixelFormat::RGB888;
        }
        return format;
    }

    IMAGE::PixelFormat Texture2D::convertRGBA8888ToFormat(const unsigned char* data, ssize_t dataLen, IMAGE::PixelFormat format, unsigned char** outData, ssize_t* outDataLen)
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
            return PixelFormat::RGBA8888;
        }

        return format;
    }

    /*
    convert map:
    1.PixelFormat::RGBA8888
    2.PixelFormat::RGB888
    3.PixelFormat::RGB565
    4.PixelFormat::A8
    5.PixelFormat::I8
    6.PixelFormat::AI88
    7.PixelFormat::RGBA4444
    8.PixelFormat::RGB5A1

    gray(5) -> 1235678
    gray alpha(6) -> 12345678
    rgb(2) -> 1235678
    rgba(1) -> 12345678

    */
    IMAGE::PixelFormat Texture2D::convertDataToFormat(const unsigned char* data, ssize_t dataLen, PixelFormat originFormat, PixelFormat format, unsigned char** outData, ssize_t* outDataLen)
    {
        // don't need to convert
        if (format == originFormat || format == PixelFormat::AUTO)
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

    // implementation Texture2D (Text)
    bool Texture2D::initWithString(const char *text, const std::string& fontName, float fontSize, const Size& dimensions/* = Size(0, 0)*/, TextHAlignment hAlignment/* =  TextHAlignment::CENTER */, TextVAlignment vAlignment/* =  TextVAlignment::TOP */)
    {
        FontDefinition tempDef;

        tempDef._shadow._shadowEnabled = false;
        tempDef._stroke._strokeEnabled = false;


        tempDef._fontName      = fontName;
        tempDef._fontSize      = fontSize;
        tempDef._dimensions    = dimensions;
        tempDef._alignment     = hAlignment;
        tempDef._vertAlignment = vAlignment;
        tempDef._fontFillColor = Color3B::WHITE;

        return initWithString(text, tempDef);
    }

    bool Texture2D::initWithString(const char *text, const FontDefinition& textDefinition)
    {
        if(!text || 0 == strlen(text))
        {
            return false;
        }

        bool ret = false;
        Device::TextAlign align;

        if (TextVAlignment::TOP == textDefinition._vertAlignment)
        {
            align = (TextHAlignment::CENTER == textDefinition._alignment) ? Device::TextAlign::TOP
            : (TextHAlignment::LEFT == textDefinition._alignment) ? Device::TextAlign::TOP_LEFT : Device::TextAlign::TOP_RIGHT;
        }
        else if (TextVAlignment::CENTER == textDefinition._vertAlignment)
        {
            align = (TextHAlignment::CENTER == textDefinition._alignment) ? Device::TextAlign::CENTER
            : (TextHAlignment::LEFT == textDefinition._alignment) ? Device::TextAlign::LEFT : Device::TextAlign::RIGHT;
        }
        else if (TextVAlignment::BOTTOM == textDefinition._vertAlignment)
        {
            align = (TextHAlignment::CENTER == textDefinition._alignment) ? Device::TextAlign::BOTTOM
            : (TextHAlignment::LEFT == textDefinition._alignment) ? Device::TextAlign::BOTTOM_LEFT : Device::TextAlign::BOTTOM_RIGHT;
        }
        else
        {
            return false;
        }

        IMAGE::PixelFormat      pixelFormat = g_defaultAlphaPixelFormat;
        unsigned char* outTempData = nullptr;
        ssize_t outTempDataLen = 0;

        int imageWidth;
        int imageHeight;
        auto textDef = textDefinition;
        auto contentScaleFactor = CC_CONTENT_SCALE_FACTOR();
        textDef._fontSize *= contentScaleFactor;
        textDef._dimensions.width *= contentScaleFactor;
        textDef._dimensions.height *= contentScaleFactor;
        textDef._stroke._strokeSize *= contentScaleFactor;
        textDef._shadow._shadowEnabled = false;

        bool hasPremultipliedAlpha;
        HData outData = Device::getTextureDataForText(text, textDef, align, imageWidth, imageHeight, hasPremultipliedAlpha);
        if(outData.isNull())
        {
            return false;
        }

        MATH::Sizef  imageSize = MATH::Sizef((float)imageWidth, (float)imageHeight);
        pixelFormat = convertDataToFormat(outData.getBytes(), imageWidth*imageHeight*4, PixelFormat::RGBA8888, pixelFormat, &outTempData, &outTempDataLen);

        ret = initWithData(outTempData, outTempDataLen, pixelFormat, imageWidth, imageHeight, imageSize);

        if (outTempData != nullptr && outTempData != outData.getBytes())
        {
            free(outTempData);
        }
        _hasPremultipliedAlpha = hasPremultipliedAlpha;

        return ret;
    }


    // implementation Texture2D (Drawing)

    void Texture2D::drawAtPoint(const Vec2& point)
    {
        GLfloat    coordinates[] = {
            0.0f,    _maxT,
            _maxS,_maxT,
            0.0f,    0.0f,
            _maxS,0.0f };

        GLfloat    width = (GLfloat)_pixelsWide * _maxS,
            height = (GLfloat)_pixelsHigh * _maxT;

        GLfloat        vertices[] = {
            point.x,            point.y,
            width + point.x,    point.y,
            point.x,            height  + point.y,
            width + point.x,    height  + point.y };

        GL::enableVertexAttribs( GL::VERTEX_ATTRIB_FLAG_POSITION | GL::VERTEX_ATTRIB_FLAG_TEX_COORD );
        _shaderProgram->use();
        _shaderProgram->setUniformsForBuiltins();

        GL::bindTexture2D( _name );


        glVertexAttribPointer(GLProgram::VERTEX_ATTRIB_POSITION, 2, GL_FLOAT, GL_FALSE, 0, vertices);
        glVertexAttribPointer(GLProgram::VERTEX_ATTRIB_TEX_COORD, 2, GL_FLOAT, GL_FALSE, 0, coordinates);

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }

    void Texture2D::drawInRect(const Rect& rect)
    {
        GLfloat    coordinates[] = {
            0.0f,    _maxT,
            _maxS,_maxT,
            0.0f,    0.0f,
            _maxS,0.0f };

        GLfloat    vertices[] = {    rect.origin.x,        rect.origin.y,                            /*0.0f,*/
            rect.origin.x + rect.size.width,        rect.origin.y,                            /*0.0f,*/
            rect.origin.x,                            rect.origin.y + rect.size.height,        /*0.0f,*/
            rect.origin.x + rect.size.width,        rect.origin.y + rect.size.height,        /*0.0f*/ };

        GL::enableVertexAttribs( GL::VERTEX_ATTRIB_FLAG_POSITION | GL::VERTEX_ATTRIB_FLAG_TEX_COORD );
        _shaderProgram->use();
        _shaderProgram->setUniformsForBuiltins();

        GL::bindTexture2D( _name );

        glVertexAttribPointer(GLProgram::VERTEX_ATTRIB_POSITION, 2, GL_FLOAT, GL_FALSE, 0, vertices);
        glVertexAttribPointer(GLProgram::VERTEX_ATTRIB_TEX_COORD, 2, GL_FLOAT, GL_FALSE, 0, coordinates);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }

    void Texture2D::PVRImagesHavePremultipliedAlpha(bool haveAlphaPremultiplied)
    {
        Image::setPVRImagesHavePremultipliedAlpha(haveAlphaPremultiplied);
    }


    //
    // Use to apply MIN/MAG filter
    //
    // implementation Texture2D (GLFilter)

    void Texture2D::generateMipmap()
    {
        CCASSERT(_pixelsWide == ccNextPOT(_pixelsWide) && _pixelsHigh == ccNextPOT(_pixelsHigh), "Mipmap texture only works in POT textures");
        GL::bindTexture2D( _name );
        glGenerateMipmap(GL_TEXTURE_2D);
        _hasMipmaps = true;
    }

    bool Texture2D::hasMipmaps() const
    {
        return _hasMipmaps;
    }

    void Texture2D::setTexParameters(const TexParams &texParams)
    {
        GL::bindTexture2D( _name );
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, texParams.minFilter );
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, texParams.magFilter );
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, texParams.wrapS );
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, texParams.wrapT );
    }

    void Texture2D::setAliasTexParameters()
    {
        if (! _antialiasEnabled)
        {
            return;
        }

        _antialiasEnabled = false;

        if (_name == 0)
        {
            return;
        }

        GL::bindTexture2D( _name );

        if( ! _hasMipmaps )
        {
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
        }
        else
        {
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST );
        }

        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    }

    void Texture2D::setAntiAliasTexParameters()
    {
        if ( _antialiasEnabled )
        {
            return;
        }

        _antialiasEnabled = true;

        if (_name == 0)
        {
            return;
        }

        GL::bindTexture2D( _name );

        if( ! _hasMipmaps )
        {
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
        }
        else
        {
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST );
        }

        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    }

    const char* Texture2D::getStringForFormat() const
    {
        switch (_pixelFormat)
        {
            case IMAGE::PixelFormat::RGBA8888:
                return  "RGBA8888";

            case IMAGE::PixelFormat::RGB888:
                return  "RGB888";

            case IMAGE::PixelFormat::RGB565:
                return  "RGB565";

            case IMAGE::PixelFormat::RGBA4444:
                return  "RGBA4444";

            case IMAGE::PixelFormat::RGB5A1:
                return  "RGB5A1";

            case IMAGE::PixelFormat::AI88:
                return  "AI88";

            case IMAGE::PixelFormat::A8:
                return  "A8";

            case IMAGE::PixelFormat::I8:
                return  "I8";

            case IMAGE::PixelFormat::PVRTC4:
                return  "PVRTC4";

            case IMAGE::PixelFormat::PVRTC2:
                return  "PVRTC2";

            default:
                CCASSERT(false , "unrecognized pixel format");
                CCLOG("stringForFormat: %ld, cannot give useful result", (long)_pixelFormat);
                break;
        }

        return  nullptr;
    }

    //
    // Texture options for images that contains alpha
    //
    // implementation Texture2D (PixelFormat)

    void Texture2D::setDefaultAlphaPixelFormat(IMAGE::PixelFormat format)
    {
        g_defaultAlphaPixelFormat = format;
    }

    IMAGE::PixelFormat Texture2D::getDefaultAlphaPixelFormat()
    {
        return g_defaultAlphaPixelFormat;
    }

    unsigned int Texture2D::getBitsPerPixelForFormat(IMAGE::PixelFormat format) const
    {
        if (format == PixelFormat::NONE || format == PixelFormat::DEFAULT)
        {
            return 0;
        }

        return _pixelFormatInfoTables.at(format).bpp;
    }

    unsigned int Texture2D::getBitsPerPixelForFormat() const
    {
        return this->getBitsPerPixelForFormat(_pixelFormat);
    }

    const IMAGE::PixelFormatInfoMap& Texture2D::getPixelFormatInfoMap()
    {
        return _pixelFormatInfoTables;
    }

    void Texture2D::addSpriteFrameCapInset(SpriteFrame* spritframe, const Rect& capInsets)
    {
        if(nullptr == _ninePatchInfo)
        {
            _ninePatchInfo = new NinePatchInfo;
        }
        if(nullptr == spritframe)
        {
            _ninePatchInfo->capInsetSize = capInsets;
        }
        else
        {
            _ninePatchInfo->capInsetMap[spritframe] = capInsets;
        }
    }

    bool Texture2D::isContain9PatchInfo()const
    {
        return nullptr != _ninePatchInfo;
    }

    const Rect& Texture2D::getSpriteFrameCapInset( cocos2d::SpriteFrame *spriteFrame )const
    {
        if(nullptr == spriteFrame)
        {
            return this->_ninePatchInfo->capInsetSize;
        }
        else
        {
            auto &capInsetMap = this->_ninePatchInfo->capInsetMap;
            if(capInsetMap.find(spriteFrame) != capInsetMap.end())
            {
                return capInsetMap.at(spriteFrame);
            }
            else
            {
                return this->_ninePatchInfo->capInsetSize;
            }
        }
    }


    void Texture2D::removeSpriteFrameCapInset(SpriteFrame* spriteFrame)
    {
        if(nullptr != this->_ninePatchInfo)
        {
            auto capInsetMap = this->_ninePatchInfo->capInsetMap;
            if(capInsetMap.find(spriteFrame) != capInsetMap.end())
            {
                capInsetMap.erase(spriteFrame);
            }
        }
    }
}
