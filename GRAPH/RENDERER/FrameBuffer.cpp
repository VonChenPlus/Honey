#include "GRAPH/RENDERER/FrameBuffer.h"
#include "IMAGE/TinyImage.h"

namespace GRAPH
{
    FrameBuffer* FrameBuffer::_defaultFBO = nullptr;
    std::set<FrameBuffer*> FrameBuffer::_frameBuffers;

    RenderTargetBase::RenderTargetBase() {

    }

    RenderTargetBase::~RenderTargetBase() {

    }

    bool RenderTargetBase::init(unsigned int width, unsigned int height) {
        width_ = width;
        height_ = height;
        return true;
    }

    RenderTarget* RenderTarget::create(unsigned int width, unsigned int height, IMAGE::PixelFormat format) {
        auto result = new (std::nothrow) RenderTarget();
        if(result && result->init(width, height,format)) {
            result->autorelease();
            return result;
        }
        else {
            SAFE_DELETE(result);
            return nullptr;
        }
    }

    bool RenderTarget::init(unsigned int width, unsigned int height, IMAGE::PixelFormat format) {
        if(!RenderTargetBase::init(width, height)) {
            return false;
        }

        texture_ = new (std::nothrow) Texture2D();
        if(nullptr == texture_) return false;

        const IMAGE::PixelFormatInfoMap& pixelFormat = IMAGE::TinyImage::getPixelFormatInfoMap();;
        auto dataLen = width * height * pixelFormat.at(format).bpp << 3;
        auto data = new(std::nothrow) HBYTE[dataLen];
        if( nullptr == data) return false;

        memset(data, 0, dataLen);
        if(texture_->initWithData(static_cast<void *>(data), dataLen, format, width, height, MATH::Sizef(width, height))) {
            texture_->autorelease();
            SAFE_RETAIN(texture_);
            free(data);
        }
        else {
            SAFE_DELETE(texture_);
            free(data);
            return false;
        }

        return true;
    }

    RenderTarget::RenderTarget()
        : texture_(nullptr) {
        type_ = Type::Texture2D;
    }

    RenderTarget::~RenderTarget() {
        SAFE_RELEASE_NULL(texture_);
    }

    RenderTargetRenderBuffer::RenderTargetRenderBuffer()
        : colorBuffer_(0)
        , format_(GL_RGBA4) {
        type_ = Type::RenderBuffer;
    }

    RenderTargetRenderBuffer::~RenderTargetRenderBuffer() {
        if(glIsRenderbuffer(colorBuffer_)) {
            glDeleteRenderbuffers(1, &colorBuffer_);
            colorBuffer_ = 0;
        }
    }

    bool RenderTargetRenderBuffer::init(unsigned int width, unsigned int height) {
        if(!RenderTargetBase::init(width, height))
            return false;

        GLint oldRenderBuffer(0);
        glGetIntegerv(GL_RENDERBUFFER_BINDING, &oldRenderBuffer);

        //generate depthStencil
        glGenRenderbuffers(1, &colorBuffer_);
        glBindRenderbuffer(GL_RENDERBUFFER, colorBuffer_);
        //todo: this could have a param
        glRenderbufferStorage(GL_RENDERBUFFER, format_, width, height);
        glBindRenderbuffer(GL_RENDERBUFFER, oldRenderBuffer);

        return true;
    }

    RenderTargetRenderBuffer* RenderTargetRenderBuffer::create(unsigned int width, unsigned int height) {
        auto result = new (std::nothrow) RenderTargetRenderBuffer();

        if(result && result->init(width, height)) {
            result->autorelease();
            return result;
        }
        else {
            SAFE_DELETE(result);
            return nullptr;
        }
    }

    RenderTargetDepthStencil::RenderTargetDepthStencil()
        : depthStencilBuffer_(0) {
        type_ = Type::RenderBuffer;
    }

    RenderTargetDepthStencil::~RenderTargetDepthStencil() {
        if(glIsRenderbuffer(depthStencilBuffer_)) {
            glDeleteRenderbuffers(1, &depthStencilBuffer_);
            depthStencilBuffer_ = 0;
        }
    }

    bool RenderTargetDepthStencil::init(unsigned int width, unsigned int height) {
        if(!RenderTargetBase::init(width, height)) return false;
        GLint oldRenderBuffer(0);
        glGetIntegerv(GL_RENDERBUFFER_BINDING, &oldRenderBuffer);

        //generate depthStencil
        glGenRenderbuffers(1, &depthStencilBuffer_);
        glBindRenderbuffer(GL_RENDERBUFFER, depthStencilBuffer_);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
        glBindRenderbuffer(GL_RENDERBUFFER, oldRenderBuffer);

        return true;
    }

