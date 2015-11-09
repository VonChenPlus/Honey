#include "GRAPH/UNITY3D/Unity3DGL.h"
#include "GRAPH/UNITY3D/Unity3DGLShader.h"
#include "GRAPH/UNITY3D/TextureCache.h"
#include "UTILS/STRING/StringUtils.h"

namespace GRAPH
{
    void Unity3DGLBlendState::apply() {
        Unity3DGLState::OpenGLState().blend.set(enabled);
        Unity3DGLState::OpenGLState().blendEquationSeparate.set(eqCol, eqAlpha);
        Unity3DGLState::OpenGLState().blendFuncSeparate.set(srcCol, dstCol, srcAlpha, dstAlpha);
        Unity3DGLState::OpenGLState().colorMask.set(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

        Unity3DGLState::OpenGLState().colorLogicOp.set(logicEnabled);
        if (logicEnabled) {
            Unity3DGLState::OpenGLState().logicOp.set(logicOp);
        }
    }

    Unity3DGLDepthState::Unity3DGLDepthState() {
        loadDefault();
    }

    void Unity3DGLDepthState::loadDefault() {
        depthTest_ = Unity3DGLState::OpenGLState().depthTest.get();
        depthWrite_ = Unity3DGLState::OpenGLState().depthWrite.get() == GL_TRUE;
        depthComp_ = Unity3DGLState::OpenGLState().depthFunc.get();
    }

    void Unity3DGLDepthState::setDepthTest(bool depthTest) {
        depthTest_ = depthTest;
    }

    void Unity3DGLDepthState::setDepthWrite(bool depthWrite) {
        depthWrite_ = depthWrite;
    }

    void Unity3DGLDepthState::setDepthComp(uint32 depthComp) {
        depthComp_ = depthComp;
    }

    void Unity3DGLDepthState::apply() {
        Unity3DGLState::OpenGLState().depthTest.set(depthTest_);
        Unity3DGLState::OpenGLState().depthWrite.set(depthWrite_);
        Unity3DGLState::OpenGLState().depthFunc.set(depthComp_);
    }

    static inline void Uint32ToFloat4(uint32 u, float f[4]) {
        f[0] = ((u >> 0) & 0xFF) * (1.0f / 255.0f);
        f[1] = ((u >> 8) & 0xFF) * (1.0f / 255.0f);
        f[2] = ((u >> 16) & 0xFF) * (1.0f / 255.0f);
        f[3] = ((u >> 24) & 0xFF) * (1.0f / 255.0f);
    }

    Unity3DGLBuffer::Unity3DGLBuffer(uint32 flags) {
        glGenBuffers(1, &buffer_);
        target_ = (flags & U3DBufferUsage::INDEXDATA) ? GL_ELEMENT_ARRAY_BUFFER : GL_ARRAY_BUFFER;
        usage_ = 0;
        if (flags & U3DBufferUsage::DYNAMIC)
            usage_ = GL_DYNAMIC_DRAW;
        else if (flags & U3DBufferUsage::STREAM)
            usage_ = GL_STREAM_DRAW;
        else
            usage_ = GL_STATIC_DRAW;
        knownSize_ = 0;
    }

    Unity3DGLBuffer::~Unity3DGLBuffer() {
        glDeleteBuffers(1, &buffer_);
    }

    void Unity3DGLBuffer::setData(const uint8 *data, uint64 size) {
        bind();
        glBufferData(target_, size, data, usage_);
        knownSize_ = size;
    }

    void Unity3DGLBuffer::subData(const uint8 *data, uint64 offset, uint64 size) {
        bind();
        if (size > knownSize_) {
            // Allocate the buffer.
            glBufferData(target_, size + offset, nullptr, usage_);
            knownSize_ = size + offset;
        }
        glBufferSubData(target_, offset, size, data);
    }

    void Unity3DGLBuffer::bind() {
        if (target_ == GL_ARRAY_BUFFER) {
            Unity3DGLState::OpenGLState().arrayBuffer.bind(buffer_);
        }
        else {
            Unity3DGLState::OpenGLState().elementArrayBuffer.bind(buffer_);
        }
    }

    Unity3DGLUniformFormat::Unity3DGLUniformFormat(Unity3DShaderSet *u3dShader, const U3DuniformComponent &component) {
        u3dShader_ = u3dShader;
        component_ = component;
    }

    void Unity3DGLUniformFormat::applyArray() {
        switch (component_.type) {
        case GL_FLOAT:
            u3dShader_->setUniformLocationWith1fv(component_.location, component_.value.floatv.pointer, component_.value.floatv.size);
            break;

        case GL_FLOAT_VEC2:
            u3dShader_->setUniformLocationWith2fv(component_.location, component_.value.v2f.pointer, component_.value.v2f.size);
            break;

        case GL_FLOAT_VEC3:
            u3dShader_->setUniformLocationWith3fv(component_.location, component_.value.v3f.pointer, component_.value.v3f.size);
            break;

        case GL_FLOAT_VEC4:
            u3dShader_->setUniformLocationWith4fv(component_.location, component_.value.v4f.pointer, component_.value.v4f.size);
            break;

        default:
            break;
        }
    }

    void Unity3DGLUniformFormat::applyValue() {
        switch (component_.type) {
        case GL_SAMPLER_2D:
            u3dShader_->setUniformLocationWith1i(component_.location, component_.value.tex.textureUnit);
            Unity3DGLState::BindTexture2DN(component_.value.tex.textureUnit, component_.value.tex.textureId);
            break;

        case GL_SAMPLER_CUBE:
            u3dShader_->setUniformLocationWith1i(component_.location, component_.value.tex.textureUnit);
            Unity3DGLState::BindTextureN(component_.value.tex.textureUnit, component_.value.tex.textureId, GL_TEXTURE_CUBE_MAP);
            break;

        case GL_INT:
            u3dShader_->setUniformLocationWith1i(component_.location, component_.value.intValue);
            break;

        case GL_FLOAT:
            u3dShader_->setUniformLocationWith1f(component_.location, component_.value.floatValue);
            break;

        case GL_FLOAT_VEC2:
            u3dShader_->setUniformLocationWith2f(component_.location, component_.value.v2Value[0], component_.value.v2Value[1]);
            break;

        case GL_FLOAT_VEC3:
            u3dShader_->setUniformLocationWith3f(component_.location, component_.value.v3Value[0], component_.value.v3Value[1], component_.value.v3Value[2]);
            break;

        case GL_FLOAT_VEC4:
            u3dShader_->setUniformLocationWith4f(component_.location, component_.value.v4Value[0], component_.value.v4Value[1], component_.value.v4Value[2], component_.value.v4Value[3]);
            break;

        case GL_FLOAT_MAT4:
            u3dShader_->setUniformLocationWithMatrix4fv(component_.location, (GLfloat*) &component_.value.matrixValue, 1);
            break;

        default:
            break;
        }
    }

    Unity3DGLVertexFormat::Unity3DGLVertexFormat(const U3DVertexComponent &component) {
        components_.push_back(component);
    }

    Unity3DGLVertexFormat::Unity3DGLVertexFormat(const std::vector<U3DVertexComponent> &components) {
        components_ = components;
    }

    void Unity3DGLVertexFormat::apply(const void *base) {
        for (int i = 0; i < SEM_MAX; i++) {
            if (semanticsMask_ & (1 << i)) {
                glEnableVertexAttribArray(i);
            }
        }

        intptr b = (intptr) base;
        for (uint64 i = 0; i < components_.size(); i++) {
            switch (components_[i].type) {
            case FLOATx2:
                glVertexAttribPointer(components_[i].semantic, 2, GL_FLOAT, GL_FALSE, components_[i].stride, (void *) (b + (intptr) components_[i].offset));
                break;
            case FLOATx3:
                glVertexAttribPointer(components_[i].semantic, 3, GL_FLOAT, GL_FALSE, components_[i].stride, (void *) (b + (intptr) components_[i].offset));
                break;
            case FLOATx4:
                glVertexAttribPointer(components_[i].semantic, 4, GL_FLOAT, GL_FALSE, components_[i].stride, (void *) (b + (intptr) components_[i].offset));
                break;
            case UNORM8x4:
                glVertexAttribPointer(components_[i].semantic, 4, GL_UNSIGNED_BYTE, GL_TRUE, components_[i].stride, (void *) (b + (intptr) components_[i].offset));
                break;
            case INVALID:
                throw _HException_Normal("Unity3DGLVertexFormat: Invalid component type applied");
            default:
                glVertexAttribPointer(components_[i].semantic, components_[i].size, components_[i].type, components_[i].normalized, components_[i].stride, (void *) (b + (intptr) components_[i].offset));
            }
        }
    }

    void Unity3DGLVertexFormat::unApply() {
        for (int i = 0; i < SEM_MAX; i++) {
            if (semanticsMask_ & (1 << i)) {
                glDisableVertexAttribArray(i);
            }
        }
    }

    void Unity3DGLVertexFormat::compile() {
        int sem = 0;
        for (int i = 0; i < (int) components_.size(); i++) {
            sem |= 1 << components_[i].semantic;
        }
        semanticsMask_ = sem;
    }

    void Unity3DGLContext::draw(U3DPrimitive prim, Unity3DVertexFormat *format, Unity3DBuffer *vdata, int vertexCount, int offset) {
        Unity3DGLBuffer *vbuf = static_cast<Unity3DGLBuffer *>(vdata);
        Unity3DGLVertexFormat *fmt = static_cast<Unity3DGLVertexFormat *>(format);

        vbuf->bind();
        fmt->apply();

        glDrawArrays(primToGL[prim], offset, vertexCount);

        fmt->unApply();
    }

    void Unity3DGLContext::drawIndexed(U3DPrimitive prim, Unity3DVertexFormat *format, Unity3DBuffer *vdata, Unity3DBuffer *idata, void *indices, int offset) {
        Unity3DGLBuffer *vbuf = static_cast<Unity3DGLBuffer *>(vdata);
        Unity3DGLBuffer *ibuf = static_cast<Unity3DGLBuffer *>(idata);
        Unity3DGLVertexFormat *fmt = static_cast<Unity3DGLVertexFormat *>(format);

        vbuf->bind();
        ibuf->bind();
        fmt->apply();

        glDrawElements(primToGL[prim], offset, GL_UNSIGNED_SHORT, indices);

        fmt->unApply();
    }

    void Unity3DGLContext::drawUp(U3DPrimitive prim, Unity3DVertexFormat *format, const void *vdata, int vertexCount) {
        Unity3DGLVertexFormat *fmt = static_cast<Unity3DGLVertexFormat *>(format);

        fmt->apply(vdata);

        glDrawArrays(primToGL[prim], 0, vertexCount);

        fmt->unApply();
    }

    void Unity3DGLContext::clear(int mask, uint32 colorval, float depthVal, int stencilVal) {
        float col[4];
        Uint32ToFloat4(colorval, col);
        GLuint glMask = 0;
        if (mask & U3DClear::COLOR) {
            glClearColor(col[0], col[1], col[2], col[3]);
            glMask |= GL_COLOR_BUFFER_BIT;
        }
        if (mask & U3DClear::DEPTH) {
            glClearDepth(depthVal);
            glMask |= GL_DEPTH_BUFFER_BIT;
        }
        if (mask & U3DClear::STENCIL) {
            glClearStencil(stencilVal);
            glMask |= GL_STENCIL_BUFFER_BIT;
        }
        glClear(glMask);
    }

    Unity3DGLTexture::Unity3DGLTexture()
        : target_(0)
        , hasMipmaps_(false)
        , antialias_(true) {
    }

    Unity3DGLTexture::~Unity3DGLTexture() {

    }

    void Unity3DGLTexture::create(U3DTextureType type, bool antialias) {
        target_ = textureToGL[type];
        antialias_ = antialias;
    }

    void Unity3DGLTexture::deleteTexture(uint32 texture) {
        glDeleteTextures(1, &texture);
    }

    bool Unity3DGLTexture::initWithMipmaps(U3DMipmap* mipmaps, int mipLevels, IMAGE::ImageFormat imageFormat, uint32 imageWidth, uint32 imageHeight) {
        if (mipLevels <= 0) {
            return false;
        }

        if (imageFormatInfoMap().find(imageFormat) == imageFormatInfoMap().end()) {
            return false;
        }

        const IMAGE::ImageFormatInfo& info = imageFormatInfoMap().at(imageFormat);

        //Set the row align only when mipmapsNum == 1 and the data is uncompressed
        if (mipLevels == 1 && !info.compressed) {
            unsigned int bytesPerRow = imageWidth * info.bpp / 8;

            if (bytesPerRow % 8 == 0)
            {
                glPixelStorei(GL_UNPACK_ALIGNMENT, 8);
            }
            else if (bytesPerRow % 4 == 0)
            {
                glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
            }
            else if (bytesPerRow % 2 == 0)
            {
                glPixelStorei(GL_UNPACK_ALIGNMENT, 2);
            }
            else
            {
                glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
            }
        }
        else {
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        }

        if (texture_ != 0) {
            glDeleteTextures(1, &texture_);
            texture_ = 0;
        }

        glGenTextures(1, &texture_);
        Unity3DGLState::BindTexture2D(texture_);

        if (mipLevels == 1) {
            glTexParameteri(target_, GL_TEXTURE_MIN_FILTER, antialias_ ? GL_LINEAR : GL_NEAREST);
        }
        else {
            glTexParameteri(target_, GL_TEXTURE_MIN_FILTER, antialias_ ? GL_LINEAR_MIPMAP_NEAREST : GL_NEAREST_MIPMAP_NEAREST);
        }

        glTexParameteri(target_, GL_TEXTURE_MAG_FILTER, antialias_ ? GL_LINEAR : GL_NEAREST);
        glTexParameteri(target_, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(target_, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        // Specify OpenGL texture image
        int width = imageWidth;
        int height = imageHeight;

        for (int i = 0; i < mipLevels; ++i) {
            unsigned char *data = mipmaps[i].address;
            GLsizei datalen = mipmaps[i].length;

            if (info.compressed) {
                glCompressedTexImage2D(target_, i, info.internalFormat, (GLsizei) width, (GLsizei) height, 0, datalen, data);
            }
            else {
                glTexImage2D(target_, i, info.internalFormat, (GLsizei) width, (GLsizei) height, 0, info.format, info.type, data);
            }

            width = MATH::MATH_MAX(width >> 1, 1);
            height = MATH::MATH_MAX(height >> 1, 1);
        }

        imageFormat_ = imageFormat;
        width_ = imageWidth;
        height_ = imageHeight;
        premultipliedAlpha_ = false;
        hasMipmaps_ = mipLevels > 1;

        return true;
    }

    bool Unity3DGLTexture::updateWithData(const void *data, int offsetX, int offsetY, int width, int height) {
        if (texture_) {
            Unity3DGLState::BindTexture2D(texture_);
            const IMAGE::ImageFormatInfo& info = imageFormatInfoMap().at(imageFormat_);
            glTexSubImage2D(target_, 0, offsetX, offsetY, width, height, info.format, info.type, data);
            return true;
        }
        return false;
    }

    void Unity3DGLTexture::setAliasTexParameters() {
        if (texture_ == 0) {
            return;
        }

        Unity3DGLState::BindTexture2D(texture_);

        if (!hasMipmaps_) {
            glTexParameteri(target_, GL_TEXTURE_MIN_FILTER, antialias_ ? GL_NEAREST : GL_LINEAR);
        }
        else {
            glTexParameteri(target_, GL_TEXTURE_MIN_FILTER, antialias_ ? GL_NEAREST_MIPMAP_NEAREST : GL_LINEAR_MIPMAP_NEAREST);
        }

        glTexParameteri(target_, GL_TEXTURE_MAG_FILTER, antialias_ ? GL_NEAREST : GL_LINEAR);
    }

    void Unity3DGLTexture::autoGenMipmaps() {
        Unity3DGLState::BindTexture2D(texture_);
        glGenerateMipmap(target_);
        hasMipmaps_ = true;
    }

    const IMAGE::ImageFormatInfoMap &Unity3DGLTexture::imageFormatInfoMap() {
        static const IMAGE::ImageFormatInfoMapValue TexturePixelFormatInfoTablesValue [] =
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

        static const IMAGE::ImageFormatInfoMap ImageFormatInfoTables(TexturePixelFormatInfoTablesValue,
            TexturePixelFormatInfoTablesValue + sizeof(TexturePixelFormatInfoTablesValue) / sizeof(TexturePixelFormatInfoTablesValue[0]));

        return ImageFormatInfoTables;
    }

    Unity3DContext *Unity3DGLCreator::CreateContext() {
        return new Unity3DGLContext;
    }

    Unity3DDepthState *Unity3DGLCreator::CreateDepthState() {
        return new Unity3DGLDepthState();
    }

    Unity3DBuffer *Unity3DGLCreator::CreateBuffer(uint32 usageFlags) {
        return new Unity3DGLBuffer(usageFlags);
    }

    Unity3DShaderSet *Unity3DGLCreator::CreateShaderSet(Unity3DShader *vshader, Unity3DShader *fshader) {
        if (!vshader || !fshader) {
            throw _HException_Normal(UTILS::STRING::StringFromFormat("ShaderSet requires both a valid vertex and a fragment shader: %p %p", vshader, fshader));
        }

        Unity3DGLShaderSet *shaderSet = new Unity3DGLShaderSet(vshader, fshader);
        shaderSet->link();
        shaderSet->updateUniforms();
        return shaderSet;
    }

    Unity3DShaderSet *Unity3DGLCreator::CreateShaderSetWithByteArray(const std::string &vShaderByteArray, const std::string &fShaderByteArray, const std::string& compileTimeDefines) {
        if (vShaderByteArray.empty() || fShaderByteArray.empty()) {
            throw _HException_Normal(UTILS::STRING::StringFromFormat("ShaderSet requires both a valid vertex and a fragment ByteArray: %p %p", &vShaderByteArray, &fShaderByteArray));
        }

        Unity3DGLShaderSet *shaderSet = new Unity3DGLShaderSet();
        shaderSet->initWithByteArrays(vShaderByteArray.c_str(), fShaderByteArray.c_str(), compileTimeDefines);
        shaderSet->link();
        shaderSet->updateUniforms();
        return shaderSet;
    }

    Unity3DShaderSet *Unity3DGLCreator::CreateShaderSetWithFileName(const std::string& vShaderFilename, const std::string& fShaderFilename, const std::string& compileTimeDefines) {
        if (vShaderFilename.empty() || fShaderFilename.empty()) {
            throw _HException_Normal(UTILS::STRING::StringFromFormat("ShaderSet requires both a valid vertex and a fragment ByteArray: %p %p", &vShaderFilename, &fShaderFilename));
        }

        Unity3DGLShaderSet *shaderSet = new Unity3DGLShaderSet();
        shaderSet->initWithFilenames(vShaderFilename.c_str(), fShaderFilename.c_str(), compileTimeDefines);
        shaderSet->link();
        shaderSet->updateUniforms();
        return shaderSet;
    }

    Unity3DVertexFormat *Unity3DGLCreator::CreateVertexFormat(const U3DVertexComponent &component) {
        Unity3DGLVertexFormat *vertexFormat = new Unity3DGLVertexFormat(component);
        vertexFormat->compile();
        return vertexFormat;
    }

    Unity3DVertexFormat *Unity3DGLCreator::CreateVertexFormat(const std::vector<U3DVertexComponent> &components) {
        Unity3DGLVertexFormat *vertexFormat = new Unity3DGLVertexFormat(components);
        vertexFormat->compile();
        return vertexFormat;
    }

    Unity3DUniformFormat *Unity3DGLCreator::CreateUniformFormat(Unity3DShaderSet * u3dShader, const U3DuniformComponent &component) {
        return new Unity3DGLUniformFormat(u3dShader, component);
    }

    Unity3DTexture *Unity3DGLCreator::CreateTexture(U3DTextureType type, bool antialias) {
        Unity3DGLTexture *texture = new Unity3DGLTexture();
        texture->create(type, antialias);
        return texture;
    }
}

