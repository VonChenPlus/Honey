#ifndef UNITY3DGL_H
#define UNITY3DGL_H

#include <map>
#include "GRAPH/UNITY3D/Unity3D.h"
#include "GRAPH/UNITY3D/GLState.h"

namespace GRAPH
{
    class Unity3DGLBuffer : public Unity3DBuffer
    {
    public:
        Unity3DGLBuffer(uint32 flags);
        ~Unity3DGLBuffer() override;

        void setData(const uint8 *data, size_t size) override;
        void subData(const uint8 *data, size_t offset, size_t size) override;
        void bind();

        GLuint buffer_;
    private:
        
        GLuint target_;
        GLuint usage_;
        size_t knownSize_;
    };

    class Unity3DGLVertexFormat : public Unity3DVertexFormat
    {
    public:
        Unity3DGLVertexFormat(const std::vector<Unity3DVertexComponent> &components, int stride);
        void apply(const void *base = nullptr);
        void unApply();
        void compile();

    private:
        std::vector<Unity3DVertexComponent> components_;
        int semanticsMask_;
        int stride_;
    };
}

#endif // UNITY3DGL_H
