#ifndef GLTEXTURE_H
#define GLTEXTURE_H

#include <string>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <queue>
#include <string>
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
    class GLShader;

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
        static void setDefaultAlphaPixelFormat(IMAGE::PixelFormat format);
        static IMAGE::PixelFormat getDefaultAlphaPixelFormat();
        typedef HData (*TextToTextureDataDef)(const char * text, const FontDefinition& textDefinition, TextAlign align, int &width, int &height, bool& hasPremultipliedAlpha);
        static TextToTextureDataDef getTextureDataForText;

    public:
        GLTexture();
        virtual ~GLTexture();

        void releaseGLTexture();

        bool initWithData(const void *data, uint64 dataLen, IMAGE::PixelFormat pixelFormat, int pixelsWide, int pixelsHigh, const MATH::Sizef& contentSize);
        bool initWithMipmaps(MipmapInfo* mipmaps, int mipmapsNum, IMAGE::PixelFormat pixelFormat, int pixelsWide, int pixelsHigh);

        bool updateWithData(const void *data,int offsetX,int offsetY,int width,int height);

        void drawAtPoint(const MATH::Vector2f& point);
        void drawInRect(const MATH::Rectf& rect);

        bool initWithImage(IMAGE::ImageObject * image);
        bool initWithImage(IMAGE::ImageObject * image, IMAGE::PixelFormat format);
        bool initWithString(const char *text,  const std::string &fontName, float fontSize, const MATH::Sizef& dimensions = MATH::Sizef(0, 0), TextHAlignment hAlignment = TextHAlignment::CENTER, TextVAlignment vAlignment = TextVAlignment::TOP);
        bool initWithString(const char *text, const FontDefinition& textDefinition);

        void setTexParameters(const TexParams& texParams);
        void setAntiAliasTexParameters();
        void setAliasTexParameters();

        void generateMipmap();

        const char* getStringForFormat() const;
        unsigned int getBitsPerPixelForFormat() const;
        unsigned int getBitsPerPixelForFormat(IMAGE::PixelFormat format) const;

        const MATH::Sizef& getContentSizeInPixels();

        bool hasPremultipliedAlpha() const;
        bool hasMipmaps() const;

        IMAGE::PixelFormat getPixelFormat() const;
        int getPixelsWidth() const;
        int getPixelsHight() const;
        GLuint getName() const;

        GLfloat getMaxS() const;
        void setMaxS(GLfloat maxS);

        GLfloat getMaxT() const;
        void setMaxT(GLfloat maxT);

        MATH::Sizef getContentSize() const;

        void setGLShader(GLShader* program);
        GLShader* getGLShader() const;

    protected:
        IMAGE::PixelFormat pixelFormat_;
        int pixelsWidth_;
        int pixelsHight_;
        GLuint name_;
        GLfloat maxS_;
        GLfloat maxT_;
        MATH::Sizef contentSize_;
        bool hasPremultipliedAlpha_;
        bool hasMipmaps_;
        GLShader* shaderProgram_;
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
