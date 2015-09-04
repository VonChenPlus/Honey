#include "BASE/HData.h"
#include "GRAPH/GFX/Texture2D.h"
#include "GRAPH/RENDERER/GLProgram.h"
#include "GRAPH/RENDERER/GLStateCache.h"

namespace GRAPH
{
    static IMAGE::PixelFormat g_defaultAlphaPixelFormat = IMAGE::PixelFormat::DEFAULT;

    Texture2D::Texture2D()
        : pixelFormat_(IMAGE::PixelFormat::DEFAULT)
        , pixelsWidth_(0)
        , pixelsHight_(0)
        , name_(0)
        , maxS_(0.0)
        , maxT_(0.0)
        , hasPremultipliedAlpha_(false)
        , hasMipmaps_(false)
        , shaderProgram_(nullptr)
        , antialiasEnabled_(true) {
    }

    Texture2D::~Texture2D() {
        SAFE_RELEASE(shaderProgram_);
        releaseGLTexture();
    }

    void Texture2D::releaseGLTexture() {
        if(name_) {
            GLStateCache::DeleteTexture(name_);
        }
        name_ = 0;
    }


    IMAGE::PixelFormat Texture2D::getPixelFormat() const {
        return pixelFormat_;
    }

    int Texture2D::getPixelsWidth() const {
        return pixelsWidth_;
    }

    int Texture2D::getPixelsHight() const {
        return pixelsHight_;
    }

    GLuint Texture2D::getName() const {
        return name_;
    }

    MATH::Sizef Texture2D::getContentSize() const {
        MATH::Sizef ret;
        ret.width = contentSize_.width;
        ret.height = contentSize_.height;
        return ret;
    }

    const MATH::Sizef& Texture2D::getContentSizeInPixels() {
        return contentSize_;
    }

    GLfloat Texture2D::getMaxS() const {
        return maxS_;
    }

    void Texture2D::setMaxS(GLfloat maxS) {
        maxS_ = maxS;
    }

    GLfloat Texture2D::getMaxT() const {
        return maxT_;
    }

    void Texture2D::setMaxT(GLfloat maxT) {
        maxT_ = maxT;
    }

    GLProgram* Texture2D::getGLProgram() const {
        return shaderProgram_;
    }

    void Texture2D::setGLProgram(GLProgram* shaderProgram) {
        SAFE_RETAIN(shaderProgram);
        SAFE_RELEASE(shaderProgram_);
        shaderProgram_ = shaderProgram;
    }

    bool Texture2D::hasPremultipliedAlpha() const {
        return hasPremultipliedAlpha_;
    }

    bool Texture2D::initWithData(const void *data, ssize_t dataLen, IMAGE::PixelFormat pixelFormat, int pixelsWide, int pixelsHigh, const MATH::Sizef&) {
        MipmapInfo mipmap;
        mipmap.address = (unsigned char*)data;
        mipmap.len = static_cast<int>(dataLen);
        return initWithMipmaps(&mipmap, 1, pixelFormat, pixelsWide, pixelsHigh);
    }

