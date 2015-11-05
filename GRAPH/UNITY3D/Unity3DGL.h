#ifndef UNITY3DGL_H
#define UNITY3DGL_H

#include <map>
#include "GRAPH/UNITY3D/Unity3D.h"
#include "GRAPH/UNITY3D/Unity3DGLState.h"

namespace GRAPH
{
    class Unity3DGLBlendState : public Unity3DBlendState
    {
    public:
        bool enabled;
        GLuint eqCol, eqAlpha;
        GLuint srcCol, srcAlpha, dstCol, dstAlpha;
        bool logicEnabled;
        GLuint logicOp;

        void loadDefault() override;
        void apply() override;
    };

    class Unity3DGLDepthState : public Unity3DDepthState
    {
    public:
        Unity3DGLDepthState();

        void loadDefault() override;
        void setDepthTest(bool depthTest) override;
        void setDepthWrite(bool depthWriteEnabled) override;
        void setDepthComp(uint32 depthComp) override;
        void apply() override;
    };

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

    static const uint32 primToGL [] =
    {
        GL_POINTS,
        GL_LINES,
        GL_TRIANGLES,
    };

    class Unity3DGLContext  final : public Unity3DContext
    {
    public:
        Unity3DGLContext() {}
        ~Unity3DGLContext() {}

        // TODO: Add more sophisticated draws with buffer offsets, and multidraws.
        void draw(U3DPrimitive prim, Unity3DVertexFormat *format, Unity3DBuffer *vdata, int vertexCount, int offset) override;
        void drawIndexed(U3DPrimitive prim, Unity3DVertexFormat *format, Unity3DBuffer *vdata, Unity3DBuffer *idata, void *indices, int offset) override;
        void drawUp(U3DPrimitive prim, Unity3DVertexFormat *format, const void *vdata, int vertexCount) override;
        void clear(int mask, uint32 colorval, float depthVal, int stencilVal) override;
    };

    class Unity3DGLTexture;

    class Unity3DGLCreator
    {
    public:
        static Unity3DContext *CreateContext();
        static Unity3DDepthState *CreateDepthState();
        static Unity3DBuffer *CreateBuffer(uint32 usageFlags);
        static Unity3DShaderSet *CreateShaderSet(Unity3DShader *vshader, Unity3DShader *fshader);
        static Unity3DVertexFormat *CreateVertexFormat(const std::vector<Unity3DVertexComponent> &components, int stride);
        static Unity3DTexture *CreateTexture(U3DTextureType type = LINEAR2D, bool antialias = true);
    };
}

#endif // UNITY3DGL_H
