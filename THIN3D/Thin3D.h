#ifndef THIN3D_H
#define THIN3D_H

#include <vector>
#include <string>

#include "BASE/BasicTypes.h"
#include "MATH/Matrix.h"

namespace THIN3D
{
    enum T3DBlendEquation : int
    {
        ADD,
        SUBTRACT,
        REV_SUBTRACT,
        MIN,
        MAX,
    };

    enum T3DComparison : int
    {
        NEVER,
        LESS,
        EQUAL,
        LESS_EQUAL,
        GREATER,
        NOT_EQUAL,
        GREATER_EQUAL,
        ALWAYS,
    };

    // Had to prefix with LOGIC, too many clashes
    enum T3DLogicOp : int
    {
        LOGIC_CLEAR,
        LOGIC_SET,
        LOGIC_COPY,
        LOGIC_COPY_INVERTED,
        LOGIC_NOOP,
        LOGIC_INVERT,
        LOGIC_AND,
        LOGIC_NAND,
        LOGIC_OR,
        LOGIC_NOR,
        LOGIC_XOR,
        LOGIC_EQUIV,
        LOGIC_AND_REVERSE,
        LOGIC_AND_INVERTED,
        LOGIC_OR_REVERSE,
        LOGIC_OR_INVERTED,
    };

    enum T3DBlendFactor : int
    {
        ZERO,
        ONE,
        SRC_COLOR,
        SRC_ALPHA,
        ONE_MINUS_SRC_COLOR,
        ONE_MINUS_SRC_ALPHA,
        DST_COLOR,
        DST_ALPHA,
        ONE_MINUS_DST_COLOR,
        ONE_MINUS_DST_ALPHA,
        FIXED_COLOR,
    };

    enum T3DBufferUsage : int
    {
        VERTEXDATA = 1,
        INDEXDATA = 2,
        GENERIC = 4,

        DYNAMIC = 16,
    };

    enum T3DVertexDataType : uint8
    {
        INVALID,
        FLOATx2,
        FLOATx3,
        FLOATx4,
        UNORM8x4,
    };

    enum T3DSemantic : int
    {
        SEM_POSITION,
        SEM_COLOR0,
        SEM_TEXCOORD0,
        SEM_TEXCOORD1,
        SEM_NORMAL,
        SEM_TANGENT,
        SEM_BINORMAL,  // really BITANGENT
        SEM_MAX,
    };

    enum T3DPrimitive : int
    {
        PRIM_POINTS,
        PRIM_LINES,
        PRIM_TRIANGLES,
    };

    enum T3DVertexShaderPreset : int
    {
        VS_COLOR_2D,
        VS_TEXTURE_COLOR_2D,
        VS_MAX_PRESET,
    };

    enum T3DFragmentShaderPreset : int
    {
        FS_COLOR_2D,
        FS_TEXTURE_COLOR_2D,
        FS_MAX_PRESET,
    };

    // Predefined full shader setups.
    enum T3DShaderSetPreset : int
    {
        SS_COLOR_2D,
        SS_TEXTURE_COLOR_2D,
        SS_MAX_PRESET,
    };

    enum T3DBlendStatePreset : int
    {
        BS_OFF,
        BS_STANDARD_ALPHA,
        BS_PREMUL_ALPHA,
        BS_ADDITIVE,
        BS_MAX_PRESET,
    };

    enum T3DClear : int
    {
        COLOR = 1,
        DEPTH = 2,
        STENCIL = 4,
    };

    enum T3DTextureType : uint8
    {
        UNKNOWN,
        LINEAR1D,
        LINEAR2D,
        LINEAR3D,
        CUBE,
        ARRAY1D,
        ARRAY2D,
    };

    enum T3DImageFormat : uint8
    {
        IMG_UNKNOWN,
        LUMINANCE,
        RGBA8888,
        RGBA4444,
        DXT1,
        ETC1,  // Needs simulation on many platforms
        D16,
        D24S8,
        D24X8,
    };

    enum T3DRenderState : uint8
    {
        CULL_MODE,
    };

