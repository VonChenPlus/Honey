#include "GRAPH/RENDERER/TextureCache.h"

#include <errno.h>
#include <stack>
#include <cctype>
#include <list>
#include "GRAPH/BASE/Director.h"
#include "UTILS/STRING/StringUtils.h"
#include "IO/FileUtils.h"
#include "GRAPH/BASE/Scheduler.h"

namespace GRAPH
{
    TextureCache * TextureCache::getInstance()
    {
        return Director::getInstance()->getTextureCache();
    }

    TextureCache::TextureCache()
    : _loadingThread(nullptr)
    , _asyncStructQueue(nullptr)
    , _imageInfoQueue(nullptr)
    , _needQuit(false)
    , _asyncRefCount(0)
    {
    }

    TextureCache::~TextureCache()
    {
        for( auto it=_textures.begin(); it!=_textures.end(); ++it)
            (it->second)->release();

        SAFE_DELETE(_loadingThread);
    }

    void TextureCache::destroyInstance()
    {
    }

    TextureCache * TextureCache::sharedTextureCache()
    {
        return Director::getInstance()->getTextureCache();
    }

    void TextureCache::purgeSharedTextureCache()
    {
    }

    std::string TextureCache::getDescription() const
    {
        return UTILS::STRING::StringFromFormat("<TextureCache | Number of textures = %d>", static_cast<int>(_textures.size()))
    }

    void TextureCache::addImageAsync(const std::string &path, const std::function<void(Texture2D*)>& callback)
    {
        Texture2D *texture = nullptr;

        std::string fullpath = FileUtils::getInstance()->fullPathForFilename(path);

        auto it = _textures.find(fullpath);
        if( it != _textures.end() )
            texture = it->second;

        if (texture != nullptr)
        {
            if (callback) callback(texture);
            return;
        }

        // check if file exists
        if ( fullpath.empty() || ! FileUtils::getInstance()->isFileExist( fullpath ) ) {
            if (callback) callback(nullptr);
            return;
        }

        // lazy init
        if (_asyncStructQueue == nullptr)
        {
            _asyncStructQueue = new (std::nothrow) deque<AsyncStruct*>();
            _imageInfoQueue = new (std::nothrow) deque<ImageInfo*>();

            // create a new thread to load images
            _loadingThread = new std::thread(&TextureCache::loadImage, this);

            _needQuit = false;
        }

        if (0 == _asyncRefCount)
        {
            Director::getInstance()->getScheduler()->schedule(CC_SCHEDULE_SELECTOR(TextureCache::addImageAsyncCallBack), this, 0, false);
        }

        ++_asyncRefCount;

        // generate async struct
        AsyncStruct *data = new (std::nothrow) AsyncStruct(fullpath, callback);

        // add async struct into queue
        _asyncMutex.lock();
        _asyncStructQueue->push_back(data);
        _asyncMutex.unlock();

        _sleepCondition.notify_one();
    }

    void TextureCache::unbindImageAsync(const std::string& filename)
    {
        std::string fullpath = FileUtils::getInstance()->fullPathForFilename(filename);

        _asyncMutex.lock();

        if (_asyncStructQueue && !_asyncStructQueue->empty())
        {
            for (auto it = _asyncStructQueue->begin(); it != _asyncStructQueue->end(); ++it)
            {
                if ((*it)->filename == fullpath)
                {
                    (*it)->callback = nullptr;
                }
            }
        }

        if (_imageInfoQueue && !_imageInfoQueue->empty())
        {
            for (auto it = _imageInfoQueue->begin(); it != _imageInfoQueue->end(); ++it)
            {
                if ((*it)->asyncStruct->filename == fullpath)
                {
                    (*it)->asyncStruct->callback = nullptr;
                }
            }
        }

        _asyncMutex.unlock();
    }

    void TextureCache::unbindAllImageAsync()
    {
        _asyncMutex.lock();
        if (_asyncStructQueue && !_asyncStructQueue->empty())
        {
            for (auto it = _asyncStructQueue->begin(); it != _asyncStructQueue->end(); ++it)
            {
                (*it)->callback = nullptr;
            }
        }
        if (_imageInfoQueue && !_imageInfoQueue->empty())
        {
            for (auto it = _imageInfoQueue->begin(); it != _imageInfoQueue->end(); ++it)
            {
                (*it)->asyncStruct->callback = nullptr;
            }
        }
        _asyncMutex.unlock();
    }

