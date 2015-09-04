#ifndef GLPROGRAM_H
#define GLPROGRAM_H

#include <unordered_map>
#include <string>
#include "BASE/HObject.h"
#include "GRAPH/Types.h"
#include "GRAPH/RENDERER/GLCommon.h"
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

    class GLProgram : public HObject
    {
    public:
        enum
        {
            VERTEX_ATTRIB_POSITION,
            VERTEX_ATTRIB_COLOR,
            VERTEX_ATTRIB_TEX_COORD,
            VERTEX_ATTRIB_TEX_COORD1,
            VERTEX_ATTRIB_TEX_COORD2,
            VERTEX_ATTRIB_TEX_COORD3,
            VERTEX_ATTRIB_NORMAL,
            VERTEX_ATTRIB_BLEND_WEIGHT,
            VERTEX_ATTRIB_BLEND_INDEX,
            VERTEX_ATTRIB_MAX,

            VERTEX_ATTRIB_TEX_COORDS = VERTEX_ATTRIB_TEX_COORD,
        };

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

        GLProgram();
        virtual ~GLProgram();

        static GLProgram* createWithByteArrays(const GLchar* vShaderByteArray, const GLchar* fShaderByteArray);
        bool initWithByteArrays(const GLchar* vShaderByteArray, const GLchar* fShaderByteArray);
        static GLProgram* createWithByteArrays(const GLchar* vShaderByteArray, const GLchar* fShaderByteArray, const std::string& compileTimeDefines);
        bool initWithByteArrays(const GLchar* vShaderByteArray, const GLchar* fShaderByteArray, const std::string& compileTimeDefines);

        static GLProgram* createWithFilenames(const std::string& vShaderFilename, const std::string& fShaderFilename);
        bool initWithFilenames(const std::string& vShaderFilename, const std::string& fShaderFilename);

        static GLProgram* createWithFilenames(const std::string& vShaderFilename, const std::string& fShaderFilename, const std::string& compileTimeDefines);
        bool initWithFilenames(const std::string& vShaderFilename, const std::string& fShaderFilename, const std::string& compileTimeDefines);

        Uniform* getUniform(const std::string& name);
        VertexAttrib* getVertexAttrib(const std::string& name);

        void bindAttribLocation(const std::string& attributeName, GLuint index) const;
        GLint getAttribLocation(const std::string& attributeName) const;
        GLint getUniformLocation(const std::string& attributeName) const;

        bool link();
        void use();

        void updateUniforms();
        GLint getUniformLocationForName(const char* name) const;
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
        inline const GLuint getProgram() const { return program_; }

    protected:
        bool updateUniformLocation(GLint location, const GLvoid* data, unsigned int bytes);

        void bindPredefinedVertexAttribs();
        void parseVertexAttribs();
        void parseUniforms();

        bool compileShader(GLuint * shader, GLenum type, const GLchar* source, const std::string& convertedDefines);
        bool compileShader(GLuint * shader, GLenum type, const GLchar* source);

        GLuint            program_;
        GLuint            vertShader_;
        GLuint            fragShader_;
        GLint             builtInUniforms_[UNIFORM_MAX];
        bool              hasShaderCompiler_;

        struct BulidInUniformsFlags {
            unsigned int usesTime:1;
            unsigned int usesNormal:1;
            unsigned int usesMVP:1;
            unsigned int usesMV:1;
            unsigned int usesP:1;
            unsigned int usesRandom:1;
            BulidInUniformsFlags() { memset(this, 0, sizeof(*this)); }
        } uniformsFlags_;

        std::unordered_map<std::string, Uniform> userUniforms_;
        std::unordered_map<std::string, VertexAttrib> vertexAttribs_;
        std::unordered_map<GLint, std::pair<GLvoid*, unsigned int> > hashForUniforms_;
    };

    class GLProgramCache : public HObject
    {
    public:
        GLProgramCache();
        ~GLProgramCache();

        static GLProgramCache* getInstance();
        static void destroyInstance();

        void loadDefaultGLPrograms();
        void reloadDefaultGLPrograms();

        GLProgram * getGLProgram(const std::string &key);

        void addGLProgram(GLProgram* program, const std::string &key);

    private:
        bool init();
        void loadDefaultGLProgram(GLProgram *program, int type);

    private:
        std::unordered_map<std::string, GLProgram*> programs_;
    };
}

#endif // GLPROGRAM_H
