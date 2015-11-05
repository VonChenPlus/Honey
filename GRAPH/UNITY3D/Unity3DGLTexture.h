#ifndef GLTEXTURE_H
#define GLTEXTURE_H

#include <string>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <queue>
#include <unordered_map>
#include <functional>
#include "BASE/HData.h"
#include "GRAPH/Types.h"
#include "GRAPH/UNITY3D/Unity3D.h"
#include "MATH/Vector.h"
#include "MATH/Rectangle.h"
#include "MATH/Size.h"
#include "IMAGE/ImageObject.h"

namespace GRAPH
{
    static const uint32 textureToGL [] =
    {
        GL_TEXTURE_1D,
        GL_TEXTURE_2D,
        GL_TEXTURE_3D,
        GL_TEXTURE_CUBE_MAP,
        GL_TEXTURE_1D_ARRAY,
        GL_TEXTURE_2D_ARRAY,
        GL_NONE
    };

    class Unity3DGLTexture final : public Unity3DTexture
    {
    public:
        Unity3DGLTexture();
        ~Unity3DGLTexture();

        void create(U3DTextureType type, bool antialias = true) override;

        bool initWithMipmaps(U3DMipmap* mipmaps, int mipLevels, IMAGE::ImageFormat imageFormat, uint32 imageWidth, uint32 imageHeight) override;
        bool updateWithData(const void *data, int offsetX, int offsetY, int width, int height) override;

        void setAliasTexParameters() override;
        void autoGenMipmaps() override;

        bool hasMipmaps() const override { return hasMipmaps_; }

        const IMAGE::ImageFormatInfoMap &imageFormatInfoMap() override;

    private:
        GLuint target_;
        bool antialias_;
        IMAGE::ImageFormat imageFormat_;
        bool hasMipmaps_;
    };

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

#endif // GLTEXTURE_H
