#ifndef THIN3DGL_H
#define THIN3DGL_H

#include <map>

#include "THIN3D/Thin3D.h"
#include "GFX/GLCommon.h"
#include "GFX/GLState.h"
#include "GFX/GfxResourceHolder.h"
#include "MATH/Matrix.h"

namespace THIN3D
{
    static const unsigned short compToGL[] =
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

    static const unsigned short blendEqToGL[] =
    {
        GL_FUNC_ADD,
        GL_FUNC_SUBTRACT,
        GL_FUNC_REVERSE_SUBTRACT,
    };

    static const unsigned short blendFactorToGL[] =
    {
        GL_ZERO,
        GL_ONE,
        GL_SRC_COLOR,
        GL_SRC_ALPHA,
        GL_ONE_MINUS_SRC_COLOR,
        GL_ONE_MINUS_SRC_ALPHA,
        GL_DST_COLOR,
        GL_DST_ALPHA,
        GL_ONE_MINUS_DST_COLOR,
        GL_ONE_MINUS_DST_ALPHA,
        GL_CONSTANT_COLOR,
    };

    static const unsigned short logicOpToGL[] =
    {
        GL_CLEAR,
        GL_SET,
        GL_COPY,
        GL_COPY_INVERTED,
        GL_NOOP,
        GL_INVERT,
        GL_AND,
        GL_NAND,
        GL_OR,
        GL_NOR,
        GL_XOR,
        GL_EQUIV,
        GL_AND_REVERSE,
        GL_AND_INVERTED,
        GL_OR_REVERSE,
        GL_OR_INVERTED,
    };

    static const unsigned short primToGL[] =
    {
        GL_POINTS,
        GL_LINES,
        GL_TRIANGLES,
    };

    static const char *glsl_fragment_prelude =
    "#ifdef GL_ES\n"
    "precision mediump float;\n"
    "#endif\n";

    static inline void Uint32ToFloat4(uint32 u, float f[4]) {
        f[0] = ((u >> 0) & 0xFF) * (1.0f / 255.0f);
        f[1] = ((u >> 8) & 0xFF) * (1.0f / 255.0f);
        f[2] = ((u >> 16) & 0xFF) * (1.0f / 255.0f);
        f[3] = ((u >> 24) & 0xFF) * (1.0f / 255.0f);
    }

    class Thin3DGLBlendState : public Thin3DBlendState
    {
    public:
        bool enabled;
        GLuint eqCol, eqAlpha;
        GLuint srcCol, srcAlpha, dstCol, dstAlpha;
        bool logicEnabled;
        GLuint logicOp;
        // int maskBits;
        // uint32 fixedColor;

        void apply();
    };

    class Thin3DGLDepthStencilState : public Thin3DDepthStencilState
    {
    public:
        bool depthTestEnabled;
        bool depthWriteEnabled;
        GLuint depthComp;
        // bool stencilTestEnabled; TODO

        void apply();
    };

    class Thin3DGLBuffer : public Thin3DBuffer, GFX::GfxResourceHolder
    {
    public:
        Thin3DGLBuffer(Size size, uint32 flags);
        ~Thin3DGLBuffer() override;

        void setData(const uint8 *data, Size size) override;

        void subData(const uint8 *data, Size offset, Size size) override;

        void bind();

        void glLost() override;

    private:
        GLuint buffer_;
        GLuint target_;
        GLuint usage_;

        Size knownSize_;
    };

    // Not registering this as a resource holder, instead ShaderSet is registered. It will
    // invoke Compile again to recreate the shader then link them together.
    class Thin3DGLShader : public Thin3DShader
    {
    public:
        Thin3DGLShader(bool isFragmentShader);

        bool compile(const char *source);

        GLuint getShader() const { return shader_; }
        const std::string &getSource() const { return source_; }

        ~Thin3DGLShader();

    private:
        GLuint shader_;
        GLuint type_;
        bool ok_;
        std::string source_;  // So we can recompile in case of context loss.
    };

    class Thin3DGLVertexFormat : public Thin3DVertexFormat
    {
    public:
        void apply(const void *base = NULLPTR);

        void unApply();

        void compile();

        std::vector<Thin3DVertexComponent> components_;
        int semanticsMask_;  // Fast way to check what semantics to enable/disable.
        int stride_;
    };

    struct UniformInfo
    {
        int loc_;
    };

    // TODO: Fold BlendState into this? Seems likely to be right for DX12 etc.
    // TODO: Add Uniform Buffer support.
    class Thin3DGLShaderSet : public Thin3DShaderSet, GFX::GfxResourceHolder
    {
    public:
        Thin3DGLShaderSet();
        ~Thin3DGLShaderSet();
        void link();

        void apply();

        void unApply();

        int getUniformLoc(const char *name);

