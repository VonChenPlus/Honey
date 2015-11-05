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
    struct VertexAttrib
    {
        GLuint index;
        GLint size;
        GLenum type;
        std::string name;
    };

    struct Uniform
    {
        GLint location;
        GLint size;
        GLenum type;
        std::string name;
    };

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

    class Unity3DGLShaderSet : public Unity3DShaderSet
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

        void link();
        void apply();
        void unApply();

        inline const GLuint getProgram() const { return program_; }

        Uniform* getUniform(const std::string &name);
        VertexAttrib* getVertexAttrib(const std::string &name);

        GLint getAttribLocation(const std::string &attributeName) const;
        GLint getUniformLocation(const std::string &attributeName) const;
        void bindAttribLocation(const std::string &attributeName, GLuint index) const;

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

        void setUniformLocationWith1i(GLint location, GLint i1);
        void setUniformLocationWith2i(GLint location, GLint i1, GLint i2);
        void setUniformLocationWith3i(GLint location, GLint i1, GLint i2, GLint i3);
        void setUniformLocationWith4i(GLint location, GLint i1, GLint i2, GLint i3, GLint i4);
        void setUniformLocationWith2iv(GLint location, GLint* ints, unsigned int numberOfArrays);
        void setUniformLocationWith3iv(GLint location, GLint* ints, unsigned int numberOfArrays);
        void setUniformLocationWith4iv(GLint location, GLint* ints, unsigned int numberOfArrays);
        void setUniformLocationWith1f(GLint location, GLfloat f1);
        void setUniformLocationWith2f(GLint location, GLfloat f1, GLfloat f2);
        void setUniformLocationWith3f(GLint location, GLfloat f1, GLfloat f2, GLfloat f3);
        void setUniformLocationWith4f(GLint location, GLfloat f1, GLfloat f2, GLfloat f3, GLfloat f4);
        void setUniformLocationWith1fv(GLint location, const GLfloat* floats, unsigned int numberOfArrays);
        void setUniformLocationWith2fv(GLint location, const GLfloat* floats, unsigned int numberOfArrays);
        void setUniformLocationWith3fv(GLint location, const GLfloat* floats, unsigned int numberOfArrays);
        void setUniformLocationWith4fv(GLint location, const GLfloat* floats, unsigned int numberOfArrays);
        void setUniformLocationWithMatrix2fv(GLint location, const GLfloat* matrixArray, unsigned int numberOfMatrices);
        void setUniformLocationWithMatrix3fv(GLint location, const GLfloat* matrixArray, unsigned int numberOfMatrices);
        void setUniformLocationWithMatrix4fv(GLint location, const GLfloat* matrixArray, unsigned int numberOfMatrices);

        void setUniformsForBuiltins();
        void setUniformsForBuiltins(const MATH::Matrix4 &modelView);

        void reset();

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

        std::unordered_map<std::string, Uniform> userUniforms_;
        std::unordered_map<std::string, VertexAttrib> vertexAttribs_;
        std::unordered_map<GLint, std::pair<GLvoid*, unsigned int> > hashForUniforms_;

        friend class GLShaderState;
    };

    class Unity3DGLShaderCache : public HObject
    {
    public:
        Unity3DGLShaderCache();
        ~Unity3DGLShaderCache();

        static Unity3DGLShaderCache& getInstance();

        void loadDefaultGLShaders();
        void reloadDefaultGLShaders();

        Unity3DGLShaderSet *getU3DShader(const std::string &key);

        void addU3DShader(Unity3DGLShaderSet* program, const std::string &key);

    private:
        bool init();
        void loadDefaultGLShader(Unity3DGLShaderSet *program, int type);

    private:
        std::unordered_map<std::string, Unity3DGLShaderSet*> programs_;
    };
}

#endif // UNITY3DGLSHADER_H
