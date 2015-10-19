#include "BASE/HData.h"
#include "GRAPH/Director.h"
#include "GRAPH/Scheduler.h"
#include "GRAPH/UNITY3D/GLTexture.h"
#include "GRAPH/UNITY3D/GLShader.h"
#include "GRAPH/UNITY3D/GLStateCache.h"
#include "IO/FileUtils.h"

namespace GRAPH
{
    static IMAGE::PixelFormat g_defaultAlphaPixelFormat = IMAGE::PixelFormat::DEFAULT;
    GLTexture::TextToTextureDataDef GLTexture::getTextureDataForText = nullptr;

    GLTexture::GLTexture()
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

    GLTexture::~GLTexture() {
        SAFE_RELEASE(shaderProgram_);
        releaseGLTexture();
    }

    void GLTexture::releaseGLTexture() {
        if(name_) {
            GLStateCache::DeleteTexture(name_);
        }
        name_ = 0;
    }


    IMAGE::PixelFormat GLTexture::getPixelFormat() const {
        return pixelFormat_;
    }

    int GLTexture::getPixelsWidth() const {
        return pixelsWidth_;
    }

    int GLTexture::getPixelsHight() const {
        return pixelsHight_;
    }

    GLuint GLTexture::getName() const {
        return name_;
    }

    MATH::Sizef GLTexture::getContentSize() const {
        MATH::Sizef ret;
        ret.width = contentSize_.width;
        ret.height = contentSize_.height;
        return ret;
    }

    const MATH::Sizef& GLTexture::getContentSizeInPixels() {
        return contentSize_;
    }

    GLfloat GLTexture::getMaxS() const {
        return maxS_;
    }

    void GLTexture::setMaxS(GLfloat maxS) {
        maxS_ = maxS;
    }

    GLfloat GLTexture::getMaxT() const {
        return maxT_;
    }

    void GLTexture::setMaxT(GLfloat maxT) {
        maxT_ = maxT;
    }

    GLShader* GLTexture::getGLShader() const {
        return shaderProgram_;
    }

    void GLTexture::setGLShader(GLShader* shaderProgram) {
        SAFE_RETAIN(shaderProgram);
        SAFE_RELEASE(shaderProgram_);
        shaderProgram_ = shaderProgram;
    }

    bool GLTexture::hasPremultipliedAlpha() const {
        return hasPremultipliedAlpha_;
    }

    bool GLTexture::initWithData(const void *data, uint64 dataLen, IMAGE::PixelFormat pixelFormat, int pixelsWide, int pixelsHigh, const MATH::Sizef&) {
        MipmapInfo mipmap;
        mipmap.address = (unsigned char*)data;
        mipmap.len = static_cast<int>(dataLen);
        return initWithMipmaps(&mipmap, 1, pixelFormat, pixelsWide, pixelsHigh);
    }

    bool GLTexture::initWithMipmaps(MipmapInfo* mipmaps, int mipmapsNum, IMAGE::PixelFormat pixelFormat, int pixelsWide, int pixelsHigh)
    {
        if (mipmapsNum <= 0) {
            return false;
        }

        if (IMAGE::ImageObject::getPixelFormatInfoMap().find(pixelFormat) == IMAGE::ImageObject::getPixelFormatInfoMap().end()) {
            return false;
        }

        const IMAGE::PixelFormatInfo& info = IMAGE::ImageObject::getPixelFormatInfoMap().at(pixelFormat);

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
        setGLShader(GLShaderCache::getInstance().getGLShader(GLShader::SHADER_NAME_POSITION_TEXTURE));
        return true;
    }

    bool GLTexture::updateWithData(const void *data,int offsetX,int offsetY,int width,int height) {
        if (name_) {
            GLStateCache::BindTexture2D(name_);
            const IMAGE::PixelFormatInfo& info = IMAGE::ImageObject::getPixelFormatInfoMap().at(pixelFormat_);
            glTexSubImage2D(GL_TEXTURE_2D,0,offsetX,offsetY,width,height,info.format, info.type,data);
            return true;
        }
        return false;
    }

    bool GLTexture::initWithImage(IMAGE::ImageObject *image) {
        return initWithImage(image, g_defaultAlphaPixelFormat);
    }

