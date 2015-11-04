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
#include "GRAPH/UNITY3D/Unity3d.h"
#include "MATH/Vector.h"
#include "MATH/Rectangle.h"
#include "MATH/Size.h"
#include "IMAGE/ImageObject.h"

namespace GRAPH
{
    class Unity3DGLShaderSet;

    const IMAGE::ImageFormatInfoMapValue TexturePixelFormatInfoTablesValue[] =
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

    struct MipmapInfo
    {
        unsigned char* address;
        int len;
        MipmapInfo()
            : address(nullptr)
            , len(0){}
    };

    class GLTexture : public HObject
    {
    public:
        struct TexParams
        {
            GLuint    minFilter;
            GLuint    magFilter;
            GLuint    wrapS;
            GLuint    wrapT;
        };

    public:
        typedef HData (*TextToTextureDataDef)(const char * text, const FontDefinition& textDefinition, TextAlign align, int &width, int &height, bool& hasPremultipliedAlpha);
        static TextToTextureDataDef getTextureDataForText;

    public:
        GLTexture();
        virtual ~GLTexture();

        void releaseGLTexture();

        bool initWithData(const void *data, uint64 dataLen, IMAGE::ImageFormat pixelFormat, int pixelsWide, int pixelsHigh, const MATH::Sizef& contentSize);
        bool initWithMipmaps(MipmapInfo* mipmaps, int mipLevels, IMAGE::ImageFormat pixelFormat, int pixelsWide, int pixelsHigh);

        bool updateWithData(const void *data,int offsetX,int offsetY,int width,int height);

        bool initWithImage(IMAGE::ImageObject * image);
        bool initWithImage(IMAGE::ImageObject * image, IMAGE::ImageFormat format);
        bool initWithString(const char *text,  const std::string &fontName, float fontSize, const MATH::Sizef& dimensions = MATH::Sizef(0, 0), TextHAlignment hAlignment = TextHAlignment::CENTER, TextVAlignment vAlignment = TextVAlignment::TOP);
        bool initWithString(const char *text, const FontDefinition& textDefinition);

        void setTexParameters(const TexParams& texParams);
        void setAntiAliasTexParameters();
        void setAliasTexParameters();

        void generateMipmap();

        const MATH::Sizef& getContentSizeInPixels();

        bool hasPremultipliedAlpha() const;
        bool hasMipmaps() const;

        GLuint getName() const;

        MATH::Sizef getContentSize() const;

    protected:
        IMAGE::ImageFormat pixelFormat_;
        int pixelsWidth_;
        int pixelsHight_;
        uint32 name_;
        MATH::Sizef contentSize_;
        bool hasPremultipliedAlpha_;
        bool hasMipmaps_;
        bool antialiasEnabled_;
    };

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

        int width() { return width_; }
        int height() { return height_; }

        const IMAGE::ImageFormatInfoMap &imageFormatInfoMap() override;

    private:
        GLuint target_;
        bool antialias_;
        GLuint texture_;
        IMAGE::ImageFormat imageFormat_;
        int width_;
        int height_;
        bool hasMipmaps_;
    };

    class TextureCache : public HObject
    {
    public:
        static TextureCache &getInstance();

        GLTexture* addImage(const std::string &filepath);
        GLTexture* addImage(IMAGE::ImageObject *image, const std::string &key);
        virtual void addImageAsync(const std::string &filepath, const std::function<void(GLTexture*)>& callback);

        virtual void unbindImageAsync(const std::string &filename);
        virtual void unbindAllImageAsync();

        bool reloadTexture(const std::string& fileName);

        void removeAllTextures();
        void removeUnusedTextures();
        void removeTexture(GLTexture* texture);
        void removeTextureForKey(const std::string &key);

        GLTexture* getTextureForKey(const std::string& key) const;
        const std::string getTextureFilePath(GLTexture* texture)const;

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
            AsyncStruct(const std::string& fn, std::function<void(GLTexture*)> f) : filename(fn), callback(f) {}

            std::string filename;
            std::function<void(GLTexture*)> callback;
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
        std::unordered_map<std::string, GLTexture*> textures_;
    };
}

#endif // GLTEXTURE_H