    void TextureCache::loadImage()
    {
        AsyncStruct *asyncStruct = nullptr;

        while (true)
        {
            _asyncMutex.lock();
            if (_asyncStructQueue->empty())
            {
                _asyncMutex.unlock();
                if (_needQuit) {
                    break;
                }
                else {
                    std::unique_lock<std::mutex> lk(_sleepMutex);
                    _sleepCondition.wait(lk);
                    continue;
                }
            }
            else
            {
                asyncStruct = _asyncStructQueue->front();
                _asyncMutex.unlock();
            }

            Image *image = nullptr;
            bool generateImage = false;

            auto it = _textures.find(asyncStruct->filename);
            if( it == _textures.end() )
            {
               ImageInfo *imageInfo;
               size_t pos = 0;
               _asyncMutex.lock();
               size_t infoSize = _imageInfoQueue->size();
               for (; pos < infoSize; pos++)
               {
                   imageInfo = (*_imageInfoQueue)[pos];
                   if(imageInfo->asyncStruct->filename.compare(asyncStruct->filename) == 0)
                       break;
               }
               _asyncMutex.unlock();
               if(infoSize == 0 || pos == infoSize)
                   generateImage = true;
            }

            if (generateImage)
            {
                const std::string& filename = asyncStruct->filename;
                // generate image
                image = new (std::nothrow) Image();
                if (image && !image->initWithImageFileThreadSafe(filename))
                {
                    SAFE_RELEASE(image);
                    _asyncMutex.lock();
                    _asyncStructQueue->pop_front();
                    _asyncMutex.unlock();
                    continue;
                }
            }

            // generate image info
            ImageInfo *imageInfo = new (std::nothrow) ImageInfo();
            imageInfo->asyncStruct = asyncStruct;
            imageInfo->image = image;

            // put the image info into the queue
            _asyncMutex.lock();
            _asyncStructQueue->pop_front();
            _imageInfoQueue->push_back(imageInfo);
            _asyncMutex.unlock();
        }

        if(_asyncStructQueue != nullptr)
        {
            delete _asyncStructQueue;
            _asyncStructQueue = nullptr;
            delete _imageInfoQueue;
            _imageInfoQueue = nullptr;
        }
    }

    void TextureCache::addImageAsyncCallBack(float dt)
    {
        // the image is generated in loading thread
        std::deque<ImageInfo*> *imagesQueue = _imageInfoQueue;

        _asyncMutex.lock();
        if (imagesQueue->empty())
        {
            _asyncMutex.unlock();
        }
        else
        {
            ImageInfo *imageInfo = imagesQueue->front();
            imagesQueue->pop_front();
            _asyncMutex.unlock();

            AsyncStruct *asyncStruct = imageInfo->asyncStruct;
            Image *image = imageInfo->image;

            const std::string& filename = asyncStruct->filename;

            Texture2D *texture = nullptr;
            if (image)
            {
                // generate texture in render thread
                texture = new (std::nothrow) Texture2D();

                texture->initWithImage(image);
                //parse 9-patch info
                this->parseNinePatchImage(image, texture, filename);

                // cache the texture. retain it, since it is added in the map
                _textures.insert( std::make_pair(filename, texture) );
                texture->retain();

                texture->autorelease();
            }
            else
            {
                auto it = _textures.find(asyncStruct->filename);
                if(it != _textures.end())
                    texture = it->second;
            }

            if (asyncStruct->callback)
            {
                asyncStruct->callback(texture);
            }

            if(image)
            {
                image->release();
            }
            delete asyncStruct;
            delete imageInfo;

            --_asyncRefCount;
            if (0 == _asyncRefCount)
            {
                Director::getInstance()->getScheduler()->unschedule(CC_SCHEDULE_SELECTOR(TextureCache::addImageAsyncCallBack), this);
            }
        }
    }

    Texture2D * TextureCache::addImage(const std::string &path)
    {
        Texture2D * texture = nullptr;
        Image* image = nullptr;
        // Split up directory and filename
        // MUTEX:
        // Needed since addImageAsync calls this method from a different thread

        std::string fullpath = IO::FileUtils::getInstance().fullPathForFilename(path);
        if (fullpath.size() == 0)
        {
            return nullptr;
        }
        auto it = _textures.find(fullpath);
        if( it != _textures.end() )
            texture = it->second;

        if (! texture)
        {
            // all images are handled by UIImage except PVR extension that is handled by our own handler
            do
            {
                image = new (std::nothrow) Image();
                CC_BREAK_IF(nullptr == image);

                bool bRet = image->initWithImageFile(fullpath);
                CC_BREAK_IF(!bRet);

                texture = new (std::nothrow) Texture2D();

                if( texture && texture->initWithImage(image) )
                {
                    // texture already retained, no need to re-retain it
                    _textures.insert( std::make_pair(fullpath, texture) );

                    //parse 9-patch info
                    this->parseNinePatchImage(image, texture, path);
                }
            } while (0);
        }

        SAFE_RELEASE(image);

        return texture;
    }

    void TextureCache::parseNinePatchImage(cocos2d::Image *image, cocos2d::Texture2D *texture,const std::string& path)
    {
        if(NinePatchImageParser::isNinePatchImage(path))
        {
            Rect frameRect = Rect(0,0,image->getWidth(), image->getHeight());
            NinePatchImageParser parser(image, frameRect, false);
            texture->addSpriteFrameCapInset(nullptr, parser.parseCapInset());
        }

    }

