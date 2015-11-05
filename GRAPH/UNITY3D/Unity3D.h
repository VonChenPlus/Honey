#ifndef UNITY3D_H
#define UNITY3D_H

#include <vector>
#include <functional>
#include "BASE/HObject.h"
#include "BASE/HData.h"
#include "IMAGE/ImageDefine.h"
#include "IMAGE/ImageObject.h"

namespace GRAPH
{
    enum U3DBlendEquation : int
    {
        ADD,
        SUBTRACT,
        REV_SUBTRACT,
        MIN,
        MAX,
    };

    enum U3DComparison : int
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
    enum U3DLogicOp : int
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

    enum U3DBlendFactor : int
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

    enum U3DBufferUsage : int
    {
        VERTEXDATA = 1,
        INDEXDATA = 2,
        GENERIC = 4,
        DYNAMIC = 8,
        STREAM = 16,
    };

    enum U3DVertexDataType : uint8
    {
        INVALID,
        FLOATx2,
        FLOATx3,
        FLOATx4,
        UNORM8x4,
    };

    enum U3DSemantic : int
    {
        SEM_POSITION,
        SEM_COLOR0,
        SEM_TEXCOORD0,
        SEM_TEXCOORD1,
        SEM_TEXCOORD2,
        SEM_TEXCOORD3,
        SEM_NORMAL,
        SEM_TANGENT,
        SEM_BINORMAL,  // really BITANGENT
        SEM_MAX,
    };

    enum U3DPrimitive : int
    {
        PRIM_POINTS,
        PRIM_LINES,
        PRIM_TRIANGLES,
    };

    enum U3DVertexShaderPreset : int
    {
        VS_COLOR_2D,
        VS_TEXTURE_COLOR_2D,
        VS_MAX_PRESET,
    };

    enum U3DFragmentShaderPreset : int
    {
        FS_COLOR_2D,
        FS_TEXTURE_COLOR_2D,
        FS_MAX_PRESET,
    };

    // Predefined full shader setups.
    enum U3DShaderSetPreset : int
    {
        SS_COLOR_2D,
        SS_TEXTURE_COLOR_2D,
        SS_MAX_PRESET,
    };

    enum U3DBlendStatePreset : int
    {
        BS_OFF,
        BS_STANDARD_ALPHA,
        BS_PREMUL_ALPHA,
        BS_ADDITIVE,
        BS_MAX_PRESET,
    };

    enum U3DClear : int
    {
        COLOR = 1,
        DEPTH = 2,
        STENCIL = 4,
    };

    enum U3DTextureType : uint8
    {
        LINEAR1D,
        LINEAR2D,
        LINEAR3D,
        CUBE,
        ARRAY1D,
        ARRAY2D,
        UNKNOWN,
    };

    enum U3DRenderState : uint8
    {
        CULL_MODE,
    };

    enum U3DCullMode : uint8
    {
        NO_CULL,
        CW,
        CCW,
    };

    // Binary compatible with D3D11 viewport.
    struct U3DViewport
    {
        U3DViewport(float left, float top, float width, float height)
            : TopLeftX(left)
            , TopLeftY(top)
            , Width(width)
            , Height(height) {

        }
        U3DViewport() {
            TopLeftX = TopLeftY = 0.f;
            Width = Height = 1.0f;
        }

        float TopLeftX;
        float TopLeftY;
        float Width;
        float Height;
        float MinDepth;
        float MaxDepth;
    };

    typedef HObject Unity3DObject;

    class Unity3DBlendState : public Unity3DObject
    {
    public:
        virtual void loadDefault() = 0;
        virtual void apply() = 0;
    };

    class Unity3DDepthState : public Unity3DObject
    {
    public:
        virtual void loadDefault() = 0;
        virtual void setDepthTest(bool depthTest) = 0;
        virtual void setDepthWrite(bool depthWrite) = 0;
        virtual void setDepthComp(uint32 depthComp) = 0;
        virtual void apply() = 0;
        
    protected:
        bool depthTest_;
        bool depthWrite_;
        uint32 depthComp_;
    };

    class Unity3DBuffer : public Unity3DObject
    {
    public:
        virtual void setData(const uint8 *data, uint64 size) = 0;
        virtual void subData(const uint8 *data, uint64 offset, uint64 size) = 0;
        virtual void bind() = 0;
    };

