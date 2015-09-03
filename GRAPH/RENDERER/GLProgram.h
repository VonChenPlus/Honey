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
    /**VertexAttrib is a structure to encapsulate data got from glGetActiveAttrib.*/
    struct VertexAttrib
    {
        /**Index of attribute, start from 0.*/
        GLuint index;
        /**Number of Data type in the attribute, could range from 0-4.*/
        GLint size;
        /**Data type of the attribute, could be GL_FLOAT, GL_UNSIGNED_BYTE etc.*/
        GLenum type;
        /**The string name in vertex shader.*/
        std::string name;
    };

    /**Uniform is a structure to encapsulate data got from glGetActiveUniform and glGetUniformLocation.*/
    struct Uniform
    {
        /**The place where the uniform placed, starts from 0.*/
        GLint location;
        /**Number of data type in attribute.*/
        GLint size;
        /**Data type of the attribute.*/
        GLenum type;
        /**String of the uniform name.*/
        std::string name;
    };

    class GLProgram : public HObject
    {
    public:
        /**Enum the preallocated vertex attribute. */
        enum
        {
            /**Index 0 will be used as Position.*/
            VERTEX_ATTRIB_POSITION,
            /**Index 1 will be used as Color.*/
            VERTEX_ATTRIB_COLOR,
            /**Index 2 will be used as Tex coord unit 0.*/
            VERTEX_ATTRIB_TEX_COORD,
            /**Index 3 will be used as Tex coord unit 1.*/
            VERTEX_ATTRIB_TEX_COORD1,
            /**Index 4 will be used as Tex coord unit 2.*/
            VERTEX_ATTRIB_TEX_COORD2,
            /**Index 5 will be used as Tex coord unit 3.*/
            VERTEX_ATTRIB_TEX_COORD3,
            /**Index 6 will be used as Normal.*/
            VERTEX_ATTRIB_NORMAL,
            /**Index 7 will be used as Blend weight for hardware skin.*/
            VERTEX_ATTRIB_BLEND_WEIGHT,
            /**Index 8 will be used as Blend index.*/
            VERTEX_ATTRIB_BLEND_INDEX,
            VERTEX_ATTRIB_MAX,

            // backward compatibility
            VERTEX_ATTRIB_TEX_COORDS = VERTEX_ATTRIB_TEX_COORD,
        };

        /**Preallocated uniform handle.*/
        enum
        {
            /**Ambient color.*/
            UNIFORM_AMBIENT_COLOR,
            /**Projection matrix.*/
            UNIFORM_P_MATRIX,
            /**Model view matrix.*/
            UNIFORM_MV_MATRIX,
            /**Model view projection matrix.*/
            UNIFORM_MVP_MATRIX,
            /**Normal matrix.*/
            UNIFORM_NORMAL_MATRIX,
            /**Time.*/
            UNIFORM_TIME,
            /**sin(Time).*/
            UNIFORM_SIN_TIME,
            /**cos(Time).*/
            UNIFORM_COS_TIME,
            /**Random number.*/
            UNIFORM_RANDOM01,
            /** @{
            * Sampler 0-3, used for texture.
            */
            UNIFORM_SAMPLER0,
            UNIFORM_SAMPLER1,
            UNIFORM_SAMPLER2,
            UNIFORM_SAMPLER3,
            /**@}*/
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

        /**  It will add a new attribute to the shader by calling glBindAttribLocation. */
        void bindAttribLocation(const std::string& attributeName, GLuint index) const;

        /** Calls glGetAttribLocation. */
        GLint getAttribLocation(const std::string& attributeName) const;

        /** Calls glGetUniformLocation(). */
        GLint getUniformLocation(const std::string& attributeName) const;

        /** links the glProgram */
        bool link();
        /** it will call glUseProgram() */
        void use();

        void updateUniforms();

        /** calls retrieves the named uniform location for this shader program. */
        GLint getUniformLocationForName(const char* name) const;

        /** calls glUniform1i only if the values are different than the previous call for this same shader program.
         */
        void setUniformLocationWith1i(GLint location, GLint i1);

        /** calls glUniform2i only if the values are different than the previous call for this same shader program. */
        void setUniformLocationWith2i(GLint location, GLint i1, GLint i2);

        /** calls glUniform3i only if the values are different than the previous call for this same shader program. */
        void setUniformLocationWith3i(GLint location, GLint i1, GLint i2, GLint i3);

        /** calls glUniform4i only if the values are different than the previous call for this same shader program. */
        void setUniformLocationWith4i(GLint location, GLint i1, GLint i2, GLint i3, GLint i4);

        /** calls glUniform2iv only if the values are different than the previous call for this same shader program. */
        void setUniformLocationWith2iv(GLint location, GLint* ints, unsigned int numberOfArrays);

        /** calls glUniform3iv only if the values are different than the previous call for this same shader program. */
        void setUniformLocationWith3iv(GLint location, GLint* ints, unsigned int numberOfArrays);

        /** calls glUniform4iv only if the values are different than the previous call for this same shader program. */

        void setUniformLocationWith4iv(GLint location, GLint* ints, unsigned int numberOfArrays);

        /** calls glUniform1f only if the values are different than the previous call for this same shader program.
         * In js or lua,please use setUniformLocationF32
         */
        void setUniformLocationWith1f(GLint location, GLfloat f1);

        /** calls glUniform2f only if the values are different than the previous call for this same shader program.
         * In js or lua,please use setUniformLocationF32
         */
        void setUniformLocationWith2f(GLint location, GLfloat f1, GLfloat f2);

        /** calls glUniform3f only if the values are different than the previous call for this same shader program.
         * In js or lua,please use setUniformLocationF32
         */
        void setUniformLocationWith3f(GLint location, GLfloat f1, GLfloat f2, GLfloat f3);

        /** calls glUniform4f only if the values are different than the previous call for this same shader program.
         * In js or lua,please use setUniformLocationF32
         */
        void setUniformLocationWith4f(GLint location, GLfloat f1, GLfloat f2, GLfloat f3, GLfloat f4);

        /** calls glUniformfv only if the values are different than the previous call for this same shader program. */
        void setUniformLocationWith1fv(GLint location, const GLfloat* floats, unsigned int numberOfArrays);

        /** calls glUniform2fv only if the values are different than the previous call for this same shader program. */
        void setUniformLocationWith2fv(GLint location, const GLfloat* floats, unsigned int numberOfArrays);

        /** calls glUniform3fv only if the values are different than the previous call for this same shader program. */
        void setUniformLocationWith3fv(GLint location, const GLfloat* floats, unsigned int numberOfArrays);

        /** calls glUniform4fv only if the values are different than the previous call for this same shader program. */
        void setUniformLocationWith4fv(GLint location, const GLfloat* floats, unsigned int numberOfArrays);

        /** calls glUniformMatrix2fv only if the values are different than the previous call for this same shader program. */
        void setUniformLocationWithMatrix2fv(GLint location, const GLfloat* matrixArray, unsigned int numberOfMatrices);

        /** calls glUniformMatrix3fv only if the values are different than the previous call for this same shader program. */
        void setUniformLocationWithMatrix3fv(GLint location, const GLfloat* matrixArray, unsigned int numberOfMatrices);

        /** calls glUniformMatrix4fv only if the values are different than the previous call for this same shader program. */
        void setUniformLocationWithMatrix4fv(GLint location, const GLfloat* matrixArray, unsigned int numberOfMatrices);

        /**
         Update the builtin uniforms if they are different than the previous call for this same shader program.
         */
        void setUniformsForBuiltins();
        /**
         Update the builtin uniforms if they are different than the previous call for this same shader program.
         */
        void setUniformsForBuiltins(const MATH::Matrix4 &modelView);

        void reset();
        inline const GLuint getProgram() const { return _program; }

    protected:
        /**
        Update the uniform data in location.
        */
        bool updateUniformLocation(GLint location, const GLvoid* data, unsigned int bytes);

        /**Bind the predefined vertex attributes to their specific slot.*/
        void bindPredefinedVertexAttribs();
        /**Parse user defined Vertex Attributes automatically.*/
        void parseVertexAttribs();
        /**Parse user defined uniform automatically.*/
        void parseUniforms();
        /**Compile the shader sources.*/
        bool compileShader(GLuint * shader, GLenum type, const GLchar* source, const std::string& convertedDefines);
        bool compileShader(GLuint * shader, GLenum type, const GLchar* source);

        /**OpenGL handle for program.*/
        GLuint            _program;
        /**OpenGL handle for vertex shader.*/
        GLuint            _vertShader;
        /**OpenGL handle for fragment shader.*/
        GLuint            _fragShader;
        /**Built in uniforms.*/
        GLint             _builtInUniforms[UNIFORM_MAX];
        /**Indicate whether it has a offline shader compiler or not.*/
        bool              _hasShaderCompiler;

        struct flag_struct {
            unsigned int usesTime:1;
            unsigned int usesNormal:1;
            unsigned int usesMVP:1;
            unsigned int usesMV:1;
            unsigned int usesP:1;
            unsigned int usesRandom:1;
            // handy way to initialize the bitfield
            flag_struct() { memset(this, 0, sizeof(*this)); }
        } _flags;

        /**User defined Uniforms.*/
        std::unordered_map<std::string, Uniform> _userUniforms;
        /**User defined vertex attributes.*/
        std::unordered_map<std::string, VertexAttrib> _vertexAttribs;
        /**Hash value of uniforms for quick access.*/
        std::unordered_map<GLint, std::pair<GLvoid*, unsigned int>> _hashForUniforms;
    };
}

#endif // GLPROGRAM_H
