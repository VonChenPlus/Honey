#ifndef UNITY3DGL_H
#define UNITY3DGL_H

#include <map>
#include "GRAPH/UNITY3D/Unity3D.h"
#include "GRAPH/UNITY3D/Unity3DGLState.h"

namespace GRAPH
{
    class Unity3DGLBuffer : public Unity3DBuffer
    {
    public:
        Unity3DGLBuffer(uint32 flags);
        ~Unity3DGLBuffer() override;

        void setData(const uint8 *data, uint64 size) override;
        void subData(const uint8 *data, uint64 offset, uint64 size) override;
        void bind() override;

    private:
        GLuint buffer_;
        GLuint target_;
        GLuint usage_;
        uint64 knownSize_;
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

    static const unsigned short primToGL [] =
    {
        GL_POINTS,
        GL_LINES,
        GL_TRIANGLES,
    };

    class Unity3DGLContext : public Unity3DContext
    {
    public:
        Unity3DGLContext() {}
        virtual ~Unity3DGLContext() {}

        virtual Unity3DBuffer *createBuffer(uint32 usageFlags) override;
        virtual Unity3DShaderSet *createShaderSet(Unity3DShader *vshader, Unity3DShader *fshader) override;
        virtual Unity3DVertexFormat *createVertexFormat(const std::vector<Unity3DVertexComponent> &components, int stride) override;

        // TODO: Add more sophisticated draws with buffer offsets, and multidraws.
        virtual void draw(U3DPrimitive prim, Unity3DShaderSet *pipeline, Unity3DVertexFormat *format, Unity3DBuffer *vdata, int vertexCount, int offset) override;
        virtual void drawIndexed(U3DPrimitive prim, Unity3DVertexFormat *format, Unity3DBuffer *vdata, Unity3DBuffer *idata, void *indices, int offset) override;
        virtual void drawUp(U3DPrimitive prim, Unity3DShaderSet *pipeline, Unity3DVertexFormat *format, const void *vdata, int vertexCount) override;
        virtual void clear(int mask, uint32 colorval, float depthVal, int stencilVal) override;
    };
}

#endif // UNITY3DGL_H
