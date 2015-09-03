#include "GRAPH/RENDERER/FrameBuffer.h"

namespace GRAPH
{
    FrameBuffer* FrameBuffer::_defaultFBO = nullptr;
    std::set<FrameBuffer*> FrameBuffer::_frameBuffers;

    Viewport::Viewport(float left, float bottom, float width, float height)
    : _left(left)
    , _bottom(bottom)
    , _width(width)
    , _height(height)
    {
    }

    Viewport::Viewport()
    {
        _left = _bottom = 0.f;
        _width = _height = 1.0f;
    }

    RenderTargetBase::RenderTargetBase()
    {

    }

    RenderTargetBase::~RenderTargetBase()
    {

    }

    bool RenderTargetBase::init(unsigned int width, unsigned int height)
    {
        _width = width;
        _height = height;
        return true;
    }

    RenderTarget* RenderTarget::create(unsigned int width, unsigned int height, IMAGE::PixelFormat format/* = IMAGE::PixelFormat::RGBA8888*/)
    {
        auto result = new (std::nothrow) RenderTarget();
        if(result && result->init(width, height,format))
        {
            result->autorelease();
            return result;
        }
        else
        {
            SAFE_DELETE(result);
            return nullptr;
        }
    }

    bool RenderTarget::init(unsigned int width, unsigned int height, IMAGE::PixelFormat format)
    {
        if(!RenderTargetBase::init(width, height))
        {
            return false;
        }

        _texture = new (std::nothrow) Texture2D();
        if(nullptr == _texture) return false;
        //TODO: FIX me, get the correct bit depth for pixelformat
        auto dataLen = width * height * 4;
        auto data = malloc(dataLen);
        if( nullptr == data) return false;

        memset(data, 0, dataLen);
        if(_texture->initWithData(data, dataLen, format, width, height, MATH::Sizef(width, height)))
        {
            _texture->autorelease();
            SAFE_RETAIN(_texture);
            free(data);
        }
        else
        {
            SAFE_DELETE(_texture);
            free(data);
            return false;
        }

        return true;
    }

    RenderTarget::RenderTarget()
    : _texture(nullptr)
    {
        _type = Type::Texture2D;
    }

    RenderTarget::~RenderTarget()
    {
        SAFE_RELEASE_NULL(_texture);
    }

    RenderTargetRenderBuffer::RenderTargetRenderBuffer()
    : _colorBuffer(0)
    , _format(GL_RGBA4)
    {
        _type = Type::RenderBuffer;
    }

    RenderTargetRenderBuffer::~RenderTargetRenderBuffer()
    {
        if(glIsRenderbuffer(_colorBuffer))
        {
            glDeleteRenderbuffers(1, &_colorBuffer);
            _colorBuffer = 0;
        }
    }

    bool RenderTargetRenderBuffer::init(unsigned int width, unsigned int height)
    {
        if(!RenderTargetBase::init(width, height)) return false;
        GLint oldRenderBuffer(0);
        glGetIntegerv(GL_RENDERBUFFER_BINDING, &oldRenderBuffer);

        //generate depthStencil
        glGenRenderbuffers(1, &_colorBuffer);
        glBindRenderbuffer(GL_RENDERBUFFER, _colorBuffer);
        //todo: this could have a param
        glRenderbufferStorage(GL_RENDERBUFFER, _format, width, height);
        glBindRenderbuffer(GL_RENDERBUFFER, oldRenderBuffer);

        return true;
    }


    RenderTargetRenderBuffer* RenderTargetRenderBuffer::create(unsigned int width, unsigned int height)
    {
        auto result = new (std::nothrow) RenderTargetRenderBuffer();

        if(result && result->init(width, height))
        {
            result->autorelease();
            return result;
        }
        else
        {
            SAFE_DELETE(result);
            return nullptr;
        }
    }

    RenderTargetDepthStencil::RenderTargetDepthStencil()
    : _depthStencilBuffer(0)
    {
        _type = Type::RenderBuffer;
    }

    RenderTargetDepthStencil::~RenderTargetDepthStencil()
    {
        if(glIsRenderbuffer(_depthStencilBuffer))
        {
            glDeleteRenderbuffers(1, &_depthStencilBuffer);
            _depthStencilBuffer = 0;
        }
    }

    bool RenderTargetDepthStencil::init(unsigned int width, unsigned int height)
    {
        if(!RenderTargetBase::init(width, height)) return false;
        GLint oldRenderBuffer(0);
        glGetIntegerv(GL_RENDERBUFFER_BINDING, &oldRenderBuffer);

        //generate depthStencil
        glGenRenderbuffers(1, &_depthStencilBuffer);
        glBindRenderbuffer(GL_RENDERBUFFER, _depthStencilBuffer);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
        glBindRenderbuffer(GL_RENDERBUFFER, oldRenderBuffer);

        return true;
    }


