#ifndef UNITY3DGLSHADER_H
#define UNITY3DGLSHADER_H

#include <unordered_map>
#include <map>
#include <string>
#include "GRAPH/UNITY3D/GLCommon.h"
#include "GRAPH/UNITY3D/Unity3D.h"
#include "MATH/Matrix.h"

namespace GRAPH
{
    class Unity3DGLShader : public Unity3DShader
    {
    public:
        Unity3DGLShader(bool isFragmentShader);
        ~Unity3DGLShader();

        void compile(const char *source, const char *compileTimeDefines);

        GLuint getShader() const { return shader_; }

    private:
        GLuint shader_;
        GLuint type_;
    };

    class Unity3DGLShaderSet final : public Unity3DShaderSet
    {
    public:
        Unity3DGLShaderSet();
        Unity3DGLShaderSet(Unity3DShader *vertShader, Unity3DShader *fragShader);
        ~Unity3DGLShaderSet();

        static Unity3DGLShaderSet* createWithByteArrays(const GLchar* vShaderByteArray, const GLchar* fShaderByteArray);
        bool initWithByteArrays(const GLchar* vShaderByteArray, const GLchar* fShaderByteArray);
        static Unity3DGLShaderSet* createWithByteArrays(const GLchar* vShaderByteArray, const GLchar* fShaderByteArray, const std::string& compileTimeDefines);
        bool initWithByteArrays(const GLchar* vShaderByteArray, const GLchar* fShaderByteArray, const std::string& compileTimeDefines);

        static Unity3DGLShaderSet* createWithFilenames(const std::string& vShaderFilename, const std::string& fShaderFilename);
        bool initWithFilenames(const std::string& vShaderFilename, const std::string& fShaderFilename);

        static Unity3DGLShaderSet* createWithFilenames(const std::string& vShaderFilename, const std::string& fShaderFilename, const std::string& compileTimeDefines);
        bool initWithFilenames(const std::string& vShaderFilename, const std::string& fShaderFilename, const std::string& compileTimeDefines);

        int name() override;
        void link() override;
        void apply() override;
        void unApply() override;
        void reset() override;

        inline const GLuint getProgram() const { return program_; }

        int32 getAttribLocation(const std::string &attributeName) const override;
        int32 getUniformLocation(const std::string &attributeName) const override;
        void bindAttribLocation(const std::string &attributeName, uint32 index) const override;

        enum
        {
            UNIFORM_AMBIENT_COLOR,
            UNIFORM_P_MATRIX,
            UNIFORM_MV_MATRIX,
            UNIFORM_MVP_MATRIX,
            UNIFORM_NORMAL_MATRIX,
            UNIFORM_TIME,
            UNIFORM_SIN_TIME,
            UNIFORM_COS_TIME,
            UNIFORM_RANDOM01,
            UNIFORM_SAMPLER0,
            UNIFORM_SAMPLER1,
            UNIFORM_SAMPLER2,
            UNIFORM_SAMPLER3,
            UNIFORM_MAX,
        };

        /**Ambient Color uniform.*/
        static const char* UNIFORM_NAME_AMBIENT_COLOR;
        /**Projection Matrix uniform.*/
        static const char* UNIFORM_NAME_P_MATRIX;
        /**Model view matrix uniform.*/
        static const char* UNIFORM_NAME_MV_MATRIX;
        /**Model view projection uniform.*/
        static const char* UNIFORM_NAME_MVP_MATRIX;
        /**Normal matrix uniform.*/
        static const char* UNIFORM_NAME_NORMAL_MATRIX;
        /**Time uniform.*/
        static const char* UNIFORM_NAME_TIME;
        /**Sin time uniform.*/
        static const char* UNIFORM_NAME_SIN_TIME;
        /**Cos time uniform.*/
        static const char* UNIFORM_NAME_COS_TIME;
        /**Random number uniform.*/
        static const char* UNIFORM_NAME_RANDOM01;

