#include "GRAPH/RENDERER/Texture2D.h"
#include "GRAPH/BASE/Configuration.h"
#include "GRAPH/RENDERER/GLProgram.h"
#include "GRAPH/RENDERER/GLProgramCache.h"
#include "BASE/HData.h"
#include "GRAPH/RENDERER/GLStateCache.h"
#include "GRAPH/BASE/Director.h"
#undef max
#undef min

namespace GRAPH
{
    // If the image has alpha, you can create RGBA8 (32-bit) or RGBA4 (16-bit) or RGB5A1 (16-bit)
    // Default is: RGBA8888 (32-bit textures)
    static IMAGE::PixelFormat g_defaultAlphaPixelFormat = IMAGE::PixelFormat::DEFAULT;

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
        ret.width = _contentSize.width / Director::getInstance()->getContentScaleFactor();
        ret.height = _contentSize.height / Director::getInstance()->getContentScaleFactor();

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

    bool Texture2D::initWithData(const void *data, ssize_t dataLen, IMAGE::PixelFormat pixelFormat, int pixelsWide, int pixelsHigh, const MATH::Sizef&)
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

    // implementation Texture2D (Image)
    bool Texture2D::initWithImage(IMAGE::SmartImage *image)
    {
        return initWithImage(image, g_defaultAlphaPixelFormat);
    }

    bool Texture2D::initWithImage(IMAGE::SmartImage *image, IMAGE::PixelFormat format)
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


        if (image->isCompressed())
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

    // implementation Texture2D (Text)
    bool Texture2D::initWithString(const char *text, const std::string& fontName, float fontSize, const MATH::Sizef& dimensions/* = Size(0, 0)*/, TextHAlignment hAlignment/* =  TextHAlignment::CENTER */, TextVAlignment vAlignment/* =  TextVAlignment::TOP */)
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
        TextAlign align;

        if (TextVAlignment::TOP == textDefinition._vertAlignment)
        {
            align = (TextHAlignment::CENTER == textDefinition._alignment) ? TextAlign::TOP
            : (TextHAlignment::LEFT == textDefinition._alignment) ? TextAlign::TOP_LEFT : TextAlign::TOP_RIGHT;
        }
        else if (TextVAlignment::CENTER == textDefinition._vertAlignment)
        {
            align = (TextHAlignment::CENTER == textDefinition._alignment) ? TextAlign::CENTER
            : (TextHAlignment::LEFT == textDefinition._alignment) ? TextAlign::LEFT : TextAlign::RIGHT;
        }
        else if (TextVAlignment::BOTTOM == textDefinition._vertAlignment)
        {
            align = (TextHAlignment::CENTER == textDefinition._alignment) ? TextAlign::BOTTOM
            : (TextHAlignment::LEFT == textDefinition._alignment) ? TextAlign::BOTTOM_LEFT : TextAlign::BOTTOM_RIGHT;
        }
        else
        {
            return false;
        }

        IMAGE::PixelFormat      pixelFormat = g_defaultAlphaPixelFormat;
        HBYTE* outTempData = nullptr;
        ssize_t outTempDataLen = 0;

        int imageWidth;
        int imageHeight;
        auto textDef = textDefinition;
        auto contentScaleFactor = Director::getInstance()->getContentScaleFactor();
        textDef._fontSize *= contentScaleFactor;
        textDef._dimensions.width *= contentScaleFactor;
        textDef._dimensions.height *= contentScaleFactor;
        textDef._stroke._strokeSize *= contentScaleFactor;
        textDef._shadow._shadowEnabled = false;

        bool hasPremultipliedAlpha;
        // TODO
        throw _HException_Normal("UnImpl getTextureDataForText");
        //HData outData = getTextureDataForText(text, textDef, align, imageWidth, imageHeight, hasPremultipliedAlpha);
        HData outData = HData::Null;
        if(outData.isNull())
        {
            return false;
        }

        MATH::Sizef  imageSize = MATH::Sizef((float)imageWidth, (float)imageHeight);
        pixelFormat = convertDataToFormat(outData.getBytes(), imageWidth*imageHeight*4, IMAGE::PixelFormat::RGBA8888, pixelFormat, &outTempData, &outTempDataLen);

        ret = initWithData(outTempData, outTempDataLen, pixelFormat, imageWidth, imageHeight, imageSize);

        if (outTempData != nullptr && outTempData != outData.getBytes())
        {
            free(outTempData);
        }
        _hasPremultipliedAlpha = hasPremultipliedAlpha;

        return ret;
    }


    // implementation Texture2D (Drawing)

    void Texture2D::drawAtPoint(const MATH::Vector2f& point)
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

        enableVertexAttribs( VERTEX_ATTRIB_FLAG_POSITION | VERTEX_ATTRIB_FLAG_TEX_COORD );
        _shaderProgram->use();
        _shaderProgram->setUniformsForBuiltins();

        bindTexture2D( _name );


        glVertexAttribPointer(GLProgram::VERTEX_ATTRIB_POSITION, 2, GL_FLOAT, GL_FALSE, 0, vertices);
        glVertexAttribPointer(GLProgram::VERTEX_ATTRIB_TEX_COORD, 2, GL_FLOAT, GL_FALSE, 0, coordinates);

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }

    void Texture2D::drawInRect(const MATH::Rectf& rect)
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

        enableVertexAttribs( VERTEX_ATTRIB_FLAG_POSITION | VERTEX_ATTRIB_FLAG_TEX_COORD );
        _shaderProgram->use();
        _shaderProgram->setUniformsForBuiltins();

        bindTexture2D( _name );

        glVertexAttribPointer(GLProgram::VERTEX_ATTRIB_POSITION, 2, GL_FLOAT, GL_FALSE, 0, vertices);
        glVertexAttribPointer(GLProgram::VERTEX_ATTRIB_TEX_COORD, 2, GL_FLOAT, GL_FALSE, 0, coordinates);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }

    void Texture2D::generateMipmap()
    {        
        bindTexture2D( _name );
        glGenerateMipmap(GL_TEXTURE_2D);
        _hasMipmaps = true;
    }

    bool Texture2D::hasMipmaps() const
    {
        return _hasMipmaps;
    }

    void Texture2D::setTexParameters(const TexParams &texParams)
    {
        bindTexture2D( _name );
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

        bindTexture2D( _name );

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

        bindTexture2D( _name );

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

    void Texture2D::setDefaultAlphaPixelFormat(IMAGE::PixelFormat format)
    {
        g_defaultAlphaPixelFormat = format;
    }

    IMAGE::PixelFormat Texture2D::getDefaultAlphaPixelFormat()
    {
        return g_defaultAlphaPixelFormat;
    }
}