    RenderTargetDepthStencil* RenderTargetDepthStencil::create(unsigned int width, unsigned int height) {
        auto result = new (std::nothrow) RenderTargetDepthStencil();

        if(result && result->init(width, height)) {
            result->autorelease();
            return result;
        }
        else {
            SAFE_DELETE(result);
            return nullptr;
        }
    }

    bool FrameBuffer::init() {
        GLint fbo;
        glGetIntegerv(GL_FRAMEBUFFER_BINDING, &fbo);
        frameBufferObject_ = fbo;
        return true;
    }

    FrameBuffer* FrameBuffer::getOrCreateDefaultFBO() {
        if(nullptr == _defaultFBO) {
            auto result = new (std::nothrow) FrameBuffer();

            if(result && result->init()) {
                result->autorelease();
                result->isDefault_ = true;
            }
            else {
                SAFE_DELETE(result);
            }

            _defaultFBO = result;
        }

        return _defaultFBO;
    }

    void FrameBuffer::applyDefaultFBO() {
        if(_defaultFBO) {
            _defaultFBO->applyFBO();
        }
    }

    void FrameBuffer::clearAllFBOs() {
        for (auto fbo : _frameBuffers) {
            fbo->clearFBO();
        }
    }

    FrameBuffer* FrameBuffer::create(uint8_t fid, unsigned int width, unsigned int height) {
        auto result = new (std::nothrow) FrameBuffer();
        if(result && result->init(fid, width, height)) {
            result->autorelease();
            return result;
        }
        else {
            SAFE_DELETE(result);
            return nullptr;
        }
    }

    bool FrameBuffer::init(uint8_t fid, unsigned int width, unsigned int height) {
        frameBufferId_ = fid;
        width_ = width;
        height_ = height;

        GLint oldfbo;
        glGetIntegerv(GL_FRAMEBUFFER_BINDING, &oldfbo);

        glGenFramebuffers(1, &frameBufferObject_);
        glBindFramebuffer(GL_FRAMEBUFFER, frameBufferObject_);
        glBindFramebuffer(GL_FRAMEBUFFER, oldfbo);

        return true;
    }

    FrameBuffer::FrameBuffer()
        : clearColor_(Color4F(0, 0, 0, 1))
        , clearDepth_(1.0)
        , clearStencil_(0)
        , frameBufferObject_(0)
        , rtBase_(nullptr)
        , rtDepthStencil_(nullptr)
        , fboBindingDirty_(true)
        , isDefault_(false) {
        _frameBuffers.insert(this);
    }

    FrameBuffer::~FrameBuffer() {
        if(!isDefaultFBO()) {
            SAFE_RELEASE_NULL(rtBase_);
            SAFE_RELEASE_NULL(rtDepthStencil_);
            glDeleteFramebuffers(1, &frameBufferObject_);
            frameBufferObject_ = 0;
            _frameBuffers.erase(this);
        }
    }

    void FrameBuffer::clearFBO() {
        applyFBO();
        glClearColor(clearColor_.red, clearColor_.green, clearColor_.blue, clearColor_.alpha);
        glClearDepth(clearDepth_);
        glClearStencil(clearStencil_);
        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);
        applyDefaultFBO();
    }

    void FrameBuffer::attachRenderTarget(RenderTargetBase* rt) {
        if(isDefaultFBO()) {
            return;
        }
        if(rt->getWidth() != width_ || rt->getHeight() != height_) {
            return;
        }
        SAFE_RETAIN(rt);
        SAFE_RELEASE(rtBase_);
        rtBase_ = rt;
        fboBindingDirty_ = true;
    }

    void FrameBuffer::applyFBO() {
        glBindFramebuffer(GL_FRAMEBUFFER, frameBufferObject_);
        if(fboBindingDirty_ && !isDefaultFBO()) {
            if(RenderTargetBase::Type::Texture2D == rtBase_->getType())
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, rtBase_->getTexture()->getName(), 0);
            else
                glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rtBase_->getBuffer());
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, nullptr == rtDepthStencil_ ? 0 : rtDepthStencil_->getBuffer());
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, nullptr == rtDepthStencil_ ? 0 : rtDepthStencil_->getBuffer());
            fboBindingDirty_ = false;
        }
    }

    void FrameBuffer::attachDepthStencilTarget(RenderTargetDepthStencil* rt) {
        if(isDefaultFBO()) {
            return;
        }

        if(nullptr != rt && (rt->getWidth() != width_ || rt->getHeight() != height_)) {
            return;
        }
        SAFE_RETAIN(rt);
        SAFE_RELEASE(rtDepthStencil_);
        rtDepthStencil_ = rt;
        fboBindingDirty_ = true;
    }
}