        static const char* UNIFORM_NAME_SAMPLER0;
        static const char* UNIFORM_NAME_SAMPLER1;
        static const char* UNIFORM_NAME_SAMPLER2;
        static const char* UNIFORM_NAME_SAMPLER3;

        /**Alpha test value uniform.*/
        static const char* UNIFORM_NAME_ALPHA_TEST_VALUE;

        /**Attribute color.*/
        static const char* ATTRIBUTE_NAME_COLOR;
        /**Attribute position.*/
        static const char* ATTRIBUTE_NAME_POSITION;
        /**@{ Attribute Texcoord 0-3.*/
        static const char* ATTRIBUTE_NAME_TEX_COORD;
        static const char* ATTRIBUTE_NAME_TEX_COORD1;
        static const char* ATTRIBUTE_NAME_TEX_COORD2;
        static const char* ATTRIBUTE_NAME_TEX_COORD3;
        /**@}*/
        /**Attribute normal.*/
        static const char* ATTRIBUTE_NAME_NORMAL;
        /**Attribute blend weight.*/
        static const char* ATTRIBUTE_NAME_BLEND_WEIGHT;
        /**Attribute blend index.*/
        static const char* ATTRIBUTE_NAME_BLEND_INDEX;

        void updateUniforms();

        void setUniformLocationWith1i(int location, int i1) override;
        void setUniformLocationWith2i(int location, int i1, int i2) override;
        void setUniformLocationWith3i(int location, int i1, int i2, int i3) override;
        void setUniformLocationWith4i(int location, int i1, int i2, int i3, int i4) override;
        void setUniformLocationWith2iv(int location, int* ints, unsigned int numberOfArrays) override;
        void setUniformLocationWith3iv(int location, int* ints, unsigned int numberOfArrays) override;
        void setUniformLocationWith4iv(int location, int* ints, unsigned int numberOfArrays) override;
        void setUniformLocationWith1f(int location, float f1) override;
        void setUniformLocationWith2f(int location, float f1, float f2) override;
        void setUniformLocationWith3f(int location, float f1, float f2, float f3) override;
        void setUniformLocationWith4f(int location, float f1, float f2, float f3, float f4) override;
        void setUniformLocationWith1fv(int location, const float* floats, unsigned int numberOfArrays) override;
        void setUniformLocationWith2fv(int location, const float* floats, unsigned int numberOfArrays) override;
        void setUniformLocationWith3fv(int location, const float* floats, unsigned int numberOfArrays) override;
        void setUniformLocationWith4fv(int location, const float* floats, unsigned int numberOfArrays) override;
        void setUniformLocationWithMatrix2fv(int location, const float* matrixArray, unsigned int numberOfMatrices) override;
        void setUniformLocationWithMatrix3fv(int location, const float* matrixArray, unsigned int numberOfMatrices) override;
        void setUniformLocationWithMatrix4fv(int location, const float* matrixArray, unsigned int numberOfMatrices) override;

        void setUniformsForBuiltins() override;
        void setUniformsForBuiltins(const MATH::Matrix4 &modelView) override;

    private:
        void bindPredefinedVertexAttribs();
        void parseVertexAttribs();
        void parseUniforms();

        bool updateUniformLocation(GLint location, const GLvoid* data, unsigned int bytes);

    private:
        GLuint program_;
        Unity3DGLShader *vertShader_;
        Unity3DGLShader *fragShader_;

        GLint builtInUniforms_[UNIFORM_MAX];

        struct BulidInUniformsFlags {
            unsigned int usesTime : 1;
            unsigned int usesNormal : 1;
            unsigned int usesMVP : 1;
            unsigned int usesMV : 1;
            unsigned int usesP : 1;
            unsigned int usesRandom : 1;
            BulidInUniformsFlags() { memset(this, 0, sizeof(*this)); }
        } uniformsFlags_;

        std::unordered_map<GLint, std::pair<GLvoid*, unsigned int> > hashForUniforms_;
    };
}

#endif // UNITY3DGLSHADER_H