    bool GLTexture::initWithImage(IMAGE::ImageObject *image, IMAGE::PixelFormat format) {
        if (image == nullptr) {
            return false;
        }

        int imageWidth = image->getWidth();
        int imageHeight = image->getHeight();

        unsigned char*   tempData = image->getData();
        MATH::Sizef      imageSize = MATH::Sizef((float)imageWidth, (float)imageHeight);
        IMAGE::PixelFormat      pixelFormat = ((IMAGE::PixelFormat::NONE == format) || (IMAGE::PixelFormat::AUTO == format)) ? image->getRenderFormat() : format;
        IMAGE::PixelFormat      renderFormat = image->getRenderFormat();
        uint64	         tempDataLen = image->getDataLen();


        if (image->isCompressed()) {
            initWithData(tempData, tempDataLen, image->getRenderFormat(), imageWidth, imageHeight, imageSize);
            return true;
        }
        else {
            unsigned char* outTempData = nullptr;
            uint64 outTempDataLen = 0;

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

    // implementation GLTexture (Text)
    bool GLTexture::initWithString(const char *text, const std::string& fontName, float fontSize, const MATH::Sizef& dimensions/* = Size(0, 0)*/, TextHAlignment hAlignment/* =  TextHAlignment::CENTER */, TextVAlignment vAlignment/* =  TextVAlignment::TOP */) {
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

    bool GLTexture::initWithString(const char *text, const FontDefinition& textDefinition) {
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
        uint64 outTempDataLen = 0;

        int imageWidth;
        int imageHeight;
        auto textDef = textDefinition;
        textDef.shadow.shadowEnabled = false;

        bool hasPremultipliedAlpha = false;
        if (getTextureDataForText == nullptr)
            throw _HException_Normal("UnImpl getTextureDataForText");
        HData outData = getTextureDataForText(text, textDef, align, imageWidth, imageHeight, hasPremultipliedAlpha);
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

    void GLTexture::drawAtPoint(const MATH::Vector2f& point)
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

        glVertexAttribPointer(GLShader::VERTEX_ATTRIB_POSITION, 2, GL_FLOAT, GL_FALSE, 0, vertices);
        glVertexAttribPointer(GLShader::VERTEX_ATTRIB_TEX_COORD, 2, GL_FLOAT, GL_FALSE, 0, coordinates);

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }

    void GLTexture::drawInRect(const MATH::Rectf& rect) {
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

        glVertexAttribPointer(GLShader::VERTEX_ATTRIB_POSITION, 2, GL_FLOAT, GL_FALSE, 0, vertices);
        glVertexAttribPointer(GLShader::VERTEX_ATTRIB_TEX_COORD, 2, GL_FLOAT, GL_FALSE, 0, coordinates);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }

    void GLTexture::generateMipmap() {
        GLStateCache::BindTexture2D(name_);
        glGenerateMipmap(GL_TEXTURE_2D);
        hasMipmaps_ = true;
    }

    bool GLTexture::hasMipmaps() const {
        return hasMipmaps_;
    }

    void GLTexture::setTexParameters(const TexParams &texParams) {
        GLStateCache::BindTexture2D( name_ );
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, texParams.minFilter );
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, texParams.magFilter );
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, texParams.wrapS );
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, texParams.wrapT );
    }

    void GLTexture::setAliasTexParameters() {
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

    void GLTexture::setAntiAliasTexParameters() {
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

    void GLTexture::setDefaultAlphaPixelFormat(IMAGE::PixelFormat format) {
        g_defaultAlphaPixelFormat = format;
    }

    IMAGE::PixelFormat GLTexture::getDefaultAlphaPixelFormat() {
        return g_defaultAlphaPixelFormat;
    }

    TextureCache::TextureCache()
        : loadingThread_(nullptr)
        , asyncStructQueue_(nullptr)
        , imageInfoQueue_(nullptr)
        , needQuit_(false)
        , asyncRefCount_(0) {
    }

    TextureCache::~TextureCache() {
        for( auto it=textures_.begin(); it!=textures_.end(); ++it)
            (it->second)->release();

        SAFE_DELETE(loadingThread_);
    }

    void TextureCache::addImageAsync(const std::string &path, const std::function<void(GLTexture*)>& callback) {
        GLTexture *texture = nullptr;

        std::string fullpath = IO::FileUtils::getInstance().fullPathForFilename(path);

        auto it = textures_.find(fullpath);
        if( it != textures_.end() )
            texture = it->second;

        if (texture != nullptr) {
            if (callback) callback(texture);
            return;
        }

        // check if file exists
        if ( fullpath.empty() || ! IO::FileUtils::getInstance().isFileExist( fullpath ) ) {
            if (callback) callback(nullptr);
            return;
        }

        // lazy init
        if (asyncStructQueue_ == nullptr) {
            asyncStructQueue_ = new (std::nothrow) std::deque<AsyncStruct*>();
            imageInfoQueue_ = new (std::nothrow) std::deque<ImageInfo*>();

            // create a new thread to load images
            loadingThread_ = new std::thread(&TextureCache::loadImageThread, this);

            needQuit_ = false;
        }

        if (0 == asyncRefCount_) {
            Director::getInstance().getScheduler()->schedule(static_cast<SelectorF>(&TextureCache::addImageAsyncCallBack), this, 0, false);
        }

        ++asyncRefCount_;

        // generate async struct
        AsyncStruct *data = new (std::nothrow) AsyncStruct(fullpath, callback);

        // add async struct into queue
        asyncMutex_.lock();
        asyncStructQueue_->push_back(data);
        asyncMutex_.unlock();

        sleepCondition_.notify_one();
    }

    void TextureCache::unbindImageAsync(const std::string& filename) {
        std::string fullpath = IO::FileUtils::getInstance().fullPathForFilename(filename);

        asyncMutex_.lock();

        if (asyncStructQueue_ && !asyncStructQueue_->empty()) {
            for (auto it = asyncStructQueue_->begin(); it != asyncStructQueue_->end(); ++it) {
                if ((*it)->filename == fullpath) {
                    (*it)->callback = nullptr;
                }
            }
        }

        if (imageInfoQueue_ && !imageInfoQueue_->empty()) {
            for (auto it = imageInfoQueue_->begin(); it != imageInfoQueue_->end(); ++it) {
                if ((*it)->asyncStruct->filename == fullpath) {
                    (*it)->asyncStruct->callback = nullptr;
                }
            }
        }

        asyncMutex_.unlock();
    }

    void TextureCache::unbindAllImageAsync() {
        asyncMutex_.lock();
        if (asyncStructQueue_ && !asyncStructQueue_->empty()) {
            for (auto it = asyncStructQueue_->begin(); it != asyncStructQueue_->end(); ++it) {
                (*it)->callback = nullptr;
            }
        }

        if (imageInfoQueue_ && !imageInfoQueue_->empty()) {
            for (auto it = imageInfoQueue_->begin(); it != imageInfoQueue_->end(); ++it) {
                (*it)->asyncStruct->callback = nullptr;
            }
        }
        asyncMutex_.unlock();
    }

    void TextureCache::loadImageThread()
    {
        AsyncStruct *asyncStruct = nullptr;

        while (true) {
            asyncMutex_.lock();
            if (asyncStructQueue_->empty()) {
                asyncMutex_.unlock();
                if (needQuit_) {
                    break;
                }
                else {
                    std::unique_lock<std::mutex> lk(sleepMutex_);
                    sleepCondition_.wait(lk);
                    continue;
                }
            }
            else {
                asyncStruct = asyncStructQueue_->front();
                asyncMutex_.unlock();
            }

            IMAGE::ImageObject *image = nullptr;
            bool generateImage = false;

            auto it = textures_.find(asyncStruct->filename);
            if( it == textures_.end() ) {
               ImageInfo *imageInfo;
               uint64 pos = 0;
               asyncMutex_.lock();
               uint64 infoSize = imageInfoQueue_->size();
               for (; pos < infoSize; pos++)
               {
                   imageInfo = (*imageInfoQueue_)[pos];
                   if(imageInfo->asyncStruct->filename.compare(asyncStruct->filename) == 0)
                       break;
               }
               asyncMutex_.unlock();
               if(infoSize == 0 || pos == infoSize)
                   generateImage = true;
            }

            if (generateImage) {
                const std::string& filename = asyncStruct->filename;
                // generate image
                image = new (std::nothrow) IMAGE::ImageObject();
                if (image && !image->initWithImageFileThreadSafe(filename)) {
                    SAFE_RELEASE(image);
                    asyncMutex_.lock();
                    asyncStructQueue_->pop_front();
                    asyncMutex_.unlock();
                    continue;
                }
            }

            // generate image info
            ImageInfo *imageInfo = new (std::nothrow) ImageInfo();
            imageInfo->asyncStruct = asyncStruct;
            imageInfo->image = image;

            // put the image info into the queue
            asyncMutex_.lock();
            asyncStructQueue_->pop_front();
            imageInfoQueue_->push_back(imageInfo);
            asyncMutex_.unlock();
        }

        if(asyncStructQueue_ != nullptr) {
            delete asyncStructQueue_;
            asyncStructQueue_ = nullptr;
            delete imageInfoQueue_;
            imageInfoQueue_ = nullptr;
        }
    }

    void TextureCache::addImageAsyncCallBack(float) {
        // the image is generated in loading thread
        std::deque<ImageInfo*> *imagesQueue = imageInfoQueue_;

        asyncMutex_.lock();
        if (imagesQueue->empty()) {
            asyncMutex_.unlock();
        }
        else {
            ImageInfo *imageInfo = imagesQueue->front();
            imagesQueue->pop_front();
            asyncMutex_.unlock();

            AsyncStruct *asyncStruct = imageInfo->asyncStruct;
            IMAGE::ImageObject *image = imageInfo->image;

            const std::string& filename = asyncStruct->filename;

            GLTexture *texture = nullptr;
            if (image) {
                // generate texture in render thread
                texture = new (std::nothrow) GLTexture();

                texture->initWithImage(image);

                // cache the texture. retain it, since it is added in the map
                textures_.insert( std::make_pair(filename, texture) );
                texture->retain();

                texture->autorelease();
            }
            else {
                auto it = textures_.find(asyncStruct->filename);
                if(it != textures_.end())
                    texture = it->second;
            }

            if (asyncStruct->callback) {
                asyncStruct->callback(texture);
            }

            if(image) {
                image->release();
            }
            delete asyncStruct;
            delete imageInfo;

            --asyncRefCount_;
            if (0 == asyncRefCount_) {
                Director::getInstance().getScheduler()->unschedule(static_cast<SelectorF>(&TextureCache::addImageAsyncCallBack), this);
            }
        }
    }

    GLTexture * TextureCache::addImage(const std::string &path) {
        GLTexture * texture = nullptr;
        IMAGE::ImageObject* image = nullptr;
        std::string fullpath = IO::FileUtils::getInstance().fullPathForFilename(path);
        if (fullpath.size() == 0) {
            return nullptr;
        }
        auto it = textures_.find(fullpath);
        if( it != textures_.end() )
            texture = it->second;

        if (! texture) {
            // all images are handled by UIImage except PVR extension that is handled by our own handler
            do
            {
                image = new (std::nothrow) IMAGE::ImageObject();
                if (nullptr == image) break;

                bool bRet = image->initWithImageFile(fullpath);
                if (!bRet) break;

                texture = new (std::nothrow) GLTexture();

                if( texture && texture->initWithImage(image) )
                {
                    // texture already retained, no need to re-retain it
                    textures_.insert( std::make_pair(fullpath, texture) );
                }
            } while (0);
        }

        SAFE_RELEASE(image);

        return texture;
    }

    GLTexture* TextureCache::addImage(IMAGE::ImageObject *image, const std::string &key) {
        GLTexture * texture = nullptr;

        do {
            auto it = textures_.find(key);
            if( it != textures_.end() ) {
                texture = it->second;
                break;
            }

            // prevents overloading the autorelease pool
            texture = new (std::nothrow) GLTexture();
            texture->initWithImage(image);

            if(texture) {
                textures_.insert( std::make_pair(key, texture) );
                texture->retain();

                texture->autorelease();
            }
        } while (false);

        return texture;
    }

    bool TextureCache::reloadTexture(const std::string& fileName) {
        GLTexture * texture = nullptr;
        IMAGE::ImageObject * image = nullptr;

        std::string fullpath = IO::FileUtils::getInstance().fullPathForFilename(fileName);
        if (fullpath.size() == 0) {
            return false;
        }

        auto it = textures_.find(fullpath);
        if (it != textures_.end()) {
            texture = it->second;
        }

        bool ret = false;
        if (! texture) {
            texture = this->addImage(fullpath);
            ret = (texture != nullptr);
        }
        else {
            do {
                image = new (std::nothrow) IMAGE::ImageObject();
                if (nullptr == image) break;

                bool bRet = image->initWithImageFile(fullpath);
                if(!bRet) break;

                ret = texture->initWithImage(image);
            } while (0);
        }

        SAFE_RELEASE(image);

        return ret;
    }

    // TextureCache - Remove
    void TextureCache::removeAllTextures() {
        for( auto it=textures_.begin(); it!=textures_.end(); ++it ) {
            (it->second)->release();
        }
        textures_.clear();
    }

    void TextureCache::removeUnusedTextures() {
        for( auto it=textures_.cbegin(); it!=textures_.cend(); /* nothing */) {
            GLTexture *tex = it->second;
            if( tex->getReferenceCount() == 1 ) {
                tex->release();
                textures_.erase(it++);
            }
            else {
                ++it;
            }
        }
    }

    void TextureCache::removeTexture(GLTexture* texture) {
        if( ! texture ) {
            return;
        }

        for( auto it=textures_.cbegin(); it!=textures_.cend(); /* nothing */ ) {
            if( it->second == texture ) {
                texture->release();
                textures_.erase(it++);
                break;
            }
            else
                ++it;
        }
    }

    void TextureCache::removeTextureForKey(const std::string &textureKeyName) {
        std::string key = textureKeyName;
        auto it = textures_.find(key);

        if( it == textures_.end() ) {
            key = IO::FileUtils::getInstance().fullPathForFilename(textureKeyName);
            it = textures_.find(key);
        }

        if( it != textures_.end() ) {
            (it->second)->release();
            textures_.erase(it);
        }
    }

    GLTexture* TextureCache::getTextureForKey(const std::string &textureKeyName) const {
        std::string key = textureKeyName;
        auto it = textures_.find(key);

        if( it == textures_.end() ) {
            key = IO::FileUtils::getInstance().fullPathForFilename(textureKeyName);
            it = textures_.find(key);
        }

        if( it != textures_.end() )
            return it->second;
        return nullptr;
    }

    const std::string TextureCache::getTextureFilePath( GLTexture *texture )const {
        for(auto& item : textures_) {
            if(item.second == texture) {
                return item.first;
            }
        }
        return "";
    }

    void TextureCache::waitForQuit() {
        // notify sub thread to quick
        needQuit_ = true;
        sleepCondition_.notify_one();
        if (loadingThread_) loadingThread_->join();
    }

    TextureAtlas::TextureAtlas()
        : dirty_(false)
        , texture_(nullptr) {
    }

    TextureAtlas::~TextureAtlas() {
        SAFE_FREE(vbo_.u2.bufferData);
        SAFE_FREE(vbo_.u2.indexData);
        glDeleteBuffers(2, vbo_.u2.objectID);
        SAFE_RELEASE(texture_);
    }

    uint64 TextureAtlas::getTotalQuads() const {
        return vbo_.u2.bufferCount;
    }

    uint64 TextureAtlas::getCapacity() const {
        return vbo_.u2.bufferCapacity;
    }

    GLTexture* TextureAtlas::getTexture() const {
        return texture_;
    }

    void TextureAtlas::setTexture(GLTexture * var) {
        SAFE_RETAIN(var);
        SAFE_RELEASE(texture_);
        texture_ = var;
    }

    V3F_C4B_T2F_Quad* TextureAtlas::getQuads() {
        dirty_ = true;
        return vbo_.u2.bufferData;
    }

    void TextureAtlas::setQuads(V3F_C4B_T2F_Quad* quads) {
        vbo_.u2.bufferData = quads;
    }

    // TextureAtlas - alloc & init
    TextureAtlas * TextureAtlas::create(const std::string& file, uint64 capacity) {
        TextureAtlas * textureAtlas = new (std::nothrow) TextureAtlas();
        if(textureAtlas && textureAtlas->initWithFile(file, capacity)) {
            textureAtlas->autorelease();
            return textureAtlas;
        }
        SAFE_DELETE(textureAtlas);
        return nullptr;
    }

    TextureAtlas * TextureAtlas::createWithTexture(GLTexture *texture, uint64 capacity) {
        TextureAtlas * textureAtlas = new (std::nothrow) TextureAtlas();
        if (textureAtlas && textureAtlas->initWithTexture(texture, capacity)) {
            textureAtlas->autorelease();
            return textureAtlas;
        }
        SAFE_DELETE(textureAtlas);
        return nullptr;
    }

    bool TextureAtlas::initWithFile(const std::string& file, uint64 capacity) {
        // retained in property
        GLTexture *texture = Director::getInstance().getTextureCache()->addImage(file);

        if (texture) {
            return initWithTexture(texture, capacity);
        }

        return false;
    }

    bool TextureAtlas::initWithTexture(GLTexture *texture, uint64 capacity) {
        vbo_.u2.indexCapacity = vbo_.u2.bufferCapacity = capacity;
        vbo_.u2.indexCount = vbo_.u2.bufferCount = 0;

        // retained in property
        this->texture_ = texture;
        SAFE_RETAIN(texture_);

        vbo_.u2.bufferData = (V3F_C4B_T2F_Quad*)malloc( capacity * sizeof(V3F_C4B_T2F_Quad) );
        vbo_.u2.indexData = (GLushort *)malloc( capacity * 6 * sizeof(GLushort) );

        if( ! ( vbo_.u2.bufferData && vbo_.u2.indexData) && capacity > 0) {
            SAFE_FREE(vbo_.u2.bufferData);
            SAFE_FREE(vbo_.u2.indexData);

            // release texture, should set it to null, because the destruction will
            SAFE_RELEASE_NULL(texture_);
            return false;
        }

        memset( vbo_.u2.bufferData, 0, capacity * sizeof(V3F_C4B_T2F_Quad) );
        memset( vbo_.u2.indexData, 0, capacity * 6 * sizeof(GLushort) );

        this->setupIndices();
        setupVBO();

        dirty_ = true;

        return true;
    }

    void TextureAtlas::setupIndices() {
        if (vbo_.u2.indexCapacity == 0)
            return;

        for( int i=0; i < vbo_.u2.indexCapacity; i++) {
            vbo_.u2.indexData[i*6+0] = i*4+0;
            vbo_.u2.indexData[i*6+1] = i*4+1;
            vbo_.u2.indexData[i*6+2] = i*4+2;

            // inverted index. issue #179
            vbo_.u2.indexData[i*6+3] = i*4+3;
            vbo_.u2.indexData[i*6+4] = i*4+2;
            vbo_.u2.indexData[i*6+5] = i*4+1;
        }
    }

    void TextureAtlas::setupVBO() {
        glGenBuffers(2, vbo_.u2.objectID);
        mapBuffers();
    }

    void TextureAtlas::mapBuffers() {
        glBindBuffer(GL_ARRAY_BUFFER, vbo_.u2.objectID[0]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(V3F_C4B_T2F) * vbo_.u2.indexCapacity, vbo_.u2.bufferData, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_.u2.objectID[1]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort) * vbo_.u2.indexCapacity * 6, vbo_.u2.indexData, GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }

    // TextureAtlas - Update, Insert, Move & Remove
    void TextureAtlas::updateQuad(V3F_C4B_T2F_Quad *quad, uint64 index) {
        vbo_.u2.bufferCount = MATH::MATH_MAX( index+1, vbo_.u2.bufferCount);
        vbo_.u2.bufferData[index] = *quad;
        dirty_ = true;
    }

    void TextureAtlas::insertQuad(V3F_C4B_T2F_Quad *quad, uint64 index) {
        vbo_.u2.bufferCount++;

        // issue #575. index can be > totalQuads
        auto remaining = (vbo_.u2.bufferCount-1) - index;

        // last object doesn't need to be moved
        if( remaining > 0) {
            // texture coordinates
            memmove( &vbo_.u2.bufferData[index+1],&vbo_.u2.bufferData[index], sizeof(vbo_.u2.bufferData[0]) * remaining );
        }

        vbo_.u2.bufferData[index] = *quad;
        dirty_ = true;
    }

    void TextureAtlas::insertQuads(V3F_C4B_T2F_Quad* quads, uint64 index, uint64 amount) {
        vbo_.u2.bufferCount += amount;
        auto remaining = (vbo_.u2.bufferCount-1) - index - amount;

        // last object doesn't need to be moved
        if( remaining > 0) {
            // tex coordinates
            memmove( &vbo_.u2.bufferData[index+amount],&vbo_.u2.bufferData[index], sizeof(vbo_.u2.bufferData[0]) * remaining );
        }


        auto max = index + amount;
        int j = 0;
        for (uint64 i = index; i < max ; i++) {
            vbo_.u2.bufferData[index] = quads[j];
            index++;
            j++;
        }

        dirty_ = true;
    }

    void TextureAtlas::insertQuadFromIndex(uint64 oldIndex, uint64 newIndex) {
        if( oldIndex == newIndex ) {
            return;
        }
        // because it is ambiguous in iphone, so we implement abs ourselves
        // unsigned int howMany = abs( oldIndex - newIndex);
        auto howMany = (oldIndex - newIndex) > 0 ? (oldIndex - newIndex) :  (newIndex - oldIndex);
        auto dst = oldIndex;
        auto src = oldIndex + 1;
        if( oldIndex > newIndex) {
            dst = newIndex+1;
            src = newIndex;
        }

        // texture coordinates
        V3F_C4B_T2F_Quad quadsBackup = vbo_.u2.bufferData[oldIndex];
        memmove( &vbo_.u2.bufferData[dst],&vbo_.u2.bufferData[src], sizeof(vbo_.u2.bufferData[0]) * howMany );
        vbo_.u2.bufferData[newIndex] = quadsBackup;
        dirty_ = true;
    }

    void TextureAtlas::removeQuadAtIndex(uint64 index) {
        auto remaining = (vbo_.u2.bufferCount-1) - index;

        // last object doesn't need to be moved
        if( remaining ) {
            // texture coordinates
            memmove( &vbo_.u2.bufferData[index],&vbo_.u2.bufferData[index+1], sizeof(vbo_.u2.bufferData[0]) * remaining );
        }
        vbo_.u2.bufferCount--;
        dirty_ = true;
    }

    void TextureAtlas::removeQuadsAtIndex(uint64 index, uint64 amount) {
        auto remaining = (vbo_.u2.bufferCount) - (index + amount);

        vbo_.u2.bufferCount -= amount;

        if ( remaining ) {
            memmove( &vbo_.u2.bufferData[index], &vbo_.u2.bufferData[index+amount], sizeof(vbo_.u2.bufferData[0]) * remaining );
        }

        dirty_ = true;
    }

    void TextureAtlas::removeAllQuads() {
        vbo_.u2.bufferCount = 0;
    }

    // TextureAtlas - Resize
    bool TextureAtlas::resizeCapacity(uint64 newCapacity) {
        if( newCapacity == vbo_.u2.indexCapacity ) {
            return true;
        }
        auto oldCapactiy = vbo_.u2.indexCapacity;
        // update capacity and totolQuads
        vbo_.u2.bufferCount = MATH::MATH_MIN(vbo_.u2.bufferCount, newCapacity);
        vbo_.u2.indexCapacity = newCapacity;

        V3F_C4B_T2F_Quad* tmpQuads = nullptr;
        GLushort* tmpIndices = nullptr;

        // when calling initWithTexture(fileName, 0) on bada device, calloc(0, 1) will fail and return nullptr,
        // so here must judge whether vbo_.u2.bufferData and vbo_.u2.indexData is nullptr.
        if (vbo_.u2.bufferData == nullptr) {
            tmpQuads = (V3F_C4B_T2F_Quad*)malloc( vbo_.u2.indexCapacity * sizeof(vbo_.u2.bufferData[0]) );
            if (tmpQuads != nullptr) {
                memset(tmpQuads, 0, vbo_.u2.indexCapacity * sizeof(vbo_.u2.bufferData[0]) );
            }
        }
        else {
            tmpQuads = (V3F_C4B_T2F_Quad*)realloc( vbo_.u2.bufferData, sizeof(vbo_.u2.bufferData[0]) * vbo_.u2.indexCapacity );
            if (tmpQuads != nullptr && vbo_.u2.indexCapacity > oldCapactiy) {
                memset(tmpQuads+oldCapactiy, 0, (vbo_.u2.indexCapacity - oldCapactiy)*sizeof(vbo_.u2.bufferData[0]) );
            }
            vbo_.u2.bufferData = nullptr;
        }

        if (vbo_.u2.indexData == nullptr) {
            tmpIndices = (GLushort*)malloc( vbo_.u2.indexCapacity * 6 * sizeof(vbo_.u2.indexData[0]) );
            if (tmpIndices != nullptr) {
                memset( tmpIndices, 0, vbo_.u2.indexCapacity * 6 * sizeof(vbo_.u2.indexData[0]) );
            }
        }
        else {
            tmpIndices = (GLushort*)realloc( vbo_.u2.indexData, sizeof(vbo_.u2.indexData[0]) * vbo_.u2.indexCapacity * 6 );
            if (tmpIndices != nullptr && vbo_.u2.indexCapacity > oldCapactiy) {
                memset( tmpIndices+oldCapactiy, 0, (vbo_.u2.indexCapacity-oldCapactiy) * 6 * sizeof(vbo_.u2.indexData[0]) );
            }
            vbo_.u2.indexData = nullptr;
        }

        if( ! ( tmpQuads && tmpIndices) ) {
            SAFE_FREE(tmpQuads);
            SAFE_FREE(tmpIndices);
            SAFE_FREE(vbo_.u2.bufferData);
            SAFE_FREE(vbo_.u2.indexData);
            vbo_.u2.indexCapacity = vbo_.u2.bufferCount = 0;
            return false;
        }

        vbo_.u2.bufferData = tmpQuads;
        vbo_.u2.indexData = tmpIndices;

        setupIndices();
        mapBuffers();

        dirty_ = true;

        return true;
    }

    void TextureAtlas::increaseTotalQuadsWith(uint64 amount) {
        vbo_.u2.bufferCount += amount;
    }

    void TextureAtlas::moveQuadsFromIndex(uint64 oldIndex, uint64 amount, uint64 newIndex) {
        if( oldIndex == newIndex ) {
            return;
        }
        //create buffer
        uint64 quadSize = sizeof(V3F_C4B_T2F_Quad);
        V3F_C4B_T2F_Quad* tempQuads = (V3F_C4B_T2F_Quad*)malloc( quadSize * amount);
        memcpy( tempQuads, &vbo_.u2.bufferData[oldIndex], quadSize * amount );

        if (newIndex < oldIndex) {
            // move quads from newIndex to newIndex + amount to make room for buffer
            memmove( &vbo_.u2.bufferData[newIndex], &vbo_.u2.bufferData[newIndex+amount], (oldIndex-newIndex)*quadSize);
        }
        else {
            // move quads above back
            memmove( &vbo_.u2.bufferData[oldIndex], &vbo_.u2.bufferData[oldIndex+amount], (newIndex-oldIndex)*quadSize);
        }
        memcpy( &vbo_.u2.bufferData[newIndex], tempQuads, amount*quadSize);
        free(tempQuads);
        dirty_ = true;
    }

    void TextureAtlas::moveQuadsFromIndex(uint64 index, uint64 newIndex) {
        memmove(vbo_.u2.bufferData + newIndex,vbo_.u2.bufferData + index, (vbo_.u2.bufferCount - index) * sizeof(vbo_.u2.bufferData[0]));
    }

    void TextureAtlas::fillWithEmptyQuadsFromIndex(uint64 index, uint64 amount) {
        V3F_C4B_T2F_Quad quad;
        memset(&quad, 0, sizeof(quad));

        auto to = index + amount;
        for (uint64 i = index ; i < to ; i++) {
            vbo_.u2.bufferData[i] = quad;
        }
    }

    // TextureAtlas - Drawing
    void TextureAtlas::drawQuads() {
        this->drawNumberOfQuads(vbo_.u2.bufferCount, 0);
    }

    void TextureAtlas::drawNumberOfQuads(uint64 numberOfQuads) {
        this->drawNumberOfQuads(numberOfQuads, 0);
    }

    void TextureAtlas::drawNumberOfQuads(uint64 numberOfQuads, uint64 start) {
        if(!numberOfQuads)
            return;

        GLStateCache::BindTexture2D(texture_->getName());

        glBindBuffer(GL_ARRAY_BUFFER, vbo_.u2.objectID[0]);

        // FIXME:: update is done in draw... perhaps it should be done in a timer
        if (dirty_) {
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vbo_.u2.bufferData[0]) * vbo_.u2.bufferCount , &vbo_.u2.bufferData[0] );
            dirty_ = false;
        }

        GLStateCache::EnableVertexAttribs(VERTEX_ATTRIB_FLAG_POS_COLOR_TEX);

        // vertices
        glVertexAttribPointer(GLShader::VERTEX_ATTRIB_POSITION, 3, GL_FLOAT, GL_FALSE, sizeof(V3F_C4B_T2F), (GLvoid*) offsetof(V3F_C4B_T2F, vertices));

        // colors
        glVertexAttribPointer(GLShader::VERTEX_ATTRIB_COLOR, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(V3F_C4B_T2F), (GLvoid*) offsetof(V3F_C4B_T2F, colors));

        // tex coords
        glVertexAttribPointer(GLShader::VERTEX_ATTRIB_TEX_COORD, 2, GL_FLOAT, GL_FALSE, sizeof(V3F_C4B_T2F), (GLvoid*) offsetof(V3F_C4B_T2F, texCoords));

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_.u2.objectID[1]);

        glDrawElements(GL_TRIANGLES, (GLsizei)numberOfQuads*6, GL_UNSIGNED_SHORT, (GLvoid*) (start*6*sizeof(vbo_.u2.indexData[0])));

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }
}