    bool Texture2D::initWithMipmaps(MipmapInfo* mipmaps, int mipmapsNum, IMAGE::PixelFormat pixelFormat, int pixelsWide, int pixelsHigh)
    {
        if (mipmapsNum <= 0) {
            return false;
        }

        if (IMAGE::TinyImage::getPixelFormatInfoMap().find(pixelFormat) == IMAGE::TinyImage::getPixelFormatInfoMap().end()) {
            return false;
        }

        const IMAGE::PixelFormatInfo& info = IMAGE::TinyImage::getPixelFormatInfoMap().at(pixelFormat);

        //Set the row align only when mipmapsNum == 1 and the data is uncompressed
        if (mipmapsNum == 1 && !info.compressed) {
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
        }
        else {
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        }

        if(name_ != 0) {
            GLStateCache::DeleteTexture(name_);
            name_ = 0;
        }

        glGenTextures(1, &name_);
        GLStateCache::BindTexture2D(name_);

        if (mipmapsNum == 1) {
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, antialiasEnabled_ ? GL_LINEAR : GL_NEAREST);
        }
        else {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, antialiasEnabled_ ? GL_LINEAR_MIPMAP_NEAREST : GL_NEAREST_MIPMAP_NEAREST);
        }

        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, antialiasEnabled_ ? GL_LINEAR : GL_NEAREST );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );

        // Specify OpenGL texture image
        int width = pixelsWide;
        int height = pixelsHigh;

        for (int i = 0; i < mipmapsNum; ++i) {
            unsigned char *data = mipmaps[i].address;
            GLsizei datalen = mipmaps[i].len;

            if (info.compressed) {
                glCompressedTexImage2D(GL_TEXTURE_2D, i, info.internalFormat, (GLsizei)width, (GLsizei)height, 0, datalen, data);
            }
            else {
                glTexImage2D(GL_TEXTURE_2D, i, info.internalFormat, (GLsizei)width, (GLsizei)height, 0, info.format, info.type, data);
            }

            width = MATH::MATH_MAX(width >> 1, 1);
            height = MATH::MATH_MAX(height >> 1, 1);
        }

        contentSize_ = MATH::Sizef((float)pixelsWide, (float)pixelsHigh);
        pixelsWidth_ = pixelsWide;
        pixelsHight_ = pixelsHigh;
        pixelFormat_ = pixelFormat;
        maxS_ = 1;
        maxT_ = 1;

        hasPremultipliedAlpha_ = false;
        hasMipmaps_ = mipmapsNum > 1;

        // shader
        setGLProgram(GLProgramCache::getInstance()->getGLProgram(GLProgram::SHADER_NAME_POSITION_TEXTURE));
        return true;
    }

    bool Texture2D::updateWithData(const void *data,int offsetX,int offsetY,int width,int height) {
        if (name_) {
            GLStateCache::BindTexture2D(name_);
            const IMAGE::PixelFormatInfo& info = IMAGE::TinyImage::getPixelFormatInfoMap().at(pixelFormat_);
            glTexSubImage2D(GL_TEXTURE_2D,0,offsetX,offsetY,width,height,info.format, info.type,data);
            return true;
        }
        return false;
    }

    bool Texture2D::initWithImage(IMAGE::TinyImage *image) {
        return initWithImage(image, g_defaultAlphaPixelFormat);
    }

    bool Texture2D::initWithImage(IMAGE::TinyImage *image, IMAGE::PixelFormat format) {
        if (image == nullptr) {
            return false;
        }

        int imageWidth = image->getWidth();
        int imageHeight = image->getHeight();

        unsigned char*   tempData = image->getData();
        MATH::Sizef      imageSize = MATH::Sizef((float)imageWidth, (float)imageHeight);
        IMAGE::PixelFormat      pixelFormat = ((IMAGE::PixelFormat::NONE == format) || (IMAGE::PixelFormat::AUTO == format)) ? image->getRenderFormat() : format;
        IMAGE::PixelFormat      renderFormat = image->getRenderFormat();
        size_t	         tempDataLen = image->getDataLen();


        if (image->isCompressed()) {
            initWithData(tempData, tempDataLen, image->getRenderFormat(), imageWidth, imageHeight, imageSize);
            return true;
        }
        else {
            unsigned char* outTempData = nullptr;
            ssize_t outTempDataLen = 0;

            pixelFormat = IMAGE::convertDataToFormat(tempData, tempDataLen, renderFormat, pixelFormat, &outTempData, &outTempDataLen);

            initWithData(outTempData, outTempDataLen, pixelFormat, imageWidth, imageHeight, imageSize);

            if (outTempData != nullptr && outTempData != tempData) {
                free(outTempData);
            }

            // set the premultiplied tag
            hasPremultipliedAlpha_ = image->hasPremultipliedAlpha();

            return true;
        }
    }

    // implementation Texture2D (Text)
    bool Texture2D::initWithString(const char *text, const std::string& fontName, float fontSize, const MATH::Sizef& dimensions/* = Size(0, 0)*/, TextHAlignment hAlignment/* =  TextHAlignment::CENTER */, TextVAlignment vAlignment/* =  TextVAlignment::TOP */) {
        FontDefinition tempDef;

        tempDef.shadow.shadowEnabled = false;
        tempDef.stroke.strokeEnabled = false;
        tempDef.fontName      = fontName;
        tempDef.fontSize      = fontSize;
        tempDef.dimensions    = dimensions;
        tempDef.alignment     = hAlignment;
        tempDef.vertAlignment = vAlignment;
        tempDef.fontFillColor = Color3B::WHITE;

        return initWithString(text, tempDef);
    }

    bool Texture2D::initWithString(const char *text, const FontDefinition& textDefinition) {
        if(!text || 0 == strlen(text)) {
            return false;
        }

        bool ret = false;
        TextAlign align;

        if (TextVAlignment::TOP == textDefinition.vertAlignment) {
            align = (TextHAlignment::CENTER == textDefinition.alignment) ? TextAlign::TOP
            : (TextHAlignment::LEFT == textDefinition.alignment) ? TextAlign::TOP_LEFT : TextAlign::TOP_RIGHT;
        }
        else if (TextVAlignment::CENTER == textDefinition.vertAlignment) {
            align = (TextHAlignment::CENTER == textDefinition.alignment) ? TextAlign::CENTER
            : (TextHAlignment::LEFT == textDefinition.alignment) ? TextAlign::LEFT : TextAlign::RIGHT;
        }
        else if (TextVAlignment::BOTTOM == textDefinition.vertAlignment) {
            align = (TextHAlignment::CENTER == textDefinition.alignment) ? TextAlign::BOTTOM
            : (TextHAlignment::LEFT == textDefinition.alignment) ? TextAlign::BOTTOM_LEFT : TextAlign::BOTTOM_RIGHT;
        }
        else {
            return false;
        }

        IMAGE::PixelFormat pixelFormat = g_defaultAlphaPixelFormat;
        HBYTE* outTempData = nullptr;
        ssize_t outTempDataLen = 0;

        int imageWidth;
        int imageHeight;
        auto textDef = textDefinition;
        textDef.shadow.shadowEnabled = false;

        bool hasPremultipliedAlpha;
        // TODO
        throw _HException_Normal("UnImpl getTextureDataForText");
        //HData outData = getTextureDataForText(text, textDef, align, imageWidth, imageHeight, hasPremultipliedAlpha);
        HData outData = HData::Null;
        if(outData.isNull()) {
            return false;
        }

        MATH::Sizef  imageSize = MATH::Sizef((float)imageWidth, (float)imageHeight);
        pixelFormat = IMAGE::convertDataToFormat(outData.getBytes(), imageWidth*imageHeight*4, IMAGE::PixelFormat::RGBA8888, pixelFormat, &outTempData, &outTempDataLen);

        ret = initWithData(outTempData, outTempDataLen, pixelFormat, imageWidth, imageHeight, imageSize);

        if (outTempData != nullptr && outTempData != outData.getBytes()) {
            free(outTempData);
        }
        hasPremultipliedAlpha_ = hasPremultipliedAlpha;
        return ret;
    }

    void Texture2D::drawAtPoint(const MATH::Vector2f& point)
    {
        GLfloat    coordinates[] = {
            0.0f,    maxT_,
            maxS_,maxT_,
            0.0f,    0.0f,
            maxS_,0.0f };

        GLfloat    width = (GLfloat)pixelsWidth_ * maxS_,
            height = (GLfloat)pixelsHight_ * maxT_;

        GLfloat        vertices[] = {
            point.x,            point.y,
            width + point.x,    point.y,
            point.x,            height  + point.y,
            width + point.x,    height  + point.y };

        GLStateCache::EnableVertexAttribs( VERTEX_ATTRIB_FLAG_POSITION | VERTEX_ATTRIB_FLAG_TEX_COORD );
        shaderProgram_->use();
        shaderProgram_->setUniformsForBuiltins();

        GLStateCache::BindTexture2D( name_ );

        glVertexAttribPointer(GLProgram::VERTEX_ATTRIB_POSITION, 2, GL_FLOAT, GL_FALSE, 0, vertices);
        glVertexAttribPointer(GLProgram::VERTEX_ATTRIB_TEX_COORD, 2, GL_FLOAT, GL_FALSE, 0, coordinates);

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }

    void Texture2D::drawInRect(const MATH::Rectf& rect) {
        GLfloat    coordinates[] = {
            0.0f,    maxT_,
            maxS_,maxT_,
            0.0f,    0.0f,
            maxS_,0.0f };

        GLfloat    vertices[] = {    rect.origin.x,        rect.origin.y,                            /*0.0f,*/
            rect.origin.x + rect.size.width,        rect.origin.y,                            /*0.0f,*/
            rect.origin.x,                            rect.origin.y + rect.size.height,        /*0.0f,*/
            rect.origin.x + rect.size.width,        rect.origin.y + rect.size.height,        /*0.0f*/ };

        GLStateCache::EnableVertexAttribs( VERTEX_ATTRIB_FLAG_POSITION | VERTEX_ATTRIB_FLAG_TEX_COORD );
        shaderProgram_->use();
        shaderProgram_->setUniformsForBuiltins();

        GLStateCache::BindTexture2D( name_ );

        glVertexAttribPointer(GLProgram::VERTEX_ATTRIB_POSITION, 2, GL_FLOAT, GL_FALSE, 0, vertices);
        glVertexAttribPointer(GLProgram::VERTEX_ATTRIB_TEX_COORD, 2, GL_FLOAT, GL_FALSE, 0, coordinates);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }

    void Texture2D::generateMipmap() {
        GLStateCache::BindTexture2D(name_);
        glGenerateMipmap(GL_TEXTURE_2D);
        hasMipmaps_ = true;
    }

    bool Texture2D::hasMipmaps() const {
        return hasMipmaps_;
    }

    void Texture2D::setTexParameters(const TexParams &texParams) {
        GLStateCache::BindTexture2D( name_ );
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, texParams.minFilter );
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, texParams.magFilter );
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, texParams.wrapS );
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, texParams.wrapT );
    }

    void Texture2D::setAliasTexParameters() {
        if (! antialiasEnabled_) {
            return;
        }

        antialiasEnabled_ = false;

        if (name_ == 0) {
            return;
        }

        GLStateCache::BindTexture2D(name_);

        if( ! hasMipmaps_ ) {
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
        }
        else {
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST );
        }

        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    }

    void Texture2D::setAntiAliasTexParameters() {
        if ( antialiasEnabled_ ) {
            return;
        }

        antialiasEnabled_ = true;

        if (name_ == 0) {
            return;
        }

        GLStateCache::BindTexture2D( name_ );

        if( ! hasMipmaps_ ) {
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
        }
        else {
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST );
        }

        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    }

    void Texture2D::setDefaultAlphaPixelFormat(IMAGE::PixelFormat format) {
        g_defaultAlphaPixelFormat = format;
    }

    IMAGE::PixelFormat Texture2D::getDefaultAlphaPixelFormat() {
        return g_defaultAlphaPixelFormat;
    }
}