    struct Unity3DVertexComponent
    {
        Unity3DVertexComponent() : type(U3DVertexDataType::INVALID), semantic(255), offset(255) {}
        Unity3DVertexComponent(U3DSemantic semantic, U3DVertexDataType dataType, uint8 offset) {
            this->semantic = semantic;
            this->type = dataType;
            this->offset = offset;
        }
        U3DVertexDataType type;
        uint8 semantic;
        uint8 offset;
    };

    class Unity3DVertexFormat : public Unity3DObject
    {
    public:
    };

    class Unity3DShader : public Unity3DObject
    {
    public:
    };

    class Unity3DShaderSet : public Unity3DObject
    {
    public:
    };

    class Unity3DContext : public Unity3DObject
    {
    public:
        Unity3DContext() {}
        virtual ~Unity3DContext() {}

        // TODO: Add more sophisticated draws with buffer offsets, and multidraws.
        virtual void draw(U3DPrimitive prim, Unity3DVertexFormat *format, Unity3DBuffer *vdata, int vertexCount, int offset) = 0;
        virtual void drawIndexed(U3DPrimitive prim, Unity3DVertexFormat *format, Unity3DBuffer *vdata, Unity3DBuffer *idata, void *indices, int offset) = 0;
        virtual void drawUp(U3DPrimitive prim, Unity3DVertexFormat *format, const void *vdata, int vertexCount) = 0;
        virtual void clear(int mask, uint32 colorval, float depthVal, int stencilVal) = 0;
    };

    struct U3DMipmap
    {
        U3DMipmap()
            : address(nullptr)
            , length(0){

        }
        uint8* address;
        int length;
    };

    typedef std::function<HData(void *owner, const char * string, uint32 &width, uint32 &height, bool& premultipliedAlpha)> U3DStringToTexture;

    class Unity3DTexture : public Unity3DObject
    {
    public:
        Unity3DTexture() 
            : texture_(0)
            , width_(0)
            , height_(0)
            , premultipliedAlpha_(false) {
        }

        virtual ~Unity3DTexture() {
            deleteTexture(texture_);
            texture_ = 0;
        }

        virtual void create(U3DTextureType type, bool antialias = true) = 0;  
        virtual void deleteTexture(uint32) { };

        bool initWithData(const void *data, uint64 dataLen, IMAGE::ImageFormat imageFormat, uint32 imageWidth, uint32 imageHeight);
        bool initWithString(const char *string, U3DStringToTexture loader = nullptr, void *loaderOwner = nullptr);

        bool initWithImage(IMAGE::ImageObject * image);
        bool initWithImage(IMAGE::ImageObject * image, IMAGE::ImageFormat format);

        virtual bool initWithMipmaps(U3DMipmap* mipmaps, int mipLevels, IMAGE::ImageFormat imageFormat, uint32 imageWidth, uint32 imageHeight) = 0;
        virtual bool updateWithData(const void *data, int offsetX, int offsetY, int width, int height) = 0;

        virtual void setAliasTexParameters() = 0;
        virtual void autoGenMipmaps() = 0;

        uint32 texture() { return texture_;  }
        int width() { return width_; }
        int height() { return height_; }

        bool hasPremultipliedAlpha() const { return premultipliedAlpha_; }
        virtual bool hasMipmaps() const = 0;

        virtual const IMAGE::ImageFormatInfoMap &imageFormatInfoMap() = 0;

    protected:
        uint32 texture_;
        int width_;
        int height_;
        bool premultipliedAlpha_;
    };

    class Unity3DCreator : public Unity3DObject
    {
    public:
        enum RenderEngine
        {
            OPENGL,
            D3D
        };
        static RenderEngine EngineMode;

        static Unity3DContext *CreateContext();
        static Unity3DDepthState *CreateDepthState();
        static Unity3DBuffer *CreateBuffer(uint32 usageFlags);
        static Unity3DShaderSet *CreateShaderSet(Unity3DShader *vshader, Unity3DShader *fshader);
        static Unity3DVertexFormat *CreateVertexFormat(const std::vector<Unity3DVertexComponent> &components, int stride);
        static Unity3DTexture *CreateTexture(U3DTextureType type = LINEAR2D, bool antialias = true);
    };
}

#endif // UNITY3D_H
