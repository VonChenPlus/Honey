#ifndef TEXTURE2D_H
#define TEXTURE2D_H

#include <string>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <queue>
#include <string>
#include <unordered_map>
#include <functional>
#include "BASE/HObject.h"
#include "GRAPH/Types.h"
#include "GRAPH/RENDERER/GLCommon.h"
#include "MATH/Vector.h"
#include "MATH/Rectangle.h"
#include "MATH/Size.h"
#include "IMAGE/TinyImage.h"

namespace GRAPH
{
    class GLProgram;

    struct MipmapInfo
    {
        unsigned char* address;
        int len;
        MipmapInfo()
            : address(nullptr)
            , len(0){}
    };

    class Texture2D : public HObject
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

    public:
        Texture2D();
        virtual ~Texture2D();

        void releaseGLTexture();

        bool initWithData(const void *data, ssize_t dataLen, IMAGE::PixelFormat pixelFormat, int pixelsWide, int pixelsHigh, const MATH::Sizef& contentSize);
        bool initWithMipmaps(MipmapInfo* mipmaps, int mipmapsNum, IMAGE::PixelFormat pixelFormat, int pixelsWide, int pixelsHigh);

        bool updateWithData(const void *data,int offsetX,int offsetY,int width,int height);

        void drawAtPoint(const MATH::Vector2f& point);
        void drawInRect(const MATH::Rectf& rect);

        bool initWithImage(IMAGE::TinyImage * image);
        bool initWithImage(IMAGE::TinyImage * image, IMAGE::PixelFormat format);
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

        void setGLProgram(GLProgram* program);
        GLProgram* getGLProgram() const;

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
        GLProgram* shaderProgram_;
        bool antialiasEnabled_;
    };

    class TextureCache : public HObject
    {
    public:
        TextureCache();
        virtual ~TextureCache();

        Texture2D* addImage(const std::string &filepath);
        Texture2D* addImage(IMAGE::TinyImage *image, const std::string &key);
        virtual void addImageAsync(const std::string &filepath, const std::function<void(Texture2D*)>& callback);

        virtual void unbindImageAsync(const std::string &filename);
        virtual void unbindAllImageAsync();

        bool reloadTexture(const std::string& fileName);

        void removeAllTextures();
        void removeUnusedTextures();
        void removeTexture(Texture2D* texture);
        void removeTextureForKey(const std::string &key);

        Texture2D* getTextureForKey(const std::string& key) const;
        const std::string getTextureFilePath(Texture2D* texture)const;

        void waitForQuit();

    private:
        void addImageAsyncCallBack(float dt);
        void loadImageThread();

    protected:
        struct AsyncStruct
        {
        public:
            AsyncStruct(const std::string& fn, std::function<void(Texture2D*)> f) : filename(fn), callback(f) {}

            std::string filename;
            std::function<void(Texture2D*)> callback;
        };

        struct ImageInfo
        {
            AsyncStruct *asyncStruct;
            IMAGE::TinyImage *image;
        };

        std::thread* loadingThread_;
        std::deque<AsyncStruct*>* asyncStructQueue_;
        std::deque<ImageInfo*>* imageInfoQueue_;
        std::mutex asyncMutex_;
        std::mutex sleepMutex_;
        std::condition_variable sleepCondition_;
        bool needQuit_;
        int asyncRefCount_;
        std::unordered_map<std::string, Texture2D*> textures_;
    };

    class TextureAtlas : public HObject
    {
    public:
        static TextureAtlas* create(const std::string& file , ssize_t capacity);
        static TextureAtlas* createWithTexture(Texture2D *texture, ssize_t capacity);

        TextureAtlas();
        virtual ~TextureAtlas();

        bool initWithFile(const std::string& file, ssize_t capacity);
        bool initWithTexture(Texture2D *texture, ssize_t capacity);

        void updateQuad(V3F_C4B_T2F_Quad* quad, ssize_t index);
        void insertQuad(V3F_C4B_T2F_Quad* quad, ssize_t index);
        void insertQuads(V3F_C4B_T2F_Quad* quads, ssize_t index, ssize_t amount);
        void insertQuadFromIndex(ssize_t fromIndex, ssize_t newIndex);

        void removeQuadAtIndex(ssize_t index);
        void removeQuadsAtIndex(ssize_t index, ssize_t amount);
        void removeAllQuads();

        bool resizeCapacity(ssize_t capacity);

        void increaseTotalQuadsWith(ssize_t amount);
        void moveQuadsFromIndex(ssize_t oldIndex, ssize_t amount, ssize_t newIndex);
        void moveQuadsFromIndex(ssize_t index, ssize_t newIndex);
        void fillWithEmptyQuadsFromIndex(ssize_t index, ssize_t amount);

        void drawNumberOfQuads(ssize_t n);
        void drawNumberOfQuads(ssize_t numberOfQuads, ssize_t start);
        void drawQuads();

        inline bool isDirty(void) { return dirty_; }
        inline void setDirty(bool bDirty) { dirty_ = bDirty; }

        ssize_t getTotalQuads() const;
        ssize_t getCapacity() const;
        Texture2D* getTexture() const;
        void setTexture(Texture2D* texture);
        V3F_C4B_T2F_Quad* getQuads();

        void setQuads(V3F_C4B_T2F_Quad* quads);

    private:
        void renderCommand();

        void setupIndices();
        void mapBuffers();
        void setupVBO();

    protected:
        GLushort* indices_;
        GLuint  buffersVBO_[2]; //0: vertex  1: indices
        bool    dirty_; //indicates whether or not the array buffer of the VBO needs to be updated
        ssize_t totalQuads_;
        ssize_t capacity_;
        Texture2D* texture_;
        V3F_C4B_T2F_Quad* quads_;
    };
}

#endif // TEXTURE2D_H
