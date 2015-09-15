#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include <set>
#include "BASE/HObject.h"
#include "GRAPH/Color.h"
#include "GRAPH/RENDERER/GLCommon.h"
#include "GRAPH/RENDERER/Texture2D.h"

namespace GRAPH
{
    class RenderTargetBase : public HObject
    {
    public:
        enum class Type
        {
            RenderBuffer,
            Texture2D,
        };

    protected:
        RenderTargetBase();
        virtual ~RenderTargetBase();
        bool init(unsigned int width, unsigned int height);

    public:
        virtual Texture2D* getTexture() const { return nullptr; }
        virtual GLuint getBuffer() const { return 0; }

        unsigned int getWidth() const { return width_; }
        unsigned int getHeight() const { return height_; }
        Type getType() const { return type_; }

    protected:
        Type type_;
        unsigned int width_;
        unsigned int height_;
    };

    class RenderTarget : public RenderTargetBase
    {
    public:
        static RenderTarget* create(unsigned int width, unsigned int height, IMAGE::PixelFormat format = IMAGE::PixelFormat::RGBA8888);

        bool init(unsigned int width, unsigned int height, IMAGE::PixelFormat format);

        virtual Texture2D* getTexture() const { return texture_; }
    public:
        RenderTarget();
        virtual ~RenderTarget();

    protected:
        Texture2D* texture_;
    };

    class RenderTargetRenderBuffer : public RenderTargetBase
    {
    public:
        static RenderTargetRenderBuffer* create(unsigned int width, unsigned int height);

        bool init(unsigned int width, unsigned int height);

        virtual GLuint getBuffer() const { return colorBuffer_; }

    public:
        RenderTargetRenderBuffer();
        virtual ~RenderTargetRenderBuffer();

    protected:
        GLenum format_;
        GLuint colorBuffer_;
    };

    class RenderTargetDepthStencil : public RenderTargetBase
    {
    public:
        static RenderTargetDepthStencil* create(unsigned int width, unsigned int height);

        bool init(unsigned int width, unsigned int height);

        virtual GLuint getBuffer() const { return depthStencilBuffer_; }

    public:
        RenderTargetDepthStencil();
        virtual ~RenderTargetDepthStencil();

    protected:
        GLuint depthStencilBuffer_;
    };

    class FrameBuffer : public HObject
    {
    public:
        static FrameBuffer* create(uint8_t fid, unsigned int width, unsigned int height);

        bool init(uint8_t fid, unsigned int width, unsigned int height);

    public:
        GLuint getFBO() const { return frameBufferObject_; }
        GLuint getFID() const { return frameBufferId_; }
        void clearFBO();
        void applyFBO();
        void setClearColor(const Color4F& color) { clearColor_ = color;}
        void setClearDepth(float depth) { clearDepth_ = depth; }
        void setClearStencil(int8_t stencil) { clearStencil_ = stencil; }
        const Color4F& getClearColor() const { return clearColor_; }
        float getClearDepth() const { return clearDepth_; }
        int8_t getClearStencil() const { return clearStencil_; }

        RenderTargetBase* getRenderTarget() const { return rtBase_; }
        RenderTargetDepthStencil* getDepthStencilTarget() const { return rtDepthStencil_; }
        void attachRenderTarget(RenderTargetBase* rt);
        void attachDepthStencilTarget(RenderTargetDepthStencil* rt);

        bool isDefaultFBO() const { return isDefault_; }
        unsigned int getWidth() const { return width_; }
        unsigned int getHeight() const { return height_; }

    public:
        static FrameBuffer* getOrCreateDefaultFBO();
        static void applyDefaultFBO();
        static void clearAllFBOs();

    private:
        static FrameBuffer* _defaultFBO;
        static std::set<FrameBuffer*> _frameBuffers;

    public:
        FrameBuffer();
        virtual ~FrameBuffer();
        bool init();

    private:
        bool isDefault_;
        GLuint frameBufferObject_;
        bool fboBindingDirty_;
        uint8_t frameBufferId_;
        Color4F clearColor_;
        float   clearDepth_;
        int8_t  clearStencil_;
        int width_;
        int height_;
        RenderTargetBase* rtBase_;
        RenderTargetDepthStencil* rtDepthStencil_;
    };
}

#endif // FRAMEBUFFER_H
