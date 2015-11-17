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

    class Unity3DGLVertexFormat final : public Unity3DVertexFormat
    {
    public:
        Unity3DGLVertexFormat(const U3DVertexComponent &component);
        Unity3DGLVertexFormat(const std::vector<U3DVertexComponent> &components);
        void apply(const void *base = nullptr) override;
        void unApply() override;
        void compile();

    protected:
        int semanticsMask_;
    };

    class Unity3DGLUniformFormat final : public Unity3DUniformFormat
    {
    public:
        Unity3DGLUniformFormat(Unity3DShaderSet *u3dShader, const U3DuniformComponent &component);

        void applyArray() override;
        void applyValue() override;

    private:
        Unity3DShaderSet *u3dShader_;
    };

    static const uint32 primToGL [] =
    {
        GL_POINTS,
        GL_LINES,
        GL_TRIANGLES,
        GL_TRIANGLE_STRIP,
        GL_TRIANGLE_FAN
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

    static const uint32 textureToGL [] =
    {
        GL_TEXTURE_1D,
        GL_TEXTURE_2D,
        GL_TEXTURE_3D,
        GL_TEXTURE_CUBE_MAP,
        GL_TEXTURE_1D_ARRAY,
        GL_TEXTURE_2D_ARRAY,
        GL_NONE
    };

    class Unity3DGLTexture final : public Unity3DTexture
    {
    public:
        Unity3DGLTexture();
        ~Unity3DGLTexture();

        void create(U3DTextureType type, bool antialias = true) override;
        void deleteTexture(uint32 texture) override;

        bool initWithMipmaps(U3DMipmap* mipmaps, int mipLevels, IMAGE::ImageFormat imageFormat, uint32 imageWidth, uint32 imageHeight) override;
        bool updateWithData(const void *data, int offsetX, int offsetY, int width, int height) override;

        void setAliasTexParameters() override;
        void autoGenMipmaps() override;

        bool hasMipmaps() const override { return hasMipmaps_; }

        const IMAGE::ImageFormatInfoMap &imageFormatInfoMap() override;

    private:
        GLuint target_;
        bool hasMipmaps_;
        bool antialias_;
        IMAGE::ImageFormat imageFormat_;
    };

    class Unity3DGLCreator
    {
    public:
        static Unity3DContext *CreateContext();
        static Unity3DDepthState *CreateDepthState();
        static Unity3DBuffer *CreateBuffer(uint32 usageFlags);
        static Unity3DShaderSet *CreateShaderSet(Unity3DShader *vshader, Unity3DShader *fshader);
        static Unity3DShaderSet *CreateShaderSetWithByteArray(const std::string &vShaderByteArray, const std::string &fShaderByteArray, const std::string& compileTimeDefines = std::string());
        static Unity3DShaderSet *CreateShaderSetWithFileName(const std::string& vShaderFilename, const std::string& fShaderFilename, const std::string& compileTimeDefines = std::string());
        static Unity3DVertexFormat *CreateVertexFormat(const U3DVertexComponent &component);
        static Unity3DVertexFormat *CreateVertexFormat(const std::vector<U3DVertexComponent> &components);
        static Unity3DUniformFormat *CreateUniformFormat(Unity3DShaderSet * u3dShader, const U3DuniformComponent &component);
        static Unity3DTexture *CreateTexture(U3DTextureType type = LINEAR2D, bool antialias = true);
    };
}

#endif // UNITY3DGL_H
