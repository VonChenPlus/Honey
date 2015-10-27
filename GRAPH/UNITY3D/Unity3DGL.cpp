#include "GRAPH/UNITY3D/Unity3DGL.h"

namespace GRAPH
{
    Unity3DGLBuffer::Unity3DGLBuffer(uint32 flags) {
        glGenBuffers(1, &buffer_);
        target_ = (flags & U3DBufferUsage::INDEXDATA) ? GL_ELEMENT_ARRAY_BUFFER : GL_ARRAY_BUFFER;
        usage_ = 0;
        if (flags & U3DBufferUsage::DYNAMIC)
            usage_ = GL_DYNAMIC_DRAW;
        else
            usage_ = GL_STATIC_DRAW;
        knownSize_ = 0;
    }

    Unity3DGLBuffer::~Unity3DGLBuffer() {
        glDeleteBuffers(1, &buffer_);
    }

    void Unity3DGLBuffer::setData(const uint8 *data, size_t size) {
        bind();
        glBufferData(target_, size, data, usage_);
        knownSize_ = size;
    }

    void Unity3DGLBuffer::subData(const uint8 *data, size_t offset, size_t size) {
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
            GLState::DefaultState().arrayBuffer.bind(buffer_);
        }
        else {
            GLState::DefaultState().elementArrayBuffer.bind(buffer_);
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
        for (size_t i = 0; i < components_.size(); i++) {
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
}

