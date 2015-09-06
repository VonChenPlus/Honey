#include "BASE/HData.h"
#include "GRAPH/Director.h"
#include "GRAPH/RENDERER/Texture2D.h"
#include "GRAPH/RENDERER/GLProgram.h"
#include "GRAPH/RENDERER/GLStateCache.h"
#include "IO/FileUtils.h"

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

    void TextureCache::addImageAsync(const std::string &path, const std::function<void(Texture2D*)>& callback) {
        Texture2D *texture = nullptr;

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
            // TODO
            // Director::getInstance()->getScheduler()->schedule(CC_SCHEDULE_SELECTOR(TextureCache::addImageAsyncCallBack), this, 0, false);
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

            IMAGE::TinyImage *image = nullptr;
            bool generateImage = false;

            auto it = textures_.find(asyncStruct->filename);
            if( it == textures_.end() ) {
               ImageInfo *imageInfo;
               size_t pos = 0;
               asyncMutex_.lock();
               size_t infoSize = imageInfoQueue_->size();
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
                image = new (std::nothrow) IMAGE::TinyImage();
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
            IMAGE::TinyImage *image = imageInfo->image;

            const std::string& filename = asyncStruct->filename;

            Texture2D *texture = nullptr;
            if (image) {
                // generate texture in render thread
                texture = new (std::nothrow) Texture2D();

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
                // TODO
                // Director::getInstance()->getScheduler()->unschedule(CC_SCHEDULE_SELECTOR(TextureCache::addImageAsyncCallBack), this);
            }
        }
    }

    Texture2D * TextureCache::addImage(const std::string &path) {
        Texture2D * texture = nullptr;
        IMAGE::TinyImage* image = nullptr;
        // Split up directory and filename
        // MUTEX:
        // Needed since addImageAsync calls this method from a different thread

        std::string fullpath = IO::FileUtils::getInstance().fullPathForFilename(path);
        if (fullpath.size() == 0) {
            return nullptr;
        }
        auto it = textures_.find(fullpath);
        if( it != textures_.end() )
            texture = it->second;

        if (! texture)
        {
            // all images are handled by UIImage except PVR extension that is handled by our own handler
            do
            {
                image = new (std::nothrow) IMAGE::TinyImage();
                if (nullptr == image) break;

                bool bRet = image->initWithImageFile(fullpath);
                if (!bRet) break;

                texture = new (std::nothrow) Texture2D();

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

    Texture2D* TextureCache::addImage(IMAGE::TinyImage *image, const std::string &key) {
        Texture2D * texture = nullptr;

        do {
            auto it = textures_.find(key);
            if( it != textures_.end() ) {
                texture = it->second;
                break;
            }

            // prevents overloading the autorelease pool
            texture = new (std::nothrow) Texture2D();
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
        Texture2D * texture = nullptr;
        IMAGE::TinyImage * image = nullptr;

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
                image = new (std::nothrow) IMAGE::TinyImage();
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
            Texture2D *tex = it->second;
            if( tex->getReferenceCount() == 1 ) {
                tex->release();
                textures_.erase(it++);
            }
            else {
                ++it;
            }
        }
    }

    void TextureCache::removeTexture(Texture2D* texture) {
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

    Texture2D* TextureCache::getTextureForKey(const std::string &textureKeyName) const {
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

    const std::string TextureCache::getTextureFilePath( Texture2D *texture )const {
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
        :indices_(nullptr)
        ,dirty_(false)
        ,texture_(nullptr)
        ,quads_(nullptr) {
    }

    TextureAtlas::~TextureAtlas() {
        SAFE_FREE(quads_);
        SAFE_FREE(indices_);

        glDeleteBuffers(2, buffersVBO_);

        SAFE_RELEASE(texture_);
    }

    ssize_t TextureAtlas::getTotalQuads() const {
        return totalQuads_;
    }

    ssize_t TextureAtlas::getCapacity() const {
        return capacity_;
    }

    Texture2D* TextureAtlas::getTexture() const {
        return texture_;
    }

    void TextureAtlas::setTexture(Texture2D * var) {
        SAFE_RETAIN(var);
        SAFE_RELEASE(texture_);
        texture_ = var;
    }

    V3F_C4B_T2F_Quad* TextureAtlas::getQuads() {
        //if someone accesses the quads directly, presume that changes will be made
        dirty_ = true;
        return quads_;
    }

    void TextureAtlas::setQuads(V3F_C4B_T2F_Quad* quads) {
        quads_ = quads;
    }

    // TextureAtlas - alloc & init
    TextureAtlas * TextureAtlas::create(const std::string& file, ssize_t capacity) {
        TextureAtlas * textureAtlas = new (std::nothrow) TextureAtlas();
        if(textureAtlas && textureAtlas->initWithFile(file, capacity)) {
            textureAtlas->autorelease();
            return textureAtlas;
        }
        SAFE_DELETE(textureAtlas);
        return nullptr;
    }

    TextureAtlas * TextureAtlas::createWithTexture(Texture2D *texture, ssize_t capacity) {
        TextureAtlas * textureAtlas = new (std::nothrow) TextureAtlas();
        if (textureAtlas && textureAtlas->initWithTexture(texture, capacity)) {
            textureAtlas->autorelease();
            return textureAtlas;
        }
        SAFE_DELETE(textureAtlas);
        return nullptr;
    }

    bool TextureAtlas::initWithFile(const std::string& file, ssize_t capacity) {
        // retained in property
        Texture2D *texture = Director::getInstance().getTextureCache()->addImage(file);

        if (texture) {
            return initWithTexture(texture, capacity);
        }

        return false;
    }

    bool TextureAtlas::initWithTexture(Texture2D *texture, ssize_t capacity) {
        capacity_ = capacity;
        totalQuads_ = 0;

        // retained in property
        this->texture_ = texture;
        SAFE_RETAIN(texture_);

        quads_ = (V3F_C4B_T2F_Quad*)malloc( capacity_ * sizeof(V3F_C4B_T2F_Quad) );
        indices_ = (GLushort *)malloc( capacity_ * 6 * sizeof(GLushort) );

        if( ! ( quads_ && indices_) && capacity_ > 0) {
            SAFE_FREE(quads_);
            SAFE_FREE(indices_);

            // release texture, should set it to null, because the destruction will
            // release it too. see cocos2d-x issue #484
            SAFE_RELEASE_NULL(texture_);
            return false;
        }

        memset( quads_, 0, capacity_ * sizeof(V3F_C4B_T2F_Quad) );
        memset( indices_, 0, capacity_ * 6 * sizeof(GLushort) );

        this->setupIndices();
        setupVBO();

        dirty_ = true;

        return true;
    }

    void TextureAtlas::setupIndices() {
        if (capacity_ == 0)
            return;

        for( int i=0; i < capacity_; i++) {
            indices_[i*6+0] = i*4+0;
            indices_[i*6+1] = i*4+1;
            indices_[i*6+2] = i*4+2;

            // inverted index. issue #179
            indices_[i*6+3] = i*4+3;
            indices_[i*6+4] = i*4+2;
            indices_[i*6+5] = i*4+1;
        }
    }

    void TextureAtlas::setupVBO() {
        glGenBuffers(2, &buffersVBO_[0]);
        mapBuffers();
    }

    void TextureAtlas::mapBuffers() {
        glBindBuffer(GL_ARRAY_BUFFER, buffersVBO_[0]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quads_[0]) * capacity_, quads_, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffersVBO_[1]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices_[0]) * capacity_ * 6, indices_, GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }

    // TextureAtlas - Update, Insert, Move & Remove
    void TextureAtlas::updateQuad(V3F_C4B_T2F_Quad *quad, ssize_t index) {
        totalQuads_ = MATH::MATH_MAX( index+1, totalQuads_);
        quads_[index] = *quad;
        dirty_ = true;
    }

    void TextureAtlas::insertQuad(V3F_C4B_T2F_Quad *quad, ssize_t index) {
        totalQuads_++;

        // issue #575. index can be > totalQuads
        auto remaining = (totalQuads_-1) - index;

        // last object doesn't need to be moved
        if( remaining > 0) {
            // texture coordinates
            memmove( &quads_[index+1],&quads_[index], sizeof(quads_[0]) * remaining );
        }

        quads_[index] = *quad;
        dirty_ = true;
    }

    void TextureAtlas::insertQuads(V3F_C4B_T2F_Quad* quads, ssize_t index, ssize_t amount) {
        totalQuads_ += amount;
        auto remaining = (totalQuads_-1) - index - amount;

        // last object doesn't need to be moved
        if( remaining > 0) {
            // tex coordinates
            memmove( &quads_[index+amount],&quads_[index], sizeof(quads_[0]) * remaining );
        }


        auto max = index + amount;
        int j = 0;
        for (ssize_t i = index; i < max ; i++) {
            quads_[index] = quads[j];
            index++;
            j++;
        }

        dirty_ = true;
    }

    void TextureAtlas::insertQuadFromIndex(ssize_t oldIndex, ssize_t newIndex) {
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
        V3F_C4B_T2F_Quad quadsBackup = quads_[oldIndex];
        memmove( &quads_[dst],&quads_[src], sizeof(quads_[0]) * howMany );
        quads_[newIndex] = quadsBackup;
        dirty_ = true;
    }

    void TextureAtlas::removeQuadAtIndex(ssize_t index) {
        auto remaining = (totalQuads_-1) - index;

        // last object doesn't need to be moved
        if( remaining ) {
            // texture coordinates
            memmove( &quads_[index],&quads_[index+1], sizeof(quads_[0]) * remaining );
        }
        totalQuads_--;
        dirty_ = true;
    }

    void TextureAtlas::removeQuadsAtIndex(ssize_t index, ssize_t amount) {
        auto remaining = (totalQuads_) - (index + amount);

        totalQuads_ -= amount;

        if ( remaining ) {
            memmove( &quads_[index], &quads_[index+amount], sizeof(quads_[0]) * remaining );
        }

        dirty_ = true;
    }

    void TextureAtlas::removeAllQuads() {
        totalQuads_ = 0;
    }

    // TextureAtlas - Resize
    bool TextureAtlas::resizeCapacity(ssize_t newCapacity) {
        if( newCapacity == capacity_ ) {
            return true;
        }
        auto oldCapactiy = capacity_;
        // update capacity and totolQuads
        totalQuads_ = MATH::MATH_MIN(totalQuads_, newCapacity);
        capacity_ = newCapacity;

        V3F_C4B_T2F_Quad* tmpQuads = nullptr;
        GLushort* tmpIndices = nullptr;

        // when calling initWithTexture(fileName, 0) on bada device, calloc(0, 1) will fail and return nullptr,
        // so here must judge whether quads_ and indices_ is nullptr.
        if (quads_ == nullptr) {
            tmpQuads = (V3F_C4B_T2F_Quad*)malloc( capacity_ * sizeof(quads_[0]) );
            if (tmpQuads != nullptr) {
                memset(tmpQuads, 0, capacity_ * sizeof(quads_[0]) );
            }
        }
        else {
            tmpQuads = (V3F_C4B_T2F_Quad*)realloc( quads_, sizeof(quads_[0]) * capacity_ );
            if (tmpQuads != nullptr && capacity_ > oldCapactiy) {
                memset(tmpQuads+oldCapactiy, 0, (capacity_ - oldCapactiy)*sizeof(quads_[0]) );
            }
            quads_ = nullptr;
        }

        if (indices_ == nullptr) {
            tmpIndices = (GLushort*)malloc( capacity_ * 6 * sizeof(indices_[0]) );
            if (tmpIndices != nullptr) {
                memset( tmpIndices, 0, capacity_ * 6 * sizeof(indices_[0]) );
            }
        }
        else {
            tmpIndices = (GLushort*)realloc( indices_, sizeof(indices_[0]) * capacity_ * 6 );
            if (tmpIndices != nullptr && capacity_ > oldCapactiy) {
                memset( tmpIndices+oldCapactiy, 0, (capacity_-oldCapactiy) * 6 * sizeof(indices_[0]) );
            }
            indices_ = nullptr;
        }

        if( ! ( tmpQuads && tmpIndices) ) {
            SAFE_FREE(tmpQuads);
            SAFE_FREE(tmpIndices);
            SAFE_FREE(quads_);
            SAFE_FREE(indices_);
            capacity_ = totalQuads_ = 0;
            return false;
        }

        quads_ = tmpQuads;
        indices_ = tmpIndices;

        setupIndices();
        mapBuffers();

        dirty_ = true;

        return true;
    }

    void TextureAtlas::increaseTotalQuadsWith(ssize_t amount) {
        totalQuads_ += amount;
    }

    void TextureAtlas::moveQuadsFromIndex(ssize_t oldIndex, ssize_t amount, ssize_t newIndex) {
        if( oldIndex == newIndex ) {
            return;
        }
        //create buffer
        size_t quadSize = sizeof(V3F_C4B_T2F_Quad);
        V3F_C4B_T2F_Quad* tempQuads = (V3F_C4B_T2F_Quad*)malloc( quadSize * amount);
        memcpy( tempQuads, &quads_[oldIndex], quadSize * amount );

        if (newIndex < oldIndex) {
            // move quads from newIndex to newIndex + amount to make room for buffer
            memmove( &quads_[newIndex], &quads_[newIndex+amount], (oldIndex-newIndex)*quadSize);
        }
        else {
            // move quads above back
            memmove( &quads_[oldIndex], &quads_[oldIndex+amount], (newIndex-oldIndex)*quadSize);
        }
        memcpy( &quads_[newIndex], tempQuads, amount*quadSize);
        free(tempQuads);
        dirty_ = true;
    }

    void TextureAtlas::moveQuadsFromIndex(ssize_t index, ssize_t newIndex) {
        memmove(quads_ + newIndex,quads_ + index, (totalQuads_ - index) * sizeof(quads_[0]));
    }

    void TextureAtlas::fillWithEmptyQuadsFromIndex(ssize_t index, ssize_t amount) {
        V3F_C4B_T2F_Quad quad;
        memset(&quad, 0, sizeof(quad));

        auto to = index + amount;
        for (ssize_t i = index ; i < to ; i++) {
            quads_[i] = quad;
        }
    }

    // TextureAtlas - Drawing
    void TextureAtlas::drawQuads() {
        this->drawNumberOfQuads(totalQuads_, 0);
    }

    void TextureAtlas::drawNumberOfQuads(ssize_t numberOfQuads) {
        this->drawNumberOfQuads(numberOfQuads, 0);
    }

    void TextureAtlas::drawNumberOfQuads(ssize_t numberOfQuads, ssize_t start) {
        if(!numberOfQuads)
            return;

        GLStateCache::BindTexture2D(texture_->getName());

        {
            #define kQuadSize sizeof(quads_[0].bl)
            glBindBuffer(GL_ARRAY_BUFFER, buffersVBO_[0]);

            // FIXME:: update is done in draw... perhaps it should be done in a timer
            if (dirty_) {
                glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(quads_[0]) * totalQuads_ , &quads_[0] );
                dirty_ = false;
            }

            GLStateCache::EnableVertexAttribs(VERTEX_ATTRIB_FLAG_POS_COLOR_TEX);

            // vertices
            glVertexAttribPointer(GLProgram::VERTEX_ATTRIB_POSITION, 3, GL_FLOAT, GL_FALSE, kQuadSize, (GLvoid*) offsetof(V3F_C4B_T2F, vertices));

            // colors
            glVertexAttribPointer(GLProgram::VERTEX_ATTRIB_COLOR, 4, GL_UNSIGNED_BYTE, GL_TRUE, kQuadSize, (GLvoid*) offsetof(V3F_C4B_T2F, colors));

            // tex coords
            glVertexAttribPointer(GLProgram::VERTEX_ATTRIB_TEX_COORD, 2, GL_FLOAT, GL_FALSE, kQuadSize, (GLvoid*) offsetof(V3F_C4B_T2F, texCoords));

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffersVBO_[1]);

            glDrawElements(GL_TRIANGLES, (GLsizei)numberOfQuads*6, GL_UNSIGNED_SHORT, (GLvoid*) (start*6*sizeof(indices_[0])));

            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        }

        do {
            // TODO
        } while(0);
    }
}
