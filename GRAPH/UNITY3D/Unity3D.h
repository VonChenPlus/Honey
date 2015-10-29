#ifndef UNITY3D_H
#define UNITY3D_H

#include <vector>
#include <string>
#include "BASE/HObject.h"
#include "MATH/Matrix.h"

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
        DYNAMIC = 16,
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
        UNKNOWN,
        LINEAR1D,
        LINEAR2D,
        LINEAR3D,
        CUBE,
        ARRAY1D,
        ARRAY2D,
    };

    enum U3DImageFormat : uint8
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

    enum U3DImageType
    {
        PNG,
        JPEG,
        ZIM,
        DETECT,
        TYPE_UNKNOWN,
    };

    enum U3DInfo
    {
        APINAME,
        APIVERSION,
        VENDOR,
        SHADELANGVERSION,
        RENDERER,
    };

    // Binary compatible with D3D11 viewport.
    struct U3DViewport
    {
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
    };

    class Unity3DDepthStencilState : public Unity3DObject
    {
    public:
    };

    class Unity3DBuffer : public Unity3DObject
    {
    public:
        virtual void setData(const uint8 *data, size_t size) = 0;
        virtual void subData(const uint8 *data, size_t offset, size_t size) = 0;
    };

    class Unity3DTexture : public Unity3DObject
    {
    public:
        void loadFromFile(const std::string &filename, U3DImageType type = U3DImageType::DETECT);
        void loadFromFileData(const uint8 *data, size_t dataSize, U3DImageType type = U3DImageType::DETECT);

        virtual void create(U3DTextureType type, U3DImageFormat format, int width, int height, int depth, int mipLevels) = 0;
        virtual void setImageData(int x, int y, int z, int width, int height, int depth, int level, int stride, const uint8 *data) = 0;
        virtual void autoGenMipmaps() = 0;
        virtual void finalize(int zim_flags) = 0;  // TODO: Tidy up

        int width() { return width_; }
        int height() { return height_; }
        int depth() { return depth_; }
    protected:
        std::string filename_;  // Textures that are loaded from files can reload themselves automatically.
        int width_, height_, depth_;
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

    struct U3DBlendStateDesc
    {
        bool enabled;
        U3DBlendEquation eqCol;
        U3DBlendFactor srcCol;
        U3DBlendFactor dstCol;
        U3DBlendEquation eqAlpha;
        U3DBlendFactor srcAlpha;
        U3DBlendFactor dstAlpha;
        bool logicEnabled;
        U3DLogicOp logicOp;
        // int colorMask;
    };
}

#endif // THIN3D_H