        void setVector(const char *name, float *value, int n);

        void setMatrix4x4(const char *name, const MATH::Matrix4x4 &value) override;

        void glLost();

        Thin3DGLShader *vshader;
        Thin3DGLShader *fshader;

    private:
        GLuint program_;
        std::map<std::string, UniformInfo> uniforms_;
    };

    class Thin3DGLTexture : public Thin3DTexture, GFX::GfxResourceHolder
    {
    public:
        Thin3DGLTexture();
        Thin3DGLTexture(T3DTextureType type, T3DImageFormat format, int width, int height, int depth, int mipLevels);
        ~Thin3DGLTexture();

        bool create(T3DTextureType type, T3DImageFormat format, int width, int height, int depth, int mipLevels);

        void destroy();
        void setImageData(int x, int y, int z, int width, int height, int depth, int level, int stride, const uint8 *data) override;
        void autoGenMipmaps() override;

        void bind();

        void glLost();
        void finalize(int zim_flags);

    private:
        GLuint tex_;
        GLuint target_;

        T3DImageFormat format_;
        int mipLevels_;
    };

    class Thin3DGLContext : public Thin3DContext
    {
    public:
        Thin3DGLContext();
        virtual ~Thin3DGLContext();

        Thin3DDepthStencilState *createDepthStencilState(bool depthTestEnabled, bool depthWriteEnabled, T3DComparison depthCompare) override;
        Thin3DBlendState *createBlendState(const T3DBlendStateDesc &desc) override;
        Thin3DBuffer *createBuffer(Size size, uint32 usageFlags) override;
        Thin3DShaderSet *createShaderSet(Thin3DShader *vshader, Thin3DShader *fshader) override;
        Thin3DVertexFormat *createVertexFormat(const std::vector<Thin3DVertexComponent> &components, int stride, Thin3DShader *vshader) override;
        Thin3DTexture *createTexture(T3DTextureType type, T3DImageFormat format, int width, int height, int depth, int mipLevels) override;
        Thin3DTexture *createTexture() override;

        // Bound state objects
        void setBlendState(Thin3DBlendState *state) override {
            Thin3DGLBlendState *s = static_cast<Thin3DGLBlendState *>(state);
            s->apply();
        }

        // Bound state objects
        void setDepthStencilState(Thin3DDepthStencilState *state) override {
            Thin3DGLDepthStencilState *s = static_cast<Thin3DGLDepthStencilState *>(state);
            s->apply();
        }

        // The implementation makes the choice of which shader code to use.
        Thin3DShader *createVertexShader(const char *glsl_source, const char *hlsl_source);
        Thin3DShader *createFragmentShader(const char *glsl_source, const char *hlsl_source);

        void setScissorEnabled(bool enable) override {
            GFX::glstate.scissorTest.set(enable);
        }

        void setScissorRect(int left, int top, int width, int height) override {
            GFX::glstate.scissorRect.set(left, targetHeight_ - (top + height), width, height);
        }

        void setViewports(int count, T3DViewport *viewports) override {
            UNUSED(count);
            // TODO: Add support for multiple viewports.
            GFX::glstate.viewport.set(viewports[0].TopLeftX, viewports[0].TopLeftY, viewports[0].Width, viewports[0].Height);
            GFX::glstate.depthRange.set(viewports[0].MinDepth, viewports[0].MaxDepth);
        }

        void setTextures(int start, int count, Thin3DTexture **textures) override;

        void setRenderState(T3DRenderState rs, uint32 value) override;

        // TODO: Add more sophisticated draws.
        void draw(T3DPrimitive prim, Thin3DShaderSet *shaderSet, Thin3DVertexFormat *format, Thin3DBuffer *vdata, int vertexCount, int offset) override;
        void drawIndexed(T3DPrimitive prim, Thin3DShaderSet *shaderSet, Thin3DVertexFormat *format, Thin3DBuffer *vdata, Thin3DBuffer *idata, int vertexCount, int offset) override;
        void drawUP(T3DPrimitive prim, Thin3DShaderSet *shaderSet, Thin3DVertexFormat *format, const void *vdata, int vertexCount) override;
        void clear(int mask, uint32 colorval, float depthVal, int stencilVal) override;

        const char *getInfoString(T3DInfo info) const override {
            // TODO: Make these actually query the right information
            switch (info) {
                case APINAME:
                    return "OpenGL";
                case VENDOR: return (const char *)glGetString(GL_VENDOR);
                case RENDERER: return (const char *)glGetString(GL_RENDERER);
                case SHADELANGVERSION: return (const char *)glGetString(GL_SHADING_LANGUAGE_VERSION);
                case APIVERSION: return (const char *)glGetString(GL_VERSION);
                default: return "?";
            }
        }
    };
}

#endif // THIN3DGL_H
