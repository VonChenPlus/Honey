#ifndef GLTEXTURE_H
#define GLTEXTURE_H

#include <string>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <queue>
#include <unordered_map>
#include <functional>
#include "BASE/HObject.h"
#include "BASE/HData.h"
#include "GRAPH/Types.h"
#include "GRAPH/UNITY3D/GLCommon.h"
#include "MATH/Vector.h"
#include "MATH/Rectangle.h"
#include "MATH/Size.h"
#include "IMAGE/ImageObject.h"

namespace GRAPH
{
    class Unity3DGLShaderSet;

    const IMAGE::PixelFormatInfoMapValue TexturePixelFormatInfoTablesValue[] =
    {
        IMAGE::PixelFormatInfoMapValue(IMAGE::ImageFormat::BGRA8888, IMAGE::ImageFormatInfo(GL_BGRA, GL_BGRA, GL_UNSIGNED_BYTE, 32, false, true)),
        IMAGE::PixelFormatInfoMapValue(IMAGE::ImageFormat::RGBA8888, IMAGE::ImageFormatInfo(GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, 32, false, true)),
        IMAGE::PixelFormatInfoMapValue(IMAGE::ImageFormat::RGBA4444, IMAGE::ImageFormatInfo(GL_RGBA, GL_RGBA, GL_UNSIGNED_SHORT_4_4_4_4, 16, false, true)),
        IMAGE::PixelFormatInfoMapValue(IMAGE::ImageFormat::RGB5A1, IMAGE::ImageFormatInfo(GL_RGBA, GL_RGBA, GL_UNSIGNED_SHORT_5_5_5_1, 16, false, true)),
        IMAGE::PixelFormatInfoMapValue(IMAGE::ImageFormat::RGB565, IMAGE::ImageFormatInfo(GL_RGB, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, 16, false, false)),
        IMAGE::PixelFormatInfoMapValue(IMAGE::ImageFormat::RGB888, IMAGE::ImageFormatInfo(GL_RGB, GL_RGB, GL_UNSIGNED_BYTE, 24, false, false)),
        IMAGE::PixelFormatInfoMapValue(IMAGE::ImageFormat::A8, IMAGE::ImageFormatInfo(GL_ALPHA, GL_ALPHA, GL_UNSIGNED_BYTE, 8, false, false)),
        IMAGE::PixelFormatInfoMapValue(IMAGE::ImageFormat::I8, IMAGE::ImageFormatInfo(GL_LUMINANCE, GL_LUMINANCE, GL_UNSIGNED_BYTE, 8, false, false)),
        IMAGE::PixelFormatInfoMapValue(IMAGE::ImageFormat::AI88, IMAGE::ImageFormatInfo(GL_LUMINANCE_ALPHA, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, 16, false, true)),
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
        bool initWithMipmaps(MipmapInfo* mipmaps, int mipmapsNum, IMAGE::ImageFormat pixelFormat, int pixelsWide, int pixelsHigh);

        bool updateWithData(const void *data,int offsetX,int offsetY,int width,int height);

        bool initWithImage(IMAGE::ImageObject * image);
        bool initWithImage(IMAGE::ImageObject * image, IMAGE::ImageFormat format);
        bool initWithString(const char *text,  const std::string &fontName, float fontSize, const MATH::Sizef& dimensions = MATH::Sizef(0, 0), TextHAlignment hAlignment = TextHAlignment::CENTER, TextVAlignment vAlignment = TextVAlignment::TOP);
        bool initWithString(const char *text, const FontDefinition& textDefinition);

        void setTexParameters(const TexParams& texParams);
        void setAntiAliasTexParameters();
        void setAliasTexParameters();

        void generateMipmap();

        const char* getStringForFormat() const;
        unsigned int getBitsPerPixelForFormat() const;
        unsigned int getBitsPerPixelForFormat(IMAGE::ImageFormat format) const;

        const MATH::Sizef& getContentSizeInPixels();

        bool hasPremultipliedAlpha() const;
        bool hasMipmaps() const;

        IMAGE::ImageFormat getPixelFormat() const;
        int getPixelsWidth() const;
        int getPixelsHight() const;
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
