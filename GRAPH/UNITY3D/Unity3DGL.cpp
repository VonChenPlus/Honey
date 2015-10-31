#include "GRAPH/UNITY3D/Unity3DGL.h"
#include "GRAPH/UNITY3D/Unity3DGLShader.h"
#include "UTILS/STRING/StringUtils.h"

namespace GRAPH
{
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
            Unity3DGLState::DefaultState().arrayBuffer.bind(buffer_);
        }
        else {
            Unity3DGLState::DefaultState().elementArrayBuffer.bind(buffer_);
        }
    }

    Unity3DGLVertexFormat::Unity3DGLVertexFormat(const std::vector<Unity3DVertexComponent> &components, int stride) {
        components_ = components;
        stride_ = stride;
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
                glVertexAttribPointer(components_[i].semantic, 2, GL_FLOAT, GL_FALSE, stride_, (void *) (b + (intptr) components_[i].offset));
                break;
            case FLOATx3:
                glVertexAttribPointer(components_[i].semantic, 3, GL_FLOAT, GL_FALSE, stride_, (void *) (b + (intptr) components_[i].offset));
                break;
            case FLOATx4:
                glVertexAttribPointer(components_[i].semantic, 4, GL_FLOAT, GL_FALSE, stride_, (void *) (b + (intptr) components_[i].offset));
                break;
            case UNORM8x4:
                glVertexAttribPointer(components_[i].semantic, 4, GL_UNSIGNED_BYTE, GL_TRUE, stride_, (void *) (b + (intptr) components_[i].offset));
                break;
            case INVALID:
                throw _HException_Normal("Unity3DGLVertexFormat: Invalid component type applied");
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

    Unity3DBuffer *Unity3DGLContext::createBuffer(uint32 usageFlags) {
        return new Unity3DGLBuffer(usageFlags);
    }

    Unity3DShaderSet *Unity3DGLContext::createShaderSet(Unity3DShader *vshader, Unity3DShader *fshader) {
        if (!vshader || !fshader) {
            throw _HException_Normal(UTILS::STRING::StringFromFormat("ShaderSet requires both a valid vertex and a fragment shader: %p %p", vshader, fshader));
        }

        Unity3DGLShaderSet *shaderSet = new Unity3DGLShaderSet(vshader, fshader);
        shaderSet->link();
        shaderSet->updateUniforms();
        shaderSet->autorelease();
        return shaderSet;
    }

    Unity3DVertexFormat *Unity3DGLContext::createVertexFormat(const std::vector<Unity3DVertexComponent> &components, int stride) {
        Unity3DGLVertexFormat *vertexFormat = new Unity3DGLVertexFormat(components, stride);
        vertexFormat->compile();
        return vertexFormat;
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
}

