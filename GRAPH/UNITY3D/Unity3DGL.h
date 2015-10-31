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

    static const unsigned short compToGL [] =
    {
        GL_NEVER,
        GL_LESS,
        GL_EQUAL,
        GL_LEQUAL,
        GL_GREATER,
        GL_NOTEQUAL,
        GL_GEQUAL,
        GL_ALWAYS
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

    static const unsigned short primToGL [] =
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

        Unity3DDepthState *createDepthState() override;
        Unity3DBuffer *createBuffer(uint32 usageFlags) override;
        Unity3DShaderSet *createShaderSet(Unity3DShader *vshader, Unity3DShader *fshader) override;
        Unity3DVertexFormat *createVertexFormat(const std::vector<Unity3DVertexComponent> &components, int stride) override;

        void setDepthState(Unity3DDepthState *state);

        // TODO: Add more sophisticated draws with buffer offsets, and multidraws.
        void draw(U3DPrimitive prim, Unity3DVertexFormat *format, Unity3DBuffer *vdata, int vertexCount, int offset) override;
        void drawIndexed(U3DPrimitive prim, Unity3DVertexFormat *format, Unity3DBuffer *vdata, Unity3DBuffer *idata, void *indices, int offset) override;
        void drawUp(U3DPrimitive prim, Unity3DVertexFormat *format, const void *vdata, int vertexCount) override;
        void clear(int mask, uint32 colorval, float depthVal, int stencilVal) override;
    };
}

#endif // UNITY3DGL_H
