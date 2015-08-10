#ifndef TEXTURECACHE_H
#define TEXTURECACHE_H

#include <string>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <queue>
#include <string>
#include <unordered_map>
#include <functional>

#include "BASE/HObject.h"
#include "GRAPH/RENDERER/Texture2D.h"

namespace GRAPH
{
    class TextureCache : public HObject
    {
    public:
        /**
         * @js ctor
         */
        TextureCache();
        /**
         * @js NA
         * @lua NA
         */
        virtual ~TextureCache();

        /** Returns a Texture2D object given an filename.
        * If the filename was not previously loaded, it will create a new Texture2D.
        * Object and it will return it. It will use the filename as a key.
        * Otherwise it will return a reference of a previously loaded image.
        * Supported image extensions: .png, .bmp, .tiff, .jpeg, .pvr.
         @param filepath A null terminated string.
        */
        Texture2D* addImage(const std::string &filepath);

        /** Returns a Texture2D object given a file image.
        * If the file image was not previously loaded, it will create a new Texture2D object and it will return it.
        * Otherwise it will load a texture in a new thread, and when the image is loaded, the callback will be called with the Texture2D as a parameter.
        * The callback will be called from the main thread, so it is safe to create any cocos2d object from the callback.
        * Supported image extensions: .png, .jpg
         @param filepath A null terminated string.
         @param callback A callback function would be inovked after the image is loaded.
         @since v0.8
        */
        virtual void addImageAsync(const std::string &filepath, const std::function<void(Texture2D*)>& callback);

        /** Unbind a specified bound image asynchronous callback.
         * In the case an object who was bound to an image asynchronous callback was destroyed before the callback is invoked,
         * the object always need to unbind this callback manually.
         * @param filename It's the related/absolute path of the file image.
         * @since v3.1
         */
        virtual void unbindImageAsync(const std::string &filename);

        /** Unbind all bound image asynchronous load callbacks.
         * @since v3.1
         */
        virtual void unbindAllImageAsync();

        /** Returns a Texture2D object given an Image.
        * If the image was not previously loaded, it will create a new Texture2D object and it will return it.
        * Otherwise it will return a reference of a previously loaded image.
        * @param key The "key" parameter will be used as the "key" for the cache.
        * If "key" is nil, then a new texture will be created each time.
        */
        Texture2D* addImage(IMAGE::Image *image, const std::string &key);

        /** Returns an already created texture. Returns nil if the texture doesn't exist.
        @param key It's the related/absolute path of the file image.
        @since v0.99.5
        */
        Texture2D* getTextureForKey(const std::string& key) const;

        /** Reload texture from the image file.
        * If the file image hasn't loaded before, load it.
        * Otherwise the texture will be reloaded from the file image.
        * @param fileName It's the related/absolute path of the file image.
        * @return True if the reloading is succeed, otherwise return false.
        */
        bool reloadTexture(const std::string& fileName);

        /** Purges the dictionary of loaded textures.
        * Call this method if you receive the "Memory Warning".
        * In the short term: it will free some resources preventing your app from being killed.
        * In the medium term: it will allocate more resources.
        * In the long term: it will be the same.
        */
        void removeAllTextures();

        /** Removes unused textures.
        * Textures that have a retain count of 1 will be deleted.
        * It is convenient to call this method after when starting a new Scene.
        * @since v0.8
        */
        void removeUnusedTextures();

        /** Deletes a texture from the cache given a texture.
        */
        void removeTexture(Texture2D* texture);

        /** Deletes a texture from the cache given a its key name.
        @param key It's the related/absolute path of the file image.
        @since v0.99.4
        */
        void removeTextureForKey(const std::string &key);

        //Wait for texture cahe to quit befor destroy instance.
        /**Called by director, please do not called outside.*/
        void waitForQuit();

        /**
         * Get the file path of the texture
         *
         * @param texture A Texture2D object pointer.
         *
         * @return The full path of the file.
         */
        const std::string getTextureFilePath(Texture2D* texture)const;

    private:
        void addImageAsyncCallBack(float dt);
        void loadImage();

    public:
        struct AsyncStruct
        {
        public:
            AsyncStruct(const std::string& fn, std::function<void(Texture2D*)> f) : filename(fn), callback(f) {}

            std::string filename;
            std::function<void(Texture2D*)> callback;
        };

    protected:
        typedef struct _ImageInfo
        {
            AsyncStruct *asyncStruct;
            IMAGE::Image        *image;
        } ImageInfo;

        std::thread* _loadingThread;

        std::deque<AsyncStruct*>* _asyncStructQueue;
        std::deque<ImageInfo*>* _imageInfoQueue;

        std::mutex _asyncMutex;

        std::mutex _sleepMutex;
        std::condition_variable _sleepCondition;

        bool _needQuit;

        int _asyncRefCount;

        std::unordered_map<std::string, Texture2D*> _textures;
    };
}

#endif // TEXTURECACHE_H

