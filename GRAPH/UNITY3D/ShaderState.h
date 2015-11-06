#ifndef SHADERSTATE_H
#define SHADERSTATE_H

#include "GRAPH/UNITY3D/GLCommon.h"
#include "GRAPH/UNITY3D/Unity3D.h"

namespace GRAPH
{
    struct U3DUniform;
    struct U3DVertexAttrib;
    class Unity3DShaderSet;

    class UniformValue
    {
        friend class Unity3DShaderSet;
        friend class ShaderState;

    public:
        UniformValue();
        UniformValue(U3DUniform *uniform, Unity3DShaderSet* u3dShader);
        ~UniformValue();

        void setFloat(float value);
        void setInt(int value);
        void setFloatv(uint64 size, const float* pointer);
        void setVec2(const MATH::Vector2f& value);
        void setVec2v(uint64 size, const MATH::Vector2f* pointer);
        void setVec3(const MATH::Vector3f& value);
        void setVec3v(uint64 size, const MATH::Vector3f* pointer);
        void setVec4(const MATH::Vector4f& value);
        void setVec4v(uint64 size, const MATH::Vector4f* pointer);
        void setMat4(const MATH::Matrix4& value);

        void setTexture(GLuint textureId, GLuint textureUnit);

        void apply();

    protected:
        enum class Type {
            VALUE,
            POINTER,
        };

        U3DUniform* uniform_;
        Unity3DShaderSet* u3dShader_;
        Type type_;

        union U{
            float floatValue;
            int intValue;
            float v2Value[2];
            float v3Value[3];
            float v4Value[4];
            float matrixValue[16];
            struct {
                GLuint textureId;
                GLuint textureUnit;
            } tex;
            struct {
                const float* pointer;
                GLsizei size;
            } floatv;
            struct {
                const float* pointer;
                GLsizei size;
            } v2f;
            struct {
                const float* pointer;
                GLsizei size;
            } v3f;
            struct {
                const float* pointer;
                GLsizei size;
            } v4f;
            std::function<void(Unity3DShaderSet*, U3DUniform*)> *callback;

            U() { memset( this, 0, sizeof(*this) ); }
            ~U(){}
            U& operator=( const U& other ) {
                memcpy(this, &other, sizeof(*this));
                return *this;
            }
        } value_;
    };

    class VertexAttribValue
    {
        friend class ShaderState;

    public:
        VertexAttribValue(U3DVertexAttrib *vertexAttrib);
        VertexAttribValue();
        ~VertexAttribValue();

        void setPointer(uint8 size, U3DVertexDataType type, bool normalized, int stride, intptr pointer);
        void apply();

    protected:
        U3DVertexAttrib *vertexAttrib_;
        Unity3DVertexFormat *vertexFormat_;
        bool enabled_;
    };

    class ShaderState : public HObject
    {
        friend class ShaderStateCache;
    public:
        static ShaderState* create(Unity3DShaderSet* u3dShader);
        static ShaderState* getOrCreateWithShader(Unity3DShaderSet* u3dShader);
        static ShaderState* getOrCreateWithShaderName(const std::string& shaderName );
        static ShaderState* getOrCreateWithShaders(const std::string& vertexShader, const std::string& fragShader, const std::string& compileTimeDefines);

        void apply(const MATH::Matrix4& modelView);
        void applyU3DShader(const MATH::Matrix4& modelView);
        void applyAttributes();
        void applyUniforms();

        void setU3DShader(Unity3DShaderSet* u3dShader);
        Unity3DShaderSet* getU3DShader() const { return u3dShader_; }
        uint64 getVertexAttribCount() const { return attributes_.size(); }
        uint64 getUniformCount() const { return uniforms_.size(); }

        void setVertexAttribPointer(const std::string& name, uint8 size, U3DVertexDataType type, bool normalized, int stride, intptr pointer);

        void setUniformInt(const std::string& uniformName, int value);
        void setUniformFloat(const std::string& uniformName, float value);
        void setUniformFloatv(const std::string& uniformName, uint64 size, const float* pointer);
        void setUniformVec2(const std::string& uniformName, const MATH::Vector2f& value);
        void setUniformVec2v(const std::string& uniformName, uint64 size, const MATH::Vector2f* pointer);
        void setUniformVec3(const std::string& uniformName, const MATH::Vector3f& value);
        void setUniformVec3v(const std::string& uniformName, uint64 size, const MATH::Vector3f* pointer);
        void setUniformVec4(const std::string& uniformName, const MATH::Vector4f& value);
        void setUniformVec4v(const std::string& uniformName, uint64 size, const MATH::Vector4f* pointer);
        void setUniformMat4(const std::string& uniformName, const MATH::Matrix4& value);
        void setUniformTexture(const std::string& uniformName, GLuint textureId);

        void setUniformInt(int32 uniformLocation, int value);
        void setUniformFloat(int32 uniformLocation, float value);
        void setUniformFloatv(int32 uniformLocation, uint64 size, const float* pointer);
        void setUniformVec2(int32 uniformLocation, const MATH::Vector2f& value);
        void setUniformVec2v(int32 uniformLocation, uint64 size, const MATH::Vector2f* pointer);
        void setUniformVec3(int32 uniformLocation, const MATH::Vector3f& value);
        void setUniformVec3v(int32 uniformLocation, uint64 size, const MATH::Vector3f* pointer);
        void setUniformVec4(int32 uniformLocation, const MATH::Vector4f& value);
        void setUniformVec4v(int32 uniformLocation, uint64 size, const MATH::Vector4f* pointer);
        void setUniformMat4(int32 uniformLocation, const MATH::Matrix4& value);
        void setUniformTexture(int32 uniformLocation, GLuint textureId);

    protected:
        ShaderState();
        ~ShaderState();
        bool init(Unity3DShaderSet* program);
        void resetU3DShader();
        void updateUniformsAndAttributes();
        VertexAttribValue* getVertexAttribValue(const std::string& attributeName);
        UniformValue* getUniformValue(const std::string& uniformName);
        UniformValue* getUniformValue(int32 uniformLocation);

    private:
        bool uniformAttributeValueDirty_;
        std::unordered_map<std::string, int32> uniformsByName_;
        std::unordered_map<int32, UniformValue> uniforms_;
        std::unordered_map<std::string, VertexAttribValue> attributes_;
        std::unordered_map<std::string, int> boundTextureUnits_;
        int textureUnitIndex_;
        Unity3DShaderSet* u3dShader_;
    };

    class ShaderStateCache
    {
    public:
        static ShaderStateCache& getInstance();

        ShaderState* getShaderState(Unity3DShaderSet* program);
        void removeAllShaderState();
        void removeUnusedShaderState();

    protected:
        ShaderStateCache();
        ~ShaderStateCache();

    private:
        HObjectMap<Unity3DShaderSet*, ShaderState*> shaderStates_;
    };
}

#endif // SHADERSTATE_H
