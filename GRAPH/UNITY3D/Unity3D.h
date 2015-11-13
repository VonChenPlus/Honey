#ifndef UNITY3D_H
#define UNITY3D_H

#include <string>
#include <string.h>
#include <vector>
#include <functional>
#include <unordered_map>
#include "BASE/HObject.h"
#include "BASE/HData.h"
#include "IMAGE/ImageDefine.h"
#include "IMAGE/ImageObject.h"
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
        DYNAMIC = 8,
        STREAM = 16,
    };

    enum U3DVertexDataType : uint32
    {
        INVALID,
        FLOATx2,
        FLOATx3,
        FLOATx4,
        UNORM8x4,
        CUSTOM,
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
        PRIM_TRIANGLESGL_STRIP,
        PRIM_TRIANGLE_FAN,
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

    struct U3DVertexAttrib
    {
        uint8 semantic;
        int32 size;
        uint32 type;
        std::string name;
    };

    struct U3DVertexComponent
    {
        U3DVertexComponent() { memset(this, 0, sizeof(*this)); }
        U3DVertexComponent(U3DSemantic semantic, U3DVertexDataType dataType, int stride = 0, intptr offset = 0) {
            memset(this, 0, sizeof(*this));
            this->type = dataType;
            this->semantic = semantic;
            this->stride = stride;
            this->offset = offset;
        }
        uint8 semantic;
        U3DVertexDataType type;
        uint8 size;
        intptr offset;
        int stride;
        bool normalized;
    };

    class Unity3DVertexFormat : public Unity3DObject
    {
    public:
        virtual void apply(const void *base = nullptr) = 0;
        virtual void unApply() = 0;

        std::vector<U3DVertexComponent> &components() { return components_; }

    protected:
        std::vector<U3DVertexComponent> components_;
    };

    struct U3DUniform
    {
        int32 location;
        int32 size;
        uint32 type;
        std::string name;
    };

    struct U3DuniformComponent
    {
        U3DuniformComponent() { memset(this, 0, sizeof(*this)); }
        U3DuniformComponent(int32 location, uint32 type) {
            memset(this, 0, sizeof(*this));
            this->location = location;
            this->type = type;
        }
        ~U3DuniformComponent(){}
        U3DuniformComponent& operator=(const U3DuniformComponent& other) {
            memcpy(this, &other, sizeof(*this));
            return *this;
        }

        int32 location;
        uint32 type;

        union U
        {
            float floatValue;
            int intValue;
            float v2Value[2];
            float v3Value[3];
            float v4Value[4];
            float matrixValue[16];
            struct {
                uint32 textureId;
                uint32 textureUnit;
            } tex;
            struct {
                const float* pointer;
                int32 size;
            } floatv;
            struct {
                const float* pointer;
                int32 size;
            } v2f;
            struct {
                const float* pointer;
                int32 size;
            } v3f;
            struct {
                const float* pointer;
                int32 size;
            } v4f;
        } value;
    };

    class Unity3DUniformFormat : public Unity3DObject
    {
    public:
        virtual void applyArray() = 0;
        virtual void applyValue() = 0;

        U3DuniformComponent &component() { return component_; }

    protected:
        U3DuniformComponent component_;
    };

    class Unity3DShader : public Unity3DObject
    {
    public:
        /**Built in shader for 2d. Support Position, Texture and Color vertex attribute.*/
        static const char* SHADER_NAME_POSITION_TEXTURE_COLOR;
        /**Built in shader for 2d. Support Position, Texture and Color vertex attribute, but without multiply vertex by MVP matrix.*/
        static const char* SHADER_NAME_POSITION_TEXTURE_COLOR_NO_MVP;
        /**Built in shader for 2d. Support Position, Texture vertex attribute, but include alpha test.*/
        static const char* SHADER_NAME_POSITION_TEXTURE_ALPHA_TEST;
        /**Built in shader for 2d. Support Position, Texture and Color vertex attribute, include alpha test and without multiply vertex by MVP matrix.*/
        static const char* SHADER_NAME_POSITION_TEXTURE_ALPHA_TEST_NO_MV;
        /**Built in shader for 2d. Support Position, Color vertex attribute.*/
        static const char* SHADER_NAME_POSITION_COLOR;
        /**Built in shader for 2d. Support Position, Color, Texture vertex attribute. texture coordinate will used as point size.*/
        static const char* SHADER_NAME_POSITION_COLOR_TEXASPOINTSIZE;
        /**Built in shader for 2d. Support Position, Color vertex attribute, without multiply vertex by MVP matrix.*/
        static const char* SHADER_NAME_POSITION_COLOR_NO_MVP;
        /**Built in shader for 2d. Support Position, Texture vertex attribute.*/
        static const char* SHADER_NAME_POSITION_TEXTURE;
        /**Built in shader for 2d. Support Position, Texture vertex attribute. with a specified uniform as color*/
        static const char* SHADER_NAME_POSITION_TEXTURE_U_COLOR;
        /**Built in shader for 2d. Support Position, Texture and Color vertex attribute. but alpha will be the multiplication of color attribute and texture.*/
        static const char* SHADER_NAME_POSITION_TEXTURE_A8_COLOR;
        /**Built in shader for 2d. Support Position, with color specified by a uniform.*/
        static const char* SHADER_NAME_POSITION_U_COLOR;
        /**Built in shader for draw a sector with 90 degrees with center at bottom left point.*/
        static const char* SHADER_NAME_POSITION_LENGTH_TEXTURE_COLOR;
        /**Built in shader for ui effects */
        static const char* SHADER_NAME_POSITION_GRAYSCALE;
        static const char* SHADER_NAME_LABEL_NORMAL;
        static const char* SHADER_NAME_LABEL_OUTLINE;
        static const char* SHADER_NAME_LABEL_DISTANCEFIELD_NORMAL;
        static const char* SHADER_NAME_LABEL_DISTANCEFIELD_GLOW;
    };

    class Unity3DShaderSet : public Unity3DObject
    {
    public:
        virtual int name() = 0;
        virtual void link() = 0;
        virtual void apply() = 0;
        virtual void unApply() = 0;
        virtual void reset() {}

        virtual int32 getAttribLocation(const std::string &attributeName) const = 0;
        virtual int32 getUniformLocation(const std::string &attributeName) const = 0;
        virtual void bindAttribLocation(const std::string &attributeName, uint32 index) const = 0;

        virtual void setUniformLocationWith1i(int location, int i1) = 0;
        virtual void setUniformLocationWith2i(int location, int i1, int i2) = 0;
        virtual void setUniformLocationWith3i(int location, int i1, int i2, int i3) = 0;
        virtual void setUniformLocationWith4i(int location, int i1, int i2, int i3, int i4) = 0;
        virtual void setUniformLocationWith2iv(int location, int* ints, unsigned int numberOfArrays) = 0;
        virtual void setUniformLocationWith3iv(int location, int* ints, unsigned int numberOfArrays) = 0;
        virtual void setUniformLocationWith4iv(int location, int* ints, unsigned int numberOfArrays) = 0;
        virtual void setUniformLocationWith1f(int location, float f1) = 0;
        virtual void setUniformLocationWith2f(int location, float f1, float f2) = 0;
        virtual void setUniformLocationWith3f(int location, float f1, float f2, float f3) = 0;
        virtual void setUniformLocationWith4f(int location, float f1, float f2, float f3, float f4) = 0;
        virtual void setUniformLocationWith1fv(int location, const float* floats, unsigned int numberOfArrays) = 0;
        virtual void setUniformLocationWith2fv(int location, const float* floats, unsigned int numberOfArrays) = 0;
        virtual void setUniformLocationWith3fv(int location, const float* floats, unsigned int numberOfArrays) = 0;
        virtual void setUniformLocationWith4fv(int location, const float* floats, unsigned int numberOfArrays) = 0;
        virtual void setUniformLocationWithMatrix2fv(int location, const float* matrixArray, unsigned int numberOfMatrices) = 0;
        virtual void setUniformLocationWithMatrix3fv(int location, const float* matrixArray, unsigned int numberOfMatrices) = 0;
        virtual void setUniformLocationWithMatrix4fv(int location, const float* matrixArray, unsigned int numberOfMatrices) = 0;

        virtual void setUniformsForBuiltins() = 0;
        virtual void setUniformsForBuiltins(const MATH::Matrix4 &modelView) = 0;

        U3DUniform* userUniform(const std::string &name) {
            const auto itr = userUniforms_.find(name);
            if (itr != userUniforms_.end())
                return &itr->second;
            return nullptr;
        }

        U3DVertexAttrib* vertexAttrib(const std::string &name) {
            const auto itr = vertexAttribs_.find(name);
            if (itr != vertexAttribs_.end())
                return &itr->second;
            return nullptr;
        }

        std::unordered_map<std::string, U3DUniform> &userUniforms() { return userUniforms_;  }
        std::unordered_map<std::string, U3DVertexAttrib> &vertexAttribs() { return vertexAttribs_; }

    protected:
        std::unordered_map<std::string, U3DUniform> userUniforms_;
        std::unordered_map<std::string, U3DVertexAttrib> vertexAttribs_;
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
        static Unity3DShaderSet *CreateShaderSetWithByteArray(const std::string &vShaderByteArray, const std::string &fShaderByteArray, const std::string& compileTimeDefines = std::string());
        static Unity3DShaderSet *CreateShaderSetWithFileName(const std::string &vShaderFilename, const std::string &fShaderFilename, const std::string& compileTimeDefines = std::string());
        static Unity3DVertexFormat *CreateVertexFormat(const U3DVertexComponent &component);
        static Unity3DVertexFormat *CreateVertexFormat(const std::vector<U3DVertexComponent> &components);
        static Unity3DUniformFormat *CreateUniformFormat(Unity3DShaderSet * u3dShader, const U3DuniformComponent &component);
        static Unity3DTexture *CreateTexture(U3DTextureType type = LINEAR2D, bool antialias = true);
    };
}

#endif // UNITY3D_H
