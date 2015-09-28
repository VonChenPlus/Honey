#include "Thin3DGL.h"
#include "IMAGE/TinyZim.h"
using MATH::Matrix4;
using IMAGE::ZIM_CLAMP;
#include "MATH/MathDef.h"
using MATH::IsPowerOf2;
using IMAGE::ZIM_HAS_MIPS;
using IMAGE::ZIM_GEN_MIPS;
#include "UTILS/STRING/StringUtils.h"
using UTILS::STRING::StringFromFormat;

namespace GLOBAL
{
    extern GFX::OpenGLState &glState();
}

namespace THIN3D
{
    Thin3DContext *T3DCreateGLContext() {
        return new Thin3DGLContext();
    }

    GLuint TypeToTarget(T3DTextureType type) {
        switch (type) {
        case LINEAR1D: return GL_TEXTURE_1D;
        case LINEAR2D: return GL_TEXTURE_2D;
        case LINEAR3D: return GL_TEXTURE_3D;
        case CUBE: return GL_TEXTURE_CUBE_MAP;
        case ARRAY1D: return GL_TEXTURE_1D_ARRAY;
        case ARRAY2D: return GL_TEXTURE_2D_ARRAY;
        default: return GL_NONE;
        }
    }

    void Thin3DGLBlendState::apply() {
        GLOBAL::glState().blend.set(enabled);
        GLOBAL::glState().blendEquationSeparate.set(eqCol, eqAlpha);
        GLOBAL::glState().blendFuncSeparate.set(srcCol, dstCol, srcAlpha, dstAlpha);
        GLOBAL::glState().colorMask.set(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        // glstate.blendColor.set(fixedColor);

        GLOBAL::glState().colorLogicOp.set(logicEnabled);
        if (logicEnabled) {
            GLOBAL::glState().logicOp.set(logicOp);
        }

        // glstate.colorMask.set(maskBits & 1, (maskBits >> 1) & 1, (maskBits >> 2) & 1, (maskBits >> 3) & 1);
    }

    void Thin3DGLDepthStencilState::apply() {
        GLOBAL::glState().depthTest.set(depthTestEnabled);
        GLOBAL::glState().depthFunc.set(depthComp);
        GLOBAL::glState().depthWrite.set(depthWriteEnabled);
        GLOBAL::glState().stencilTest.disable();
    }

    Thin3DGLBuffer::Thin3DGLBuffer(size_t size, uint32 flags) {
        UNUSED(size);
        glGenBuffers(1, &buffer_);
        target_ = (flags & T3DBufferUsage::INDEXDATA) ? GL_ELEMENT_ARRAY_BUFFER : GL_ARRAY_BUFFER;
        usage_ = 0;
        if (flags & T3DBufferUsage::DYNAMIC)
            usage_ = GL_STREAM_DRAW;
        else
            usage_ = GL_STATIC_DRAW;
        knownSize_ = 0;
        GFX::register_gl_resource_holder(this);
    }

    Thin3DGLBuffer::~Thin3DGLBuffer() {
        GFX::unregister_gl_resource_holder(this);
        glDeleteBuffers(1, &buffer_);
    }

    void Thin3DGLBuffer::setData(const uint8 *data, size_t size) {
        bind();
        glBufferData(target_, size, data, usage_);
        knownSize_ = size;
    }

    void Thin3DGLBuffer::subData(const uint8 *data, size_t offset, size_t size) {
        bind();
        if (size > knownSize_) {
            // Allocate the buffer.
            glBufferData(target_, size + offset, NULLPTR, usage_);
            knownSize_ = size + offset;
        }
        glBufferSubData(target_, offset, size, data);
    }

    void Thin3DGLBuffer::bind() {
        if (target_ == GL_ARRAY_BUFFER) {
            GLOBAL::glState().arrayBuffer.bind(buffer_);
        }
        else {
            GLOBAL::glState().elementArrayBuffer.bind(buffer_);
        }
    }

    void Thin3DGLBuffer::glLost() {
        //ILOG("Recreating vertex buffer after glLost");
        knownSize_ = 0;  // Will cause a new glBufferData call. Should genBuffers again though?
        glGenBuffers(1, &buffer_);
    }

    Thin3DGLShader::Thin3DGLShader(bool isFragmentShader)
        : shader_(0)
        , type_(0) {
        type_ = isFragmentShader ? GL_FRAGMENT_SHADER : GL_VERTEX_SHADER;
    }

    Thin3DGLShader::~Thin3DGLShader() {
        glDeleteShader(shader_);
    }

    void Thin3DGLShader::compile(const char *source) {
        source_ = source;
        shader_ = glCreateShader(type_);

        std::string temp;
        // Add the prelude on automatically for fragment shaders.
        if (type_ == GL_FRAGMENT_SHADER) {
            temp = std::string(glsl_fragment_prelude) + source;
            source = temp.c_str();
        }

        glShaderSource(shader_, 1, &source, 0);
        glCompileShader(shader_);
        GLint success = 0;
        glGetShaderiv(shader_, GL_COMPILE_STATUS, &success);
        if (!success) {
            #define MAX_INFO_LOG_SIZE 2048
            GLchar infoLog[MAX_INFO_LOG_SIZE];
            GLsizei len = 0;
            glGetShaderInfoLog(shader_, MAX_INFO_LOG_SIZE, &len, infoLog);
            infoLog[len] = '\0';
            glDeleteShader(shader_);
            shader_ = 0;
            throw _HException_(StringFromFormat("%s Shader compile error:\n%s", type_ == GL_FRAGMENT_SHADER ? "Fragment" : "Vertex", infoLog), HException::GFX);
        }
    }

    void Thin3DGLVertexFormat::apply(const void *base) {
        for (int i = 0; i < SEM_MAX; i++) {
            if (semanticsMask_ & (1 << i)) {
                glEnableVertexAttribArray(i);
            }
        }
        intptr b = (intptr)base;
        for (size_t i = 0; i < components_.size(); i++) {
            switch (components_[i].type) {
            case FLOATx2:
                glVertexAttribPointer(components_[i].semantic, 2, GL_FLOAT, GL_FALSE, stride_, (void *)(b + (intptr)components_[i].offset));
                break;
            case FLOATx3:
                glVertexAttribPointer(components_[i].semantic, 3, GL_FLOAT, GL_FALSE, stride_, (void *)(b + (intptr)components_[i].offset));
                break;
            case FLOATx4:
                glVertexAttribPointer(components_[i].semantic, 4, GL_FLOAT, GL_FALSE, stride_, (void *)(b + (intptr)components_[i].offset));
                break;
            case UNORM8x4:
                glVertexAttribPointer(components_[i].semantic, 4, GL_UNSIGNED_BYTE, GL_TRUE, stride_, (void *)(b + (intptr)components_[i].offset));
                break;
            case INVALID:
                throw _HException_Normal("Thin3DGLVertexFormat: Invalid component type applied");
            }
        }
    }

    void Thin3DGLVertexFormat::unApply() {
        for (int i = 0; i < SEM_MAX; i++) {
            if (semanticsMask_ & (1 << i)) {
                glDisableVertexAttribArray(i);
            }
        }
    }

    void Thin3DGLVertexFormat::compile() {
        int sem = 0;
        for (int i = 0; i < (int)components_.size(); i++) {
            sem |= 1 << components_[i].semantic;
        }
        semanticsMask_ = sem;
        // TODO : Compute stride as well?
    }

    Thin3DGLShaderSet::Thin3DGLShaderSet() {
        program_ = 0;
        GFX::register_gl_resource_holder(this);
    }

    Thin3DGLShaderSet::~Thin3DGLShaderSet() {
        GFX::unregister_gl_resource_holder(this);
        vshader->release();
        fshader->release();
        glDeleteProgram(program_);
    }

    void Thin3DGLShaderSet::link() {
        program_ = glCreateProgram();
        glAttachShader(program_, vshader->getShader());
        glAttachShader(program_, fshader->getShader());

        // Bind all the common vertex data points. Mismatching ones will be ignored.
        glBindAttribLocation(program_, SEM_POSITION, "Position");
        glBindAttribLocation(program_, SEM_COLOR0, "Color0");
        glBindAttribLocation(program_, SEM_TEXCOORD0, "TexCoord0");
        glBindAttribLocation(program_, SEM_NORMAL, "Normal");
        glBindAttribLocation(program_, SEM_TANGENT, "Tangent");
        glBindAttribLocation(program_, SEM_BINORMAL, "Binormal");
        glLinkProgram(program_);

        GLint linkStatus = GL_FALSE;
        glGetProgramiv(program_, GL_LINK_STATUS, &linkStatus);
        if (linkStatus != GL_TRUE) {
            GLint bufLength = 0;
            glGetProgramiv(program_, GL_INFO_LOG_LENGTH, &bufLength);
            if (bufLength) {
                char* buf = new char[bufLength];
                glGetProgramInfoLog(program_, bufLength, NULLPTR, buf);
                std::string log = StringFromFormat("Could not link program:\n %s", buf);
                delete[] buf;
                // We've thrown out the source at this point. Might want to do something about that.
                throw _HException_(log, HException::GFX);
            }
        }

        // Auto-initialize samplers.
        glUseProgram(program_);
        for (int i = 0; i < 4; i++) {
            char temp[256];
            sprintf(temp, "Sampler%i", i);
            int samplerLoc = getUniformLoc(temp);
            if (samplerLoc != -1) {
                glUniform1i(samplerLoc, i);
            }
        }

        // Here we could (using glGetAttribLocation) save a bitmask about which pieces of vertex data are used in the shader
        // and then AND it with the vertex format bitmask later...
    }

    void Thin3DGLShaderSet::apply() {
        glUseProgram(program_);
    }

    void Thin3DGLShaderSet::unApply() {
        glUseProgram(0);
    }

    int Thin3DGLShaderSet::getUniformLoc(const char *name) {
        auto iter = uniforms_.find(name);
        int loc = -1;
        if (iter != uniforms_.end()) {
            loc = iter->second.loc_;
        }
        else {
            loc = glGetUniformLocation(program_, name);
            UniformInfo info;
            info.loc_ = loc;
            uniforms_[name] = info;
        }
        return loc;
    }

    void Thin3DGLShaderSet::setVector(const char *name, float *value, int n) {
        glUseProgram(program_);
        int loc = getUniformLoc(name);
        if (loc != -1) {
            switch (n) {
            case 1: glUniform1fv(loc, 1, value); break;
            case 2: glUniform1fv(loc, 2, value); break;
            case 3: glUniform1fv(loc, 3, value); break;
            case 4: glUniform1fv(loc, 4, value); break;
            }
        }
    }

    void Thin3DGLShaderSet::setMatrix4(const char *name, const Matrix4 &value) {
        glUseProgram(program_);
        int loc = getUniformLoc(name);
        if (loc != -1) {
            glUniformMatrix4fv(loc, 1, false, value);
        }
    }

    void Thin3DGLShaderSet::glLost() {
        vshader->compile(vshader->getSource().c_str());
        fshader->compile(fshader->getSource().c_str());
        link();
    }

    Thin3DGLTexture::Thin3DGLTexture()
        : tex_(0)
        , target_(0)
        , format_(IMG_UNKNOWN) {
        width_ = 0;
        height_ = 0;
        depth_ = 0;
        glGenTextures(1, &tex_);
        GFX::register_gl_resource_holder(this);
    }

    Thin3DGLTexture::Thin3DGLTexture(T3DTextureType type, T3DImageFormat format, int width, int height, int depth, int mipLevels)
        : tex_(0)
        , target_(TypeToTarget(type))
        , format_(format)
        , mipLevels_(mipLevels) {
        width_ = width;
        height_ = height;
        depth_ = depth;
        glGenTextures(1, &tex_);
        GFX::register_gl_resource_holder(this);
    }

    Thin3DGLTexture::~Thin3DGLTexture() {
        GFX::unregister_gl_resource_holder(this);
        destroy();
    }

    void Thin3DGLTexture::create(T3DTextureType type, T3DImageFormat format, int width, int height, int depth, int mipLevels) {
        format_ = format;
        target_ = TypeToTarget(type);
        mipLevels_ = mipLevels;
        width_ = width;
        height_ = height;
        depth_ = depth;
    }

    void Thin3DGLTexture::destroy() {
        if (tex_) {
            glDeleteTextures(1, &tex_);
            tex_ = 0;
        }
    }
    void Thin3DGLTexture::setImageData(int x, int y, int z, int width, int height, int depth, int level, int stride, const uint8 *data) {
        UNUSED(x);
        UNUSED(y);
        UNUSED(z);
        UNUSED(width);
        UNUSED(height);
        UNUSED(depth);
        UNUSED(stride);
        int internalFormat;
        int format;
        int type;
        switch (format_) {
        case RGBA8888:
            internalFormat = GL_RGBA;
            format = GL_RGBA;
            type = GL_UNSIGNED_BYTE;
            break;
        case RGBA4444:
            internalFormat = GL_RGBA;
            format = GL_RGBA;
            type = GL_UNSIGNED_SHORT_4_4_4_4;
            break;
        default:
            return;
        }

        bind();
        switch (target_) {
        case GL_TEXTURE_2D:
            glTexImage2D(GL_TEXTURE_2D, level, internalFormat, width_, height_, 0, format, type, data);
            break;
        default:
            throw _HException_Normal("Targets other than GL_TEXTURE_2D not yet supported");
        }
    }

    void Thin3DGLTexture::autoGenMipmaps() {
        bind();
        glGenerateMipmap(target_);
    }

    void Thin3DGLTexture::bind() {
        glBindTexture(target_, tex_);
    }

    void Thin3DGLTexture::glLost() {
        if (!filename_.empty()) {
            loadFromFile(filename_.c_str());
        }
        else {
            throw _HException_Normal(StringFromFormat("Texture %p cannot be restored - has no filename", this));
        }
    }

    void Thin3DGLTexture::finalize(int zim_flags) {
        GLenum wrap = GL_REPEAT;
        if ((zim_flags & ZIM_CLAMP) || !IsPowerOf2(width_) || !IsPowerOf2(height_))
            wrap = GL_CLAMP_TO_EDGE;
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        if ((zim_flags & (ZIM_HAS_MIPS | ZIM_GEN_MIPS))) {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
        }
        else {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        }
    }

    Thin3DGLContext::Thin3DGLContext() {
        createPresets();
    }

    Thin3DGLContext::~Thin3DGLContext() {
    }

    Thin3DDepthStencilState *Thin3DGLContext::createDepthStencilState(bool depthTestEnabled, bool depthWriteEnabled, T3DComparison depthCompare) {
        Thin3DGLDepthStencilState *ds = new Thin3DGLDepthStencilState();
        ds->depthTestEnabled = depthTestEnabled;
        ds->depthWriteEnabled = depthWriteEnabled;
        ds->depthComp = compToGL[depthCompare];
        return ds;
    }

    Thin3DBlendState *Thin3DGLContext::createBlendState(const T3DBlendStateDesc &desc) {
        Thin3DGLBlendState *bs = new Thin3DGLBlendState();
        bs->enabled = desc.enabled;
        bs->eqCol = blendEqToGL[desc.eqCol];
        bs->srcCol = blendFactorToGL[desc.srcCol];
        bs->dstCol = blendFactorToGL[desc.dstCol];
        bs->eqAlpha = blendEqToGL[desc.eqAlpha];
        bs->srcAlpha = blendFactorToGL[desc.srcAlpha];
        bs->dstAlpha = blendFactorToGL[desc.dstAlpha];
        bs->logicEnabled = desc.logicEnabled;
        bs->logicOp = logicOpToGL[desc.logicOp];
        return bs;
    }

    Thin3DBuffer *Thin3DGLContext::createBuffer(size_t size, uint32 usageFlags) {
        return new Thin3DGLBuffer(size, usageFlags);
    }

    Thin3DShaderSet *Thin3DGLContext::createShaderSet(Thin3DShader *vshader, Thin3DShader *fshader) {
        if (!vshader || !fshader) {
            throw _HException_Normal(StringFromFormat("ShaderSet requires both a valid vertex and a fragment shader: %p %p", vshader, fshader));
        }
        Thin3DGLShaderSet *shaderSet = new Thin3DGLShaderSet();
        vshader->addRef();
        fshader->addRef();
        shaderSet->vshader = static_cast<Thin3DGLShader *>(vshader);
        shaderSet->fshader = static_cast<Thin3DGLShader *>(fshader);
        shaderSet->link();
        return shaderSet;
    }

    Thin3DVertexFormat *Thin3DGLContext::createVertexFormat(const std::vector<Thin3DVertexComponent> &components, int stride, Thin3DShader *vshader) {
        UNUSED(vshader);
        Thin3DGLVertexFormat *fmt = new Thin3DGLVertexFormat();
        fmt->components_ = components;
        fmt->stride_ = stride;
        fmt->compile();
        return fmt;
    }

    Thin3DTexture *Thin3DGLContext::createTexture(T3DTextureType type, T3DImageFormat format, int width, int height, int depth, int mipLevels) {
        return new Thin3DGLTexture(type, format, width, height, depth, mipLevels);
    }

    Thin3DTexture *Thin3DGLContext::createTexture() {
        return new Thin3DGLTexture();
    }

    void Thin3DGLContext::setBlendState(Thin3DBlendState *state) {
        Thin3DGLBlendState *s = static_cast<Thin3DGLBlendState *>(state);
        s->apply();
    }

    void Thin3DGLContext::setDepthStencilState(Thin3DDepthStencilState *state) {
        Thin3DGLDepthStencilState *s = static_cast<Thin3DGLDepthStencilState *>(state);
        s->apply();
    }

    Thin3DShader *Thin3DGLContext::createVertexShader(const char *glsl_source, const char *hlsl_source) {
        UNUSED(hlsl_source);
        Thin3DGLShader *shader = new Thin3DGLShader(false);
        shader->compile(glsl_source);
        return shader;
    }

    Thin3DShader *Thin3DGLContext::createFragmentShader(const char *glsl_source, const char *hlsl_source) {
        UNUSED(hlsl_source);
        Thin3DGLShader *shader = new Thin3DGLShader(true);
        shader->compile(glsl_source);
        return shader;
    }

    void Thin3DGLContext::setScissorEnabled(bool enable) {
        GLOBAL::glState().scissorTest.set(enable);
    }

    void Thin3DGLContext::setScissorRect(int left, int top, int width, int height) {
        GLOBAL::glState().scissorRect.set(left, targetHeight_ - (top + height), width, height);
    }

    void Thin3DGLContext::setViewports(int count, T3DViewport *viewports) {
        UNUSED(count);
        // TODO: Add support for multiple viewports.
        GLOBAL::glState().viewport.set(viewports[0].TopLeftX, viewports[0].TopLeftY, viewports[0].Width, viewports[0].Height);
        GLOBAL::glState().depthRange.set(viewports[0].MinDepth, viewports[0].MaxDepth);
    }

    void Thin3DGLContext::setTextures(int start, int count, Thin3DTexture **textures) {
        for (int i = start; i < start + count; i++) {
            Thin3DGLTexture *glTex = static_cast<Thin3DGLTexture *>(textures[i]);
            glActiveTexture(GL_TEXTURE0 + i);
            glTex->bind();
        }
        glActiveTexture(GL_TEXTURE0);
    }

    void Thin3DGLContext::setRenderState(T3DRenderState rs, uint32 value) {
        switch (rs) {
        case T3DRenderState::CULL_MODE:
            switch (value) {
            case T3DCullMode::NO_CULL: GLOBAL::glState().cullFace.disable(); break;
            case T3DCullMode::CCW: GLOBAL::glState().cullFace.enable(); GLOBAL::glState().cullFaceMode.set(GL_CCW); break;
            case T3DCullMode::CW: GLOBAL::glState().cullFace.enable(); GLOBAL::glState().cullFaceMode.set(GL_CW); break;
            }
            break;
        }
    }

    void Thin3DGLContext::draw(T3DPrimitive prim, Thin3DShaderSet *shaderSet, Thin3DVertexFormat *format, Thin3DBuffer *vdata, int vertexCount, int offset) {
        Thin3DGLShaderSet *ss = static_cast<Thin3DGLShaderSet *>(shaderSet);
        Thin3DGLBuffer *vbuf = static_cast<Thin3DGLBuffer *>(vdata);
        Thin3DGLVertexFormat *fmt = static_cast<Thin3DGLVertexFormat *>(format);

        vbuf->bind();
        fmt->apply();
        ss->apply();

        glDrawArrays(primToGL[prim], offset, vertexCount);

        ss->unApply();
        fmt->unApply();
    }

    void Thin3DGLContext::drawIndexed(T3DPrimitive prim, Thin3DShaderSet *shaderSet, Thin3DVertexFormat *format, Thin3DBuffer *vdata, Thin3DBuffer *idata, int vertexCount, int offset) {
        UNUSED(vertexCount);
        Thin3DGLShaderSet *ss = static_cast<Thin3DGLShaderSet *>(shaderSet);
        Thin3DGLBuffer *vbuf = static_cast<Thin3DGLBuffer *>(vdata);
        Thin3DGLBuffer *ibuf = static_cast<Thin3DGLBuffer *>(idata);
        Thin3DGLVertexFormat *fmt = static_cast<Thin3DGLVertexFormat *>(format);

        vbuf->bind();
        ibuf->bind();
        fmt->apply();
        ss->apply();

        glDrawElements(primToGL[prim], offset, GL_INT, 0);

        ss->unApply();
        fmt->unApply();
    }

    void Thin3DGLContext::drawUP(T3DPrimitive prim, Thin3DShaderSet *shaderSet, Thin3DVertexFormat *format, const void *vdata, int vertexCount) {
        Thin3DGLShaderSet *ss = static_cast<Thin3DGLShaderSet *>(shaderSet);
        Thin3DGLVertexFormat *fmt = static_cast<Thin3DGLVertexFormat *>(format);

        fmt->apply(vdata);
        ss->apply();

        glDrawArrays(primToGL[prim], 0, vertexCount);

        ss->unApply();
        fmt->unApply();
    }

    void Thin3DGLContext::clear(int mask, uint32 colorval, float depthVal, int stencilVal) {
        float col[4];
        Uint32ToFloat4(colorval, col);
        GLuint glMask = 0;
        if (mask & T3DClear::COLOR) {
            glClearColor(col[0], col[1], col[2], col[3]);
            glMask |= GL_COLOR_BUFFER_BIT;
        }
        if (mask & T3DClear::DEPTH) {
            glClearDepth(depthVal);
            glMask |= GL_DEPTH_BUFFER_BIT;
        }
        if (mask & T3DClear::STENCIL) {
            glClearStencil(stencilVal);
            glMask |= GL_STENCIL_BUFFER_BIT;
        }
        glClear(glMask);
    }
}