    Texture2D* TextureCache::addImage(Image *image, const std::string &key)
    {
        Texture2D * texture = nullptr;

        do
        {
            auto it = _textures.find(key);
            if( it != _textures.end() ) {
                texture = it->second;
                break;
            }

            // prevents overloading the autorelease pool
            texture = new (std::nothrow) Texture2D();
            texture->initWithImage(image);

            if(texture)
            {
                _textures.insert( std::make_pair(key, texture) );
                texture->retain();

                texture->autorelease();
            }
        } while (0);

        return texture;
    }

    bool TextureCache::reloadTexture(const std::string& fileName)
    {
        Texture2D * texture = nullptr;
        Image * image = nullptr;

        std::string fullpath = FileUtils::getInstance()->fullPathForFilename(fileName);
        if (fullpath.size() == 0)
        {
            return false;
        }

        auto it = _textures.find(fullpath);
        if (it != _textures.end()) {
            texture = it->second;
        }

        bool ret = false;
        if (! texture) {
            texture = this->addImage(fullpath);
            ret = (texture != nullptr);
        }
        else
        {
            do {
                image = new (std::nothrow) Image();
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

    void TextureCache::removeAllTextures()
    {
        for( auto it=_textures.begin(); it!=_textures.end(); ++it ) {
            (it->second)->release();
        }
        _textures.clear();
    }

    void TextureCache::removeUnusedTextures()
    {
        for( auto it=_textures.cbegin(); it!=_textures.cend(); /* nothing */) {
            Texture2D *tex = it->second;
            if( tex->getReferenceCount() == 1 ) {
                tex->release();
                _textures.erase(it++);
            }
            else {
                ++it;
            }
        }
    }

    void TextureCache::removeTexture(Texture2D* texture)
    {
        if( ! texture )
        {
            return;
        }

        for( auto it=_textures.cbegin(); it!=_textures.cend(); /* nothing */ ) {
            if( it->second == texture ) {
                texture->release();
                _textures.erase(it++);
                break;
            } else
                ++it;
        }
    }

    void TextureCache::removeTextureForKey(const std::string &textureKeyName)
    {
        std::string key = textureKeyName;
        auto it = _textures.find(key);

        if( it == _textures.end() ) {
            key = IO::FileUtils::getInstance().fullPathForFilename(textureKeyName);
            it = _textures.find(key);
        }

        if( it != _textures.end() ) {
            (it->second)->release();
            _textures.erase(it);
        }
    }

    Texture2D* TextureCache::getTextureForKey(const std::string &textureKeyName) const
    {
        std::string key = textureKeyName;
        auto it = _textures.find(key);

        if( it == _textures.end() ) {
            key = FileUtils::getInstance()->fullPathForFilename(textureKeyName);
            it = _textures.find(key);
        }

        if( it != _textures.end() )
            return it->second;
        return nullptr;
    }

    void TextureCache::reloadAllTextures()
    {
        //will do nothing
    }

    const std::string TextureCache::getTextureFilePath( cocos2d::Texture2D *texture )const
    {
        for(auto& item : _textures)
        {
            if(item.second == texture)
            {
                return item.first;
                break;
            }
        }
        return "";
    }

    void TextureCache::waitForQuit()
    {
        // notify sub thread to quick
        _needQuit = true;
        _sleepCondition.notify_one();
        if (_loadingThread) _loadingThread->join();
    }

    std::string TextureCache::getCachedTextureInfo() const
    {
        std::string buffer;
        char buftmp[4096];

        unsigned int count = 0;
        unsigned int totalBytes = 0;

        for( auto it = _textures.begin(); it != _textures.end(); ++it ) {

            memset(buftmp,0,sizeof(buftmp));


            Texture2D* tex = it->second;
            unsigned int bpp = tex->getBitsPerPixelForFormat();
            // Each texture takes up width * height * bytesPerPixel bytes.
            auto bytes = tex->getPixelsWide() * tex->getPixelsHigh() * bpp / 8;
            totalBytes += bytes;
            count++;
            snprintf(buftmp,sizeof(buftmp)-1,"\"%s\" rc=%lu id=%lu %lu x %lu @ %ld bpp => %lu KB\n",
                   it->first.c_str(),
                   (long)tex->getReferenceCount(),
                   (long)tex->getName(),
                   (long)tex->getPixelsWide(),
                   (long)tex->getPixelsHigh(),
                   (long)bpp,
                   (long)bytes / 1024);

            buffer += buftmp;
        }

        snprintf(buftmp, sizeof(buftmp)-1, "TextureCache dumpDebugInfo: %ld textures, for %lu KB (%.2f MB)\n", (long)count, (long)totalBytes / 1024, totalBytes / (1024.0f*1024.0f));
        buffer += buftmp;

        return buffer;
    }
}
