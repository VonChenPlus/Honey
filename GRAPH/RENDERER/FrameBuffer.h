#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include "BASE/HObject.h"
#include "GRAPH/RENDERER/GLCommon.h"
#include "GRAPH/BASE/Color.h"
#include "GRAPH/RENDERER/Texture2D.h"
#include <set>

namespace GRAPH
{
    class GLView;
    class EventListenerCustom;

    /**
     Viewport is a normalized to FrameBufferObject
     But for default FBO, the size is absolute.
     */
    struct Viewport
    {
        Viewport(float left, float bottom, float width, float height);
        Viewport();

        float _left;
        float _bottom;
        float _width;
        float _height;
    };

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

        unsigned int getWidth() const { return _width; }
        unsigned int getHeight() const { return _height; }
        Type getType() const { return _type; }
    protected:
        Type _type;
        unsigned int _width;
        unsigned int _height;

    };

    class RenderTarget : public RenderTargetBase
    {
    public:

        static RenderTarget* create(unsigned int width, unsigned int height, IMAGE::PixelFormat format = IMAGE::PixelFormat::RGBA8888);

        bool init(unsigned int width, unsigned int height, IMAGE::PixelFormat format);

        virtual Texture2D* getTexture() const { return _texture; }
    public:
        RenderTarget();
        virtual ~RenderTarget();

    protected:
        Texture2D* _texture;
    };

    class RenderTargetRenderBuffer : public RenderTargetBase
    {
    public:

        static RenderTargetRenderBuffer* create(unsigned int width, unsigned int height);

        bool init(unsigned int width, unsigned int height);

        virtual GLuint getBuffer() const { return _colorBuffer; }

    public:
        RenderTargetRenderBuffer();
        virtual ~RenderTargetRenderBuffer();

    protected:
        GLenum _format;
        GLuint _colorBuffer;
    };

    class RenderTargetDepthStencil : public RenderTargetBase
    {
    public:

        static RenderTargetDepthStencil* create(unsigned int width, unsigned int height);

        bool init(unsigned int width, unsigned int height);

        virtual GLuint getBuffer() const { return _depthStencilBuffer; }

    public:
        RenderTargetDepthStencil();
        virtual ~RenderTargetDepthStencil();

    protected:
        GLuint _depthStencilBuffer;
    };

    class FrameBuffer : public HObject
    {
    public:
        static FrameBuffer* create(uint8_t fid, unsigned int width, unsigned int height);

        bool init(uint8_t fid, unsigned int width, unsigned int height);
    public:
        GLuint getFBO() const { return _fbo; }
        GLuint getFID() const { return _fid; }
        //call glclear to clear frame buffer object
        void clearFBO();
        void applyFBO();
        void setClearColor(const Color4F& color) { _clearColor = color;}
        void setClearDepth(float depth) { _clearDepth = depth; }
        void setClearStencil(int8_t stencil) { _clearStencil = stencil; }
        const Color4F& getClearColor() const { return _clearColor; }
        float getClearDepth() const { return _clearDepth; }
        int8_t getClearStencil() const { return _clearStencil; }

        RenderTargetBase* getRenderTarget() const { return _rt; }
        RenderTargetDepthStencil* getDepthStencilTarget() const { return _rtDepthStencil; }
        void attachRenderTarget(RenderTargetBase* rt);
        void attachDepthStencilTarget(RenderTargetDepthStencil* rt);

        bool isDefaultFBO() const { return _isDefault; }
        unsigned int getWidth() const { return _width; }
        unsigned int getHeight() const { return _height; }

    public:
        FrameBuffer();
        virtual ~FrameBuffer();
        bool initWithGLView(GLView* view);
    private:
        //openGL content for FrameBuffer
        GLuint _fbo;
        //dirty flag for fbo binding
        bool _fboBindingDirty;
        //
        uint8_t _fid;
        //
        Color4F _clearColor;
        float   _clearDepth;
        int8_t  _clearStencil;
        int _width;
        int _height;
        RenderTargetBase* _rt;
        RenderTargetDepthStencil* _rtDepthStencil;
        bool _isDefault;
    public:
        static FrameBuffer* getOrCreateDefaultFBO(GLView* glView);
        static void applyDefaultFBO();
        static void clearAllFBOs();
    private:
        //static GLuint _defaultFBO;
        static FrameBuffer* _defaultFBO;
        static std::set<FrameBuffer*> _frameBuffers;
    };
}

#endif // FRAMEBUFFER_H
