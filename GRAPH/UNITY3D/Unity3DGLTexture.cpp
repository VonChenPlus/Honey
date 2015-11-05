#include "BASE/HData.h"
#include "GRAPH/Director.h"
#include "GRAPH/Scheduler.h"
#include "GRAPH/UNITY3D/Unity3DGLTexture.h"
#include "GRAPH/UNITY3D/Unity3DGLShader.h"
#include "GRAPH/UNITY3D/GLStateCache.h"
#include "IMAGE/ImageConvert.h"
#include "IO/FileUtils.h"

namespace GRAPH
{
    Unity3DGLTexture::Unity3DGLTexture()
        : target_(0)
        , hasMipmaps_(false)
        , antialias_(true) {
    }

    Unity3DGLTexture::~Unity3DGLTexture() {
        if (texture_) {
            GLStateCache::DeleteTexture(texture_);
        }
        texture_ = 0;
    }

    void Unity3DGLTexture::create(U3DTextureType type, bool antialias) {
        target_ = textureToGL[type];
        antialias_ = antialias;
    }

    bool Unity3DGLTexture::initWithMipmaps(U3DMipmap* mipmaps, int mipLevels, IMAGE::ImageFormat imageFormat, uint32 imageWidth, uint32 imageHeight) {
        if (mipLevels <= 0) {
            return false;
        }

        if (imageFormatInfoMap().find(imageFormat) == imageFormatInfoMap().end()) {
            return false;
        }

        const IMAGE::ImageFormatInfo& info = imageFormatInfoMap().at(imageFormat);

        //Set the row align only when mipmapsNum == 1 and the data is uncompressed
        if (mipLevels == 1 && !info.compressed) {
            unsigned int bytesPerRow = imageWidth * info.bpp / 8;

            if (bytesPerRow % 8 == 0)
            {
                glPixelStorei(GL_UNPACK_ALIGNMENT, 8);
            }
            else if (bytesPerRow % 4 == 0)
            {
                glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
            }
            else if (bytesPerRow % 2 == 0)
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

        if (texture_ != 0) {
            GLStateCache::DeleteTexture(texture_);
            texture_ = 0;
        }

        glGenTextures(1, &texture_);
        GLStateCache::BindTexture2D(texture_);

        if (mipLevels == 1) {
            glTexParameteri(target_, GL_TEXTURE_MIN_FILTER, antialias_ ? GL_LINEAR : GL_NEAREST);
        }
        else {
            glTexParameteri(target_, GL_TEXTURE_MIN_FILTER, antialias_ ? GL_LINEAR_MIPMAP_NEAREST : GL_NEAREST_MIPMAP_NEAREST);
        }

        glTexParameteri(target_, GL_TEXTURE_MAG_FILTER, antialias_ ? GL_LINEAR : GL_NEAREST);
        glTexParameteri(target_, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(target_, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        // Specify OpenGL texture image
        int width = imageWidth;
        int height = imageHeight;

        for (int i = 0; i < mipLevels; ++i) {
            unsigned char *data = mipmaps[i].address;
            GLsizei datalen = mipmaps[i].length;

            if (info.compressed) {
                glCompressedTexImage2D(target_, i, info.internalFormat, (GLsizei) width, (GLsizei) height, 0, datalen, data);
            }
            else {
                glTexImage2D(target_, i, info.internalFormat, (GLsizei) width, (GLsizei) height, 0, info.format, info.type, data);
            }

            width = MATH::MATH_MAX(width >> 1, 1);
            height = MATH::MATH_MAX(height >> 1, 1);
        }

        imageFormat_ = imageFormat;
        width_ = width;
        height_ = height;
        premultipliedAlpha_ = false;
        hasMipmaps_ = mipLevels > 1;

        return true;
    }

    bool Unity3DGLTexture::updateWithData(const void *data, int offsetX, int offsetY, int width, int height) {
        if (texture_) {
            GLStateCache::BindTexture2D(texture_);
            const IMAGE::ImageFormatInfo& info = imageFormatInfoMap().at(imageFormat_);
            glTexSubImage2D(target_, 0, offsetX, offsetY, width, height, info.format, info.type, data);
            return true;
        }
        return false;
    }

    void Unity3DGLTexture::setAliasTexParameters() {
        if (texture_ == 0) {
            return;
        }

        GLStateCache::BindTexture2D(texture_);

        if (!hasMipmaps_) {
            glTexParameteri(target_, GL_TEXTURE_MIN_FILTER, antialias_ ? GL_NEAREST : GL_LINEAR);
        }
        else {
            glTexParameteri(target_, GL_TEXTURE_MIN_FILTER, antialias_ ? GL_NEAREST_MIPMAP_NEAREST : GL_LINEAR_MIPMAP_NEAREST);
        }

        glTexParameteri(target_, GL_TEXTURE_MAG_FILTER, antialias_ ? GL_NEAREST : GL_LINEAR);
    }

    void Unity3DGLTexture::autoGenMipmaps() {
        GLStateCache::BindTexture2D(texture_);
        glGenerateMipmap(target_);
        hasMipmaps_ = true;
    }

    const IMAGE::ImageFormatInfoMap &Unity3DGLTexture::imageFormatInfoMap() {
        static const IMAGE::ImageFormatInfoMapValue TexturePixelFormatInfoTablesValue [] =
        {
            IMAGE::ImageFormatInfoMapValue(IMAGE::ImageFormat::BGRA8888, IMAGE::ImageFormatInfo(GL_BGRA, GL_BGRA, GL_UNSIGNED_BYTE, 32, false, true)),
            IMAGE::ImageFormatInfoMapValue(IMAGE::ImageFormat::RGBA8888, IMAGE::ImageFormatInfo(GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, 32, false, true)),
            IMAGE::ImageFormatInfoMapValue(IMAGE::ImageFormat::RGBA4444, IMAGE::ImageFormatInfo(GL_RGBA, GL_RGBA, GL_UNSIGNED_SHORT_4_4_4_4, 16, false, true)),
            IMAGE::ImageFormatInfoMapValue(IMAGE::ImageFormat::RGB5A1, IMAGE::ImageFormatInfo(GL_RGBA, GL_RGBA, GL_UNSIGNED_SHORT_5_5_5_1, 16, false, true)),
            IMAGE::ImageFormatInfoMapValue(IMAGE::ImageFormat::RGB565, IMAGE::ImageFormatInfo(GL_RGB, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, 16, false, false)),
            IMAGE::ImageFormatInfoMapValue(IMAGE::ImageFormat::RGB888, IMAGE::ImageFormatInfo(GL_RGB, GL_RGB, GL_UNSIGNED_BYTE, 24, false, false)),
            IMAGE::ImageFormatInfoMapValue(IMAGE::ImageFormat::A8, IMAGE::ImageFormatInfo(GL_ALPHA, GL_ALPHA, GL_UNSIGNED_BYTE, 8, false, false)),
            IMAGE::ImageFormatInfoMapValue(IMAGE::ImageFormat::I8, IMAGE::ImageFormatInfo(GL_LUMINANCE, GL_LUMINANCE, GL_UNSIGNED_BYTE, 8, false, false)),
            IMAGE::ImageFormatInfoMapValue(IMAGE::ImageFormat::AI88, IMAGE::ImageFormatInfo(GL_LUMINANCE_ALPHA, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, 16, false, true)),
        };

        static const IMAGE::ImageFormatInfoMap ImageFormatInfoTables(TexturePixelFormatInfoTablesValue,
            TexturePixelFormatInfoTablesValue + sizeof(TexturePixelFormatInfoTablesValue) / sizeof(TexturePixelFormatInfoTablesValue[0]));

        return ImageFormatInfoTables;
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

    void TextureCache::addImageAsync(const std::string &path, const std::function<void(Unity3DTexture*)>& callback) {
        Unity3DTexture *texture = nullptr;

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

            Unity3DTexture *texture = nullptr;
            if (image) {
                // generate texture in render thread
                texture = Unity3DCreator::CreateTexture();

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

    Unity3DTexture * TextureCache::addImage(const std::string &path) {
        Unity3DTexture * texture = nullptr;
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

                texture = Unity3DCreator::CreateTexture();

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

    Unity3DTexture* TextureCache::addImage(IMAGE::ImageObject *image, const std::string &key) {
        Unity3DTexture * texture = nullptr;

        do {
            auto it = textures_.find(key);
            if( it != textures_.end() ) {
                texture = it->second;
                break;
            }

            // prevents overloading the autorelease pool
            texture = Unity3DCreator::CreateTexture();
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
        Unity3DTexture * texture = nullptr;
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
            Unity3DTexture *tex = it->second;
            if( tex->getReferenceCount() == 1 ) {
                tex->release();
                textures_.erase(it++);
            }
            else {
                ++it;
            }
        }
    }

    void TextureCache::removeTexture(Unity3DTexture* texture) {
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

    Unity3DTexture* TextureCache::getTextureForKey(const std::string &textureKeyName) const {
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

    const std::string TextureCache::getTextureFilePath(Unity3DTexture *texture)const {
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
