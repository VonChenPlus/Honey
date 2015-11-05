#ifndef TEXTURECACHE_H
#define TEXTURECACHE_H

#include <string>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <queue>
#include <unordered_map>
#include "GRAPH/UNITY3D/Unity3D.h"

namespace GRAPH
{
    class TextureCache : public HObject
    {
    public:
        static TextureCache &getInstance();

        Unity3DTexture* addImage(const std::string &filepath);
        Unity3DTexture* addImage(IMAGE::ImageObject *image, const std::string &key);
        virtual void addImageAsync(const std::string &filepath, const std::function<void(Unity3DTexture*)>& callback);

        virtual void unbindImageAsync(const std::string &filename);
        virtual void unbindAllImageAsync();

        bool reloadTexture(const std::string& fileName);

        void removeAllTextures();
        void removeUnusedTextures();
        void removeTexture(Unity3DTexture* texture);
        void removeTextureForKey(const std::string &key);

        Unity3DTexture* getTextureForKey(const std::string& key) const;
        const std::string getTextureFilePath(Unity3DTexture* texture)const;

        void waitForQuit();

    private:
        TextureCache();
        virtual ~TextureCache();

        void addImageAsyncCallBack(float dt);
        void loadImageThread();

    protected:
        struct AsyncStruct
        {
        public:
            AsyncStruct(const std::string& fn, std::function<void(Unity3DTexture*)> f) : filename(fn), callback(f) {}

            std::string filename;
            std::function<void(Unity3DTexture*)> callback;
        };

        struct ImageInfo
        {
            AsyncStruct *asyncStruct;
            IMAGE::ImageObject *image;
        };

        std::thread* loadingThread_;
        std::deque<AsyncStruct*>* asyncStructQueue_;
        std::deque<ImageInfo*>* imageInfoQueue_;
        std::mutex asyncMutex_;
        std::mutex sleepMutex_;
        std::condition_variable sleepCondition_;
        bool needQuit_;
        int asyncRefCount_;
        std::unordered_map<std::string, Unity3DTexture*> textures_;
    };
}

#endif // TEXTURECACHE_H
