#include "Thin3DGL.h"
#include "IMAGE/ZimLoad.h"
using MATH::Matrix4x4;
using IMAGE::ZIM_CLAMP;
#include "MATH/Utils.h"
using MATH::IsPowerOf2;
using IMAGE::ZIM_HAS_MIPS;
using IMAGE::ZIM_GEN_MIPS;

namespace THIN3D
{
    GLuint TypeToTarget(T3DTextureType type)
    {
        switch (type)
        {
        case LINEAR1D: return GL_TEXTURE_1D;
        case LINEAR2D: return GL_TEXTURE_2D;
        case LINEAR3D: return GL_TEXTURE_3D;
        case CUBE: return GL_TEXTURE_CUBE_MAP;
        case ARRAY1D: return GL_TEXTURE_1D_ARRAY;
        case ARRAY2D: return GL_TEXTURE_2D_ARRAY;
        default: return GL_NONE;
        }
    }

    void Thin3DGLBlendState::apply()
    {
        GFX::glstate.blend.set(enabled);
        GFX::glstate.blendEquationSeparate.set(eqCol, eqAlpha);
        GFX::glstate.blendFuncSeparate.set(srcCol, dstCol, srcAlpha, dstAlpha);
        GFX::glstate.colorMask.set(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        // glstate.blendColor.set(fixedColor);

        GFX::glstate.colorLogicOp.set(logicEnabled);
        if (logicEnabled)
        {
            GFX::glstate.logicOp.set(logicOp);
        }

        // glstate.colorMask.set(maskBits & 1, (maskBits >> 1) & 1, (maskBits >> 2) & 1, (maskBits >> 3) & 1);
    }

    void Thin3DGLDepthStencilState::apply()
    {
        GFX::glstate.depthTest.set(depthTestEnabled);
        GFX::glstate.depthFunc.set(depthComp);
        GFX::glstate.depthWrite.set(depthWriteEnabled);
        GFX::glstate.stencilTest.disable();
    }

    Thin3DGLBuffer::Thin3DGLBuffer(size_t size, uint32_t flags)
    {
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

    Thin3DGLBuffer::~Thin3DGLBuffer()
    {
        GFX::unregister_gl_resource_holder(this);
        glDeleteBuffers(1, &buffer_);
    }

    void Thin3DGLBuffer::setData(const uint8_t *data, size_t size)
    {
        bind();
        glBufferData(target_, size, data, usage_);
        knownSize_ = size;
    }

    void Thin3DGLBuffer::subData(const uint8_t *data, size_t offset, size_t size)
    {
        bind();
        if (size > knownSize_)
        {
            // Allocate the buffer.
            glBufferData(target_, size + offset, NULL, usage_);
            knownSize_ = size + offset;
        }
        glBufferSubData(target_, offset, size, data);
    }

    void Thin3DGLBuffer::bind()
    {
        if (target_ == GL_ARRAY_BUFFER)
        {
            GFX::glstate.arrayBuffer.bind(buffer_);
        }
        else
        {
            GFX::glstate.elementArrayBuffer.bind(buffer_);
        }
    }

    void Thin3DGLBuffer::glLost()
    {
        //ILOG("Recreating vertex buffer after glLost");
        knownSize_ = 0;  // Will cause a new glBufferData call. Should genBuffers again though?
        glGenBuffers(1, &buffer_);
    }

    Thin3DGLShader::Thin3DGLShader(bool isFragmentShader)
        : shader_(0)
        , type_(0)
    {
        type_ = isFragmentShader ? GL_FRAGMENT_SHADER : GL_VERTEX_SHADER;
    }

    Thin3DGLShader::~Thin3DGLShader()
    {
        glDeleteShader(shader_);
    }

    bool Thin3DGLShader::compile(const char *source)
    {
        source_ = source;
        shader_ = glCreateShader(type_);

        std::string temp;
        // Add the prelude on automatically for fragment shaders.
        if (type_ == GL_FRAGMENT_SHADER)
        {
            temp = std::string(glsl_fragment_prelude) + source;
            source = temp.c_str();
        }

        glShaderSource(shader_, 1, &source, 0);
        glCompileShader(shader_);
        GLint success = 0;
        glGetShaderiv(shader_, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            #define MAX_INFO_LOG_SIZE 2048
            GLchar infoLog[MAX_INFO_LOG_SIZE];
            GLsizei len = 0;
            glGetShaderInfoLog(shader_, MAX_INFO_LOG_SIZE, &len, infoLog);
            infoLog[len] = '\0';
            glDeleteShader(shader_);
            shader_ = 0;
            //ILOG("%s Shader compile error:\n%s", type_ == GL_FRAGMENT_SHADER ? "Fragment" : "Vertex", infoLog);
        }
        ok_ = success != 0;
        return ok_;
    }

    void Thin3DGLVertexFormat::apply(const void *base)
    {
        for (int i = 0; i < SEM_MAX; i++)
        {
            if (semanticsMask_ & (1 << i))
            {
                glEnableVertexAttribArray(i);
            }
        }
        intptr_t b = (intptr_t)base;
        for (size_t i = 0; i < components_.size(); i++)
        {
            switch (components_[i].type) {
            case FLOATx2:
                glVertexAttribPointer(components_[i].semantic, 2, GL_FLOAT, GL_FALSE, stride_, (void *)(b + (intptr_t)components_[i].offset));
                break;
            case FLOATx3:
                glVertexAttribPointer(components_[i].semantic, 3, GL_FLOAT, GL_FALSE, stride_, (void *)(b + (intptr_t)components_[i].offset));
                break;
            case FLOATx4:
                glVertexAttribPointer(components_[i].semantic, 4, GL_FLOAT, GL_FALSE, stride_, (void *)(b + (intptr_t)components_[i].offset));
                break;
            case UNORM8x4:
                glVertexAttribPointer(components_[i].semantic, 4, GL_UNSIGNED_BYTE, GL_TRUE, stride_, (void *)(b + (intptr_t)components_[i].offset));
                break;
            case INVALID:
                ;
                //ELOG("Thin3DGLVertexFormat: Invalid component type applied.");
            }
        }
    }

    void Thin3DGLVertexFormat::unApply()
    {
        for (int i = 0; i < SEM_MAX; i++)
        {
            if (semanticsMask_ & (1 << i))
            {
                glDisableVertexAttribArray(i);
            }
        }
    }

    void Thin3DGLVertexFormat::compile()
    {
        int sem = 0;
        for (int i = 0; i < (int)components_.size(); i++)
        {
            sem |= 1 << components_[i].semantic;
        }
        semanticsMask_ = sem;
        // TODO : Compute stride as well?
    }

    Thin3DGLShaderSet::Thin3DGLShaderSet()
    {
        program_ = 0;
        GFX::register_gl_resource_holder(this);
    }

    Thin3DGLShaderSet::~Thin3DGLShaderSet()
    {
        GFX::unregister_gl_resource_holder(this);
        vshader->release();
        fshader->release();
        glDeleteProgram(program_);
    }

    bool Thin3DGLShaderSet::link()
    {
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
        if (linkStatus != GL_TRUE)
        {
            GLint bufLength = 0;
            glGetProgramiv(program_, GL_INFO_LOG_LENGTH, &bufLength);
            if (bufLength)
            {
                char* buf = new char[bufLength];
                glGetProgramInfoLog(program_, bufLength, NULL, buf);
                //ELOG("Could not link program:\n %s", buf);
                // We've thrown out the source at this point. Might want to do something about that.
                delete[] buf;
            }
            return false;
        }

        // Auto-initialize samplers.
        glUseProgram(program_);
        for (int i = 0; i < 4; i++)
        {
            char temp[256];
            sprintf(temp, "Sampler%i", i);
            int samplerLoc = getUniformLoc(temp);
            if (samplerLoc != -1) {
                glUniform1i(samplerLoc, i);
            }
        }

        // Here we could (using glGetAttribLocation) save a bitmask about which pieces of vertex data are used in the shader
        // and then AND it with the vertex format bitmask later...
        return true;
    }

    void Thin3DGLShaderSet::apply()
    {
        glUseProgram(program_);
    }

    void Thin3DGLShaderSet::unApply()
    {
        glUseProgram(0);
    }

    int Thin3DGLShaderSet::getUniformLoc(const char *name)
    {
        auto iter = uniforms_.find(name);
        int loc = -1;
        if (iter != uniforms_.end())
        {
            loc = iter->second.loc_;
        }
        else
        {
            loc = glGetUniformLocation(program_, name);
            UniformInfo info;
            info.loc_ = loc;
            uniforms_[name] = info;
        }
        return loc;
    }

    void Thin3DGLShaderSet::setVector(const char *name, float *value, int n)
    {
        glUseProgram(program_);
        int loc = getUniformLoc(name);
        if (loc != -1)
        {
            switch (n)
            {
            case 1: glUniform1fv(loc, 1, value); break;
            case 2: glUniform1fv(loc, 2, value); break;
            case 3: glUniform1fv(loc, 3, value); break;
            case 4: glUniform1fv(loc, 4, value); break;
            }
        }
    }

    void Thin3DGLShaderSet::setMatrix4x4(const char *name, const Matrix4x4 &value)
    {
        glUseProgram(program_);
        int loc = getUniformLoc(name);
        if (loc != -1)
        {
            glUniformMatrix4fv(loc, 1, false, value.getReadPtr());
        }
    }

    void Thin3DGLShaderSet::glLost()
    {
        vshader->compile(vshader->getSource().c_str());
        fshader->compile(fshader->getSource().c_str());
        link();
    }

    Thin3DGLTexture::Thin3DGLTexture()
        : tex_(0)
        , target_(0)
    {
        width_ = 0;
        height_ = 0;
        depth_ = 0;
        format_ = IMG_UNKNOWN;
        glGenTextures(1, &tex_);
        GFX::register_gl_resource_holder(this);
    }

    Thin3DGLTexture::Thin3DGLTexture(T3DTextureType type, T3DImageFormat format, int width, int height, int depth, int mipLevels) : format_(format), tex_(0), target_(TypeToTarget(type)), mipLevels_(mipLevels)
    {
        width_ = width;
        height_ = height;
        depth_ = depth;
        glGenTextures(1, &tex_);
        GFX::register_gl_resource_holder(this);
    }

    Thin3DGLTexture::~Thin3DGLTexture()
    {
        GFX::unregister_gl_resource_holder(this);
        destroy();
    }

    bool Thin3DGLTexture::create(T3DTextureType type, T3DImageFormat format, int width, int height, int depth, int mipLevels)
    {
        format_ = format;
        target_ = TypeToTarget(type);
        mipLevels_ = mipLevels;
        width_ = width;
        height_ = height;
        depth_ = depth;
        return true;
    }

    void Thin3DGLTexture::destroy()
    {
        if (tex_)
        {
            glDeleteTextures(1, &tex_);
            tex_ = 0;
        }
    }
    void Thin3DGLTexture::setImageData(int x, int y, int z, int width, int height, int depth, int level, int stride, const uint8_t *data)
    {
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
        switch (format_)
        {
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
        switch (target_)
        {
        case GL_TEXTURE_2D:
            glTexImage2D(GL_TEXTURE_2D, level, internalFormat, width_, height_, 0, format, type, data);
            break;
        default:
            //ELOG("Thin3D GL: Targets other than GL_TEXTURE_2D not yet supported");
            break;
        }
    }

    void Thin3DGLTexture::autoGenMipmaps()
    {
        bind();
        glGenerateMipmap(target_);
    }

    void Thin3DGLTexture::bind()
    {
        glBindTexture(target_, tex_);
    }

    void Thin3DGLTexture::glLost()
    {
        if (!filename_.empty())
        {
            if (loadFromFile(filename_.c_str()))
            {
                //ILOG("Reloaded lost texture %s", filename_.c_str());
            }
            else
            {
                //ELOG("Failed to reload lost texture %s", filename_.c_str());
            }
        }
        else
        {
            //WLOG("Texture %p cannot be restored - has no filename", this);
            tex_ = 0;
        }
    }

    void Thin3DGLTexture::finalize(int zim_flags)
    {
        GLenum wrap = GL_REPEAT;
        if ((zim_flags & ZIM_CLAMP) || !IsPowerOf2(width_) || !IsPowerOf2(height_))
            wrap = GL_CLAMP_TO_EDGE;
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        if ((zim_flags & (ZIM_HAS_MIPS | ZIM_GEN_MIPS)))
        {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
        }
        else
        {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        }
    }

    Thin3DDepthStencilState *Thin3DGLContext::createDepthStencilState(bool depthTestEnabled, bool depthWriteEnabled, T3DComparison depthCompare)
    {
        Thin3DGLDepthStencilState *ds = new Thin3DGLDepthStencilState();
        ds->depthTestEnabled = depthTestEnabled;
        ds->depthWriteEnabled = depthWriteEnabled;
        ds->depthComp = compToGL[depthCompare];
        return ds;
    }

    Thin3DBlendState *Thin3DGLContext::createBlendState(const T3DBlendStateDesc &desc)
    {
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

    Thin3DBuffer *Thin3DGLContext::createBuffer(size_t size, uint32_t usageFlags)
    {
        return new Thin3DGLBuffer(size, usageFlags);
    }

    Thin3DShaderSet *Thin3DGLContext::createShaderSet(Thin3DShader *vshader, Thin3DShader *fshader)
    {
        if (!vshader || !fshader)
        {
            //ELOG("ShaderSet requires both a valid vertex and a fragment shader: %p %p", vshader, fshader);
            return NULL;
        }
        Thin3DGLShaderSet *shaderSet = new Thin3DGLShaderSet();
        vshader->addRef();
        fshader->addRef();
        shaderSet->vshader = static_cast<Thin3DGLShader *>(vshader);
        shaderSet->fshader = static_cast<Thin3DGLShader *>(fshader);
        if (shaderSet->link())
        {
            return shaderSet;
        }
        else
        {
            delete shaderSet;
            return NULL;
        }
    }

    Thin3DVertexFormat *Thin3DGLContext::createVertexFormat(const std::vector<Thin3DVertexComponent> &components, int stride, Thin3DShader *vshader)
    {
        UNUSED(vshader);
        Thin3DGLVertexFormat *fmt = new Thin3DGLVertexFormat();
        fmt->components_ = components;
        fmt->stride_ = stride;
        fmt->compile();
        return fmt;
    }

    Thin3DTexture *Thin3DGLContext::createTexture(T3DTextureType type, T3DImageFormat format, int width, int height, int depth, int mipLevels)
    {
        return new Thin3DGLTexture(type, format, width, height, depth, mipLevels);
    }

    Thin3DTexture *Thin3DGLContext::createTexture()
    {
        return new Thin3DGLTexture();
    }

    Thin3DShader *Thin3DGLContext::createVertexShader(const char *glsl_source, const char *hlsl_source)
    {
        UNUSED(hlsl_source);
        Thin3DGLShader *shader = new Thin3DGLShader(false);
        if (shader->compile(glsl_source))
        {
            return shader;
        }
        else
        {
            shader->release();
            return nullptr;
        }
    }

    Thin3DShader *Thin3DGLContext::createFragmentShader(const char *glsl_source, const char *hlsl_source)
    {
        UNUSED(hlsl_source);
        Thin3DGLShader *shader = new Thin3DGLShader(true);
        if (shader->compile(glsl_source))
        {
            return shader;
        }
        else
        {
            shader->release();
            return nullptr;
        }
    }

    void Thin3DGLContext::setTextures(int start, int count, Thin3DTexture **textures)
    {
        for (int i = start; i < start + count; i++)
        {
            Thin3DGLTexture *glTex = static_cast<Thin3DGLTexture *>(textures[i]);
            glActiveTexture(GL_TEXTURE0 + i);
            glTex->bind();
        }
        glActiveTexture(GL_TEXTURE0);
    }

    void Thin3DGLContext::setRenderState(T3DRenderState rs, uint32_t value)
    {
        switch (rs)
        {
        case T3DRenderState::CULL_MODE:
            switch (value)
            {
            case T3DCullMode::NO_CULL: GFX::glstate.cullFace.disable(); break;
            case T3DCullMode::CCW: GFX::glstate.cullFace.enable(); GFX::glstate.cullFaceMode.set(GL_CCW); break;
            case T3DCullMode::CW: GFX::glstate.cullFace.enable(); GFX::glstate.cullFaceMode.set(GL_CW); break;
            }
            break;
        }
    }

    void Thin3DGLContext::draw(T3DPrimitive prim, Thin3DShaderSet *shaderSet, Thin3DVertexFormat *format, Thin3DBuffer *vdata, int vertexCount, int offset)
    {
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

    void Thin3DGLContext::drawIndexed(T3DPrimitive prim, Thin3DShaderSet *shaderSet, Thin3DVertexFormat *format, Thin3DBuffer *vdata, Thin3DBuffer *idata, int vertexCount, int offset)
    {
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

    void Thin3DGLContext::drawUP(T3DPrimitive prim, Thin3DShaderSet *shaderSet, Thin3DVertexFormat *format, const void *vdata, int vertexCount)
    {
        Thin3DGLShaderSet *ss = static_cast<Thin3DGLShaderSet *>(shaderSet);
        Thin3DGLVertexFormat *fmt = static_cast<Thin3DGLVertexFormat *>(format);

        fmt->apply(vdata);
        ss->apply();

        glDrawArrays(primToGL[prim], 0, vertexCount);

        ss->unApply();
        fmt->unApply();
    }

    void Thin3DGLContext::clear(int mask, uint32_t colorval, float depthVal, int stencilVal)
    {
        float col[4];
        Uint32ToFloat4(colorval, col);
        GLuint glMask = 0;
        if (mask & T3DClear::COLOR)
        {
            glClearColor(col[0], col[1], col[2], col[3]);
            glMask |= GL_COLOR_BUFFER_BIT;
        }
        if (mask & T3DClear::DEPTH)
        {
            glClearDepth(depthVal);
            glMask |= GL_DEPTH_BUFFER_BIT;
        }
        if (mask & T3DClear::STENCIL)
        {
            glClearStencil(stencilVal);
            glMask |= GL_STENCIL_BUFFER_BIT;
        }
        glClear(glMask);
    }
}

