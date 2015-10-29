#ifndef GLSHADERSTATE_H
#define GLSHADERSTATE_H

#include <string>
#include <functional>
#include <unordered_map>
#include "BASE/HObject.h"
#include "MATH/Matrix.h"
#include "GRAPH/UNITY3D/GLCommon.h"

namespace GRAPH
{
    struct Uniform;
    struct VertexAttrib;
    class Unity3DGLShaderSet;

    class UniformValue
    {
        friend class Unity3DGLShaderSet;
        friend class GLShaderState;

    public:
        UniformValue();
        UniformValue(Uniform *uniform, Unity3DGLShaderSet* u3dShader);
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

        void setCallback(const std::function<void(Unity3DGLShaderSet*, Uniform*)> &callback);
        void setTexture(GLuint textureId, GLuint textureUnit);

        void apply();

    protected:
        enum class Type {
            VALUE,
            POINTER,
            CALLBACK_FN     // CALLBACK is already defined in windows, can't use it.
        };

        Uniform* uniform_;
        Unity3DGLShaderSet* u3dShader_;
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
            std::function<void(Unity3DGLShaderSet*, Uniform*)> *callback;

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
        friend class GLShaderState;

    public:
        VertexAttribValue(VertexAttrib *vertexAttrib);
        VertexAttribValue();
        ~VertexAttribValue();

        void setPointer(GLint size, GLenum type, GLboolean normalized, GLsizei stride, GLvoid *pointer);
        void setCallback(const std::function<void(VertexAttrib*)> &callback);
        void apply();

    protected:
        VertexAttrib* vertexAttrib_;
        bool useCallback_;
        bool enabled_;

        union U{
            struct {
                GLint size;
                GLenum type;
                GLboolean normalized;
                GLsizei stride;
                GLvoid *pointer;
            } pointer;
            std::function<void(VertexAttrib*)> *callback;

            U() { memset( this, 0, sizeof(*this) ); }
            ~U(){}
            U& operator=( const U& other ) {
                memcpy(this, &other, sizeof(*this));
                return *this;
            }
        } value_;
    };

    class GLShaderState : public HObject
    {
        friend class GLShaderStateCache;
    public:
        static GLShaderState* create(Unity3DGLShaderSet* u3dShader);
        static GLShaderState* getOrCreateWithGLShader(Unity3DGLShaderSet* u3dShader);
        static GLShaderState* getOrCreateWithGLShaderName(const std::string& glShaderName );
        static GLShaderState* getOrCreateWithShaders(const std::string& vertexShader, const std::string& fragShader, const std::string& compileTimeDefines);

        void apply(const MATH::Matrix4& modelView);
        void applyU3DShader(const MATH::Matrix4& modelView);
        void applyAttributes(bool applyAttribFlags = true);
        void applyUniforms();

        void setGLShader(Unity3DGLShaderSet* u3dShader);
        Unity3DGLShaderSet* getU3DShader() const { return u3dShader_; }
        uint32_t getVertexAttribsFlags() const;
        uint64 getVertexAttribCount() const;
        void setVertexAttribCallback(const std::string& name, const std::function<void(VertexAttrib*)> &callback);
        void setVertexAttribPointer(const std::string& name, GLint size, GLenum type, GLboolean normalized, GLsizei stride, GLvoid *pointer);
        uint64 getUniformCount() const { return uniforms_.size(); }

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
        void setUniformCallback(const std::string& uniformName, const std::function<void(Unity3DGLShaderSet*, Uniform*)> &callback);
        void setUniformTexture(const std::string& uniformName, GLuint textureId);

        void setUniformInt(GLint uniformLocation, int value);
        void setUniformFloat(GLint uniformLocation, float value);
        void setUniformFloatv(GLint uniformLocation, uint64 size, const float* pointer);
        void setUniformVec2(GLint uniformLocation, const MATH::Vector2f& value);
        void setUniformVec2v(GLint uniformLocation, uint64 size, const MATH::Vector2f* pointer);
        void setUniformVec3(GLint uniformLocation, const MATH::Vector3f& value);
        void setUniformVec3v(GLint uniformLocation, uint64 size, const MATH::Vector3f* pointer);
        void setUniformVec4(GLint uniformLocation, const MATH::Vector4f& value);
        void setUniformVec4v(GLint uniformLocation, uint64 size, const MATH::Vector4f* pointer);
        void setUniformMat4(GLint uniformLocation, const MATH::Matrix4& value);
        void setUniformCallback(GLint uniformLocation, const std::function<void(Unity3DGLShaderSet*, Uniform*)> &callback);
        void setUniformTexture(GLint uniformLocation, GLuint textureId);

    protected:
        GLShaderState();
        ~GLShaderState();
        bool init(Unity3DGLShaderSet* program);
        void resetGLShader();
        void updateUniformsAndAttributes();
        VertexAttribValue* getVertexAttribValue(const std::string& attributeName);
        UniformValue* getUniformValue(const std::string& uniformName);
        UniformValue* getUniformValue(GLint uniformLocation);

    private:
        bool uniformAttributeValueDirty_;
        std::unordered_map<std::string, GLint> uniformsByName_;
        std::unordered_map<GLint, UniformValue> uniforms_;
        std::unordered_map<std::string, VertexAttribValue> attributes_;
        std::unordered_map<std::string, int> boundTextureUnits_;
        int textureUnitIndex_;
        uint32_t vertexAttribsFlags_;
        Unity3DGLShaderSet* u3dShader_;
    };

    class GLShaderStateCache
    {
    public:
        static GLShaderStateCache& getInstance();

        GLShaderState* getGLShaderState(Unity3DGLShaderSet* program);
        void removeAllGLShaderState();
        void removeUnusedGLShaderState();

    protected:
        GLShaderStateCache();
        ~GLShaderStateCache();

    private:
        HObjectMap<Unity3DGLShaderSet*, GLShaderState*> glShaderStates_;
    };
}

#endif // GLSHADERSTATE_H