    RenderTargetDepthStencil* RenderTargetDepthStencil::create(unsigned int width, unsigned int height)
    {
        auto result = new (std::nothrow) RenderTargetDepthStencil();

        if(result && result->init(width, height))
        {
            result->autorelease();
            return result;
        }
        else
        {
            SAFE_DELETE(result);
            return nullptr;
        }
    }

    bool FrameBuffer::initWithGLView(GLView* view)
    {
        if(view == nullptr)
        {
            return false;
        }
        GLint fbo;
        glGetIntegerv(GL_FRAMEBUFFER_BINDING, &fbo);
        _fbo = fbo;
        return true;
    }

    FrameBuffer* FrameBuffer::getOrCreateDefaultFBO(GLView* view)
    {
        if(nullptr == _defaultFBO)
        {
            auto result = new (std::nothrow) FrameBuffer();

            if(result && result->initWithGLView(view))
            {
                result->autorelease();
                result->_isDefault = true;
            }
            else
            {
                SAFE_DELETE(result);
            }

            _defaultFBO = result;
        }

        return _defaultFBO;
    }

    void FrameBuffer::applyDefaultFBO()
    {
        if(_defaultFBO)
        {
            _defaultFBO->applyFBO();
        }
    }

    void FrameBuffer::clearAllFBOs()
    {
        for (auto fbo : _frameBuffers)
        {
            fbo->clearFBO();
        }
    }

    FrameBuffer* FrameBuffer::create(uint8_t fid, unsigned int width, unsigned int height)
    {
        auto result = new (std::nothrow) FrameBuffer();
        if(result && result->init(fid, width, height))
        {
            result->autorelease();
            return result;
        }
        else
        {
            SAFE_DELETE(result);
            return nullptr;
        }
    }

    bool FrameBuffer::init(uint8_t fid, unsigned int width, unsigned int height)
    {
        _fid = fid;
        _width = width;
        _height = height;

        GLint oldfbo;
        glGetIntegerv(GL_FRAMEBUFFER_BINDING, &oldfbo);

        glGenFramebuffers(1, &_fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, _fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, oldfbo);

        return true;
    }

    FrameBuffer::FrameBuffer()
    : _clearColor(Color4F(0, 0, 0, 1))
    , _clearDepth(1.0)
    , _clearStencil(0)
    , _fbo(0)
    , _rt(nullptr)
    , _rtDepthStencil(nullptr)
    , _fboBindingDirty(true)
    , _isDefault(false)
    {
        _frameBuffers.insert(this);
    }

    FrameBuffer::~FrameBuffer()
    {
        if(!isDefaultFBO())
        {
            SAFE_RELEASE_NULL(_rt);
            SAFE_RELEASE_NULL(_rtDepthStencil);
            glDeleteFramebuffers(1, &_fbo);
            _fbo = 0;
            _frameBuffers.erase(this);
        }
    }

    void FrameBuffer::clearFBO()
    {
        applyFBO();
        glClearColor(_clearColor.red, _clearColor.green, _clearColor.blue, _clearColor.alpha);
        glClearDepth(_clearDepth);
        glClearStencil(_clearStencil);
        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);
        applyDefaultFBO();
    }

    void FrameBuffer::attachRenderTarget(RenderTargetBase* rt)
    {
        if(isDefaultFBO())
        {
            return;
        }
        if(rt->getWidth() != _width || rt->getHeight() != _height)
        {
            return;
        }
        SAFE_RETAIN(rt);
        SAFE_RELEASE(_rt);
        _rt = rt;
        _fboBindingDirty = true;
    }

    void FrameBuffer::applyFBO()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, _fbo);
        if(_fboBindingDirty && !isDefaultFBO())
        {
            if(RenderTargetBase::Type::Texture2D == _rt->getType())
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _rt->getTexture()->getName(), 0);
            else
                glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, _rt->getBuffer());
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, nullptr == _rtDepthStencil ? 0 : _rtDepthStencil->getBuffer());
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, nullptr == _rtDepthStencil ? 0 : _rtDepthStencil->getBuffer());
            _fboBindingDirty = false;
        }
    }

    void FrameBuffer::attachDepthStencilTarget(RenderTargetDepthStencil* rt)
    {
        if(isDefaultFBO())
        {
            return;
        }

        if(nullptr != rt && (rt->getWidth() != _width || rt->getHeight() != _height))
        {
            return;
        }
        SAFE_RETAIN(rt);
        SAFE_RELEASE(_rtDepthStencil);
        _rtDepthStencil = rt;
        _fboBindingDirty = true;
    }
}