    enum T3DCullMode : uint8
    {
        NO_CULL,
        CW,
        CCW,
    };

    enum T3DImageType
    {
        PNG,
        JPEG,
        ZIM,
        DETECT,
        TYPE_UNKNOWN,
    };

    enum T3DInfo
    {
        APINAME,
        APIVERSION,
        VENDOR,
        SHADELANGVERSION,
        RENDERER,
    };

    // Binary compatible with D3D11 viewport.
    struct T3DViewport
    {
        float TopLeftX;
        float TopLeftY;
        float Width;
        float Height;
        float MinDepth;
        float MaxDepth;
    };

    class Thin3DObject
    {
    public:
        Thin3DObject() : refcount_(1) {}
        virtual ~Thin3DObject() {}

        virtual void addRef() { refcount_++; }
        virtual void release() { refcount_--; if (!refcount_) delete this; }

    private:
        int refcount_;
    };

    class Thin3DBlendState : public Thin3DObject
    {
    public:
    };

    class Thin3DDepthStencilState : public Thin3DObject
    {
    public:
    };

    class Thin3DBuffer : public Thin3DObject
    {
    public:
        virtual void setData(const uint8_t *data, size_t size) = 0;
        virtual void subData(const uint8_t *data, size_t offset, size_t size) = 0;
    };

    class Thin3DTexture : public Thin3DObject
    {
    public:
        bool loadFromFile(const std::string &filename, T3DImageType type = T3DImageType::DETECT);
        bool loadFromFileData(const uint8_t *data, size_t dataSize, T3DImageType type = T3DImageType::DETECT);

        virtual bool create(T3DTextureType type, T3DImageFormat format, int width, int height, int depth, int mipLevels) = 0;
        virtual void setImageData(int x, int y, int z, int width, int height, int depth, int level, int stride, const uint8_t *data) = 0;
        virtual void autoGenMipmaps() = 0;
        virtual void finalize(int zim_flags) = 0;  // TODO: Tidy up

        int width() { return width_; }
        int height() { return height_; }
        int depth() { return depth_; }
    protected:
        std::string filename_;  // Textures that are loaded from files can reload themselves automatically.
        int width_, height_, depth_;
    };

    struct Thin3DVertexComponent
    {
        Thin3DVertexComponent() : name(NULLPTR), type(T3DVertexDataType::INVALID), semantic(255), offset(255) {}
        Thin3DVertexComponent(const char *name, T3DSemantic semantic, T3DVertexDataType dataType, uint8_t offset)
        {
            this->name = name;
            this->semantic = semantic;
            this->type = dataType;
            this->offset = offset;
        }
        const char *name;
        T3DVertexDataType type;
        uint8_t semantic;
        uint8_t offset;
    };

    class Thin3DVertexFormat : public Thin3DObject
    {
    public:
    };

    class Thin3DShader : public Thin3DObject
    {
    public:
    };

    class Thin3DShaderSet : public Thin3DObject
    {
    public:
        // TODO: Make some faster way of doing these. Support uniform buffers (and fake them on GL 2.0?)
        virtual void setVector(const char *name, float *value, int n) = 0;
        virtual void setMatrix4x4(const char *name, const MATH::Matrix4x4 &value) = 0;
    };

    struct T3DBlendStateDesc
    {
        bool enabled;
        T3DBlendEquation eqCol;
        T3DBlendFactor srcCol;
        T3DBlendFactor dstCol;
        T3DBlendEquation eqAlpha;
        T3DBlendFactor srcAlpha;
        T3DBlendFactor dstAlpha;
        bool logicEnabled;
        T3DLogicOp logicOp;
        // int colorMask;
    };

    class Thin3DContext : public Thin3DObject
    {
    public:
        Thin3DContext() {}
        virtual ~Thin3DContext();

        virtual Thin3DDepthStencilState *createDepthStencilState(bool depthTestEnabled, bool depthWriteEnabled, T3DComparison depthCompare) = 0;
        virtual Thin3DBlendState *createBlendState(const T3DBlendStateDesc &desc) = 0;
        virtual Thin3DBuffer *createBuffer(size_t size, uint32_t usageFlags) = 0;
        virtual Thin3DShaderSet *createShaderSet(Thin3DShader *vshader, Thin3DShader *fshader) = 0;
        virtual Thin3DVertexFormat *createVertexFormat(const std::vector<Thin3DVertexComponent> &components, int stride, Thin3DShader *vshader) = 0;

