#include "BASE/HData.h"
#include "GRAPH/Director.h"
#include "GRAPH/Scheduler.h"
#include "GRAPH/UNITY3D/GLTexture.h"
#include "GRAPH/UNITY3D/Unity3DGLShader.h"
#include "GRAPH/UNITY3D/GLStateCache.h"
#include "IO/FileUtils.h"

namespace GRAPH
{
    GLTexture::TextToTextureDataDef GLTexture::getTextureDataForText = nullptr;

    GLTexture::GLTexture()
        : pixelFormat_(IMAGE::PixelFormat::DEFAULT)
        , pixelsWidth_(0)
        , pixelsHight_(0)
        , name_(0)
        , hasPremultipliedAlpha_(false)
        , hasMipmaps_(false)
        , antialiasEnabled_(true) {
    }

    GLTexture::~GLTexture() {
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

        hasPremultipliedAlpha_ = false;
        hasMipmaps_ = mipmapsNum > 1;

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
        return initWithImage(image, IMAGE::PixelFormat::DEFAULT);
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

        IMAGE::PixelFormat pixelFormat = IMAGE::PixelFormat::DEFAULT;
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

    TextureCache &TextureCache::getInstance() {
        static TextureCache instance;
        return instance;
    }

    TextureCache::TextureCache()
        : loadingThread_(nullptr)
        , asyncStructQueue_(nullptr)
        , imageInfoQueue_(nullptr)
        , needQuit_(false)
        , asyncRefCount_(0) {
    }

    TextureCache::~TextureCache() {
        waitForQuit();

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
}