        virtual Thin3DTexture *createTexture() = 0;  // To be later filled in by ->LoadFromFile or similar.
        virtual Thin3DTexture *createTexture(T3DTextureType type, T3DImageFormat format, int width, int height, int depth, int mipLevels) = 0;

        // Common Thin3D function, uses CreateTexture
        Thin3DTexture *createTextureFromFile(const char *filename, T3DImageType fileType);
        Thin3DTexture *createTextureFromFileData(const uint8_t *data, int size, T3DImageType fileType);

        // Note that these DO NOT AddRef so you must not ->Release presets unless you manually AddRef them.
        Thin3DBlendState *getBlendStatePreset(T3DBlendStatePreset preset) { return bsPresets_[preset]; }
        Thin3DShader *getVshaderPreset(T3DVertexShaderPreset preset) { return fsPresets_[preset]; }
        Thin3DShader *getFshaderPreset(T3DFragmentShaderPreset preset) { return vsPresets_[preset]; }
        Thin3DShaderSet *getShaderSetPreset(T3DShaderSetPreset preset) { return ssPresets_[preset]; }

        // The implementation makes the choice of which shader code to use.
        virtual Thin3DShader *createVertexShader(const char *glsl_source, const char *hlsl_source) = 0;
        virtual Thin3DShader *createFragmentShader(const char *glsl_source, const char *hlsl_source) = 0;

        // Bound state objects. Too cumbersome to add them all as parameters to Draw.
        virtual void setBlendState(Thin3DBlendState *state) = 0;
        virtual void setDepthStencilState(Thin3DDepthStencilState *state) = 0;
        virtual void setTextures(int start, int count, Thin3DTexture **textures) = 0;

        void setTexture(int stage, Thin3DTexture *texture) {
            setTextures(stage, 1, &texture);
        }  // from sampler 0 and upwards

        // Raster state
        virtual void setScissorEnabled(bool enable) = 0;
        virtual void setScissorRect(int left, int top, int width, int height) = 0;
        virtual void setViewports(int count, T3DViewport *viewports) = 0;

        // Single render states that aren't worth state blocks. May have to convert some of these state blocks on D3D11 though...
        virtual void setRenderState(T3DRenderState rs, uint32_t value) = 0;

        // TODO: Add more sophisticated draws with buffer offsets, and multidraws.
        virtual void draw(T3DPrimitive prim, Thin3DShaderSet *pipeline, Thin3DVertexFormat *format, Thin3DBuffer *vdata, int vertexCount, int offset) = 0;
        virtual void drawIndexed(T3DPrimitive prim, Thin3DShaderSet *pipeline, Thin3DVertexFormat *format, Thin3DBuffer *vdata, Thin3DBuffer *idata, int vertexCount, int offset) = 0;
        virtual void drawUP(T3DPrimitive prim, Thin3DShaderSet *pipeline, Thin3DVertexFormat *format, const void *vdata, int vertexCount) = 0;
        virtual void clear(int mask, uint32_t colorval, float depthVal, int stencilVal) = 0;

        // Necessary to correctly flip scissor rectangles etc for OpenGL.
        void setTargetSize(int w, int h)
        {
            targetWidth_ = w;
            targetHeight_ = h;
        }

        virtual const char *getInfoString(T3DInfo info) const = 0;

    protected:
        void createPresets();

        Thin3DShader *vsPresets_[VS_MAX_PRESET];
        Thin3DShader *fsPresets_[FS_MAX_PRESET];
        Thin3DBlendState *bsPresets_[BS_MAX_PRESET];
        Thin3DShaderSet *ssPresets_[SS_MAX_PRESET];

        int targetWidth_;
        int targetHeight_;

    private:
    };

    Thin3DContext *T3DCreateGLContext();
}

#endif // THIN3D_H
