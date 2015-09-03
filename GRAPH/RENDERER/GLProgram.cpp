#include "GRAPH/RENDERER/GLProgram.h"
#include "GRAPH/Director.h"
#include "GRAPH/RENDERER/GLStateCache.h"
#include "GRAPH/RENDERER/GLStateCache.h"
#include "IO/FileUtils.h"
#include "UTILS/RANDOM/RandomUtils.h"

namespace GRAPH
{
    // helper functions
    static void ReplaceDefines(const std::string& compileTimeDefines, std::string& out) {
        // Replace semicolons with '#define ... \n'
        if (compileTimeDefines.size() > 0) {
            size_t pos;
            out = compileTimeDefines;
            out.insert(0, "#define ");
            while ((pos = out.find(';')) != std::string::npos) {
                out.replace(pos, 1, "\n#define ");
            }
            out += "\n";
        }
    }

    const char* GLProgram::SHADER_NAME_POSITION_TEXTURE_COLOR = "ShaderPositionTextureColor";
    const char* GLProgram::SHADER_NAME_POSITION_TEXTURE_COLOR_NO_MVP = "ShaderPositionTextureColor_noMVP";
    const char* GLProgram::SHADER_NAME_POSITION_TEXTURE_ALPHA_TEST = "ShaderPositionTextureColorAlphaTest";
    const char* GLProgram::SHADER_NAME_POSITION_TEXTURE_ALPHA_TEST_NO_MV = "ShaderPositionTextureColorAlphaTest_NoMV";
    const char* GLProgram::SHADER_NAME_POSITION_COLOR = "ShaderPositionColor";
    const char* GLProgram::SHADER_NAME_POSITION_COLOR_TEXASPOINTSIZE = "ShaderPositionColorTexAsPointsize";
    const char* GLProgram::SHADER_NAME_POSITION_COLOR_NO_MVP = "ShaderPositionColor_noMVP";

    const char* GLProgram::SHADER_NAME_POSITION_TEXTURE = "ShaderPositionTexture";
    const char* GLProgram::SHADER_NAME_POSITION_TEXTURE_U_COLOR = "ShaderPositionTexture_uColor";
    const char* GLProgram::SHADER_NAME_POSITION_TEXTURE_A8_COLOR = "ShaderPositionTextureA8Color";
    const char* GLProgram::SHADER_NAME_POSITION_U_COLOR = "ShaderPosition_uColor";
    const char* GLProgram::SHADER_NAME_POSITION_LENGTH_TEXTURE_COLOR = "ShaderPositionLengthTextureColor";
    const char* GLProgram::SHADER_NAME_POSITION_GRAYSCALE = "ShaderUIGrayScale";
    const char* GLProgram::SHADER_NAME_LABEL_DISTANCEFIELD_NORMAL = "ShaderLabelDFNormal";
    const char* GLProgram::SHADER_NAME_LABEL_DISTANCEFIELD_GLOW = "ShaderLabelDFGlow";
    const char* GLProgram::SHADER_NAME_LABEL_NORMAL = "ShaderLabelNormal";
    const char* GLProgram::SHADER_NAME_LABEL_OUTLINE = "ShaderLabelOutline";

    // uniform names
    const char* GLProgram::UNIFORM_NAME_AMBIENT_COLOR = "AmbientColor";
    const char* GLProgram::UNIFORM_NAME_P_MATRIX = "PMatrix";
    const char* GLProgram::UNIFORM_NAME_MV_MATRIX = "MVMatrix";
    const char* GLProgram::UNIFORM_NAME_MVP_MATRIX  = "MVPMatrix";
    const char* GLProgram::UNIFORM_NAME_NORMAL_MATRIX = "NormalMatrix";
    const char* GLProgram::UNIFORM_NAME_TIME = "Time";
    const char* GLProgram::UNIFORM_NAME_SIN_TIME = "SinTime";
    const char* GLProgram::UNIFORM_NAME_COS_TIME = "CosTime";
    const char* GLProgram::UNIFORM_NAME_RANDOM01 = "Random01";
    const char* GLProgram::UNIFORM_NAME_SAMPLER0 = "Texture0";
    const char* GLProgram::UNIFORM_NAME_SAMPLER1 = "Texture1";
    const char* GLProgram::UNIFORM_NAME_SAMPLER2 = "Texture2";
    const char* GLProgram::UNIFORM_NAME_SAMPLER3 = "Texture3";
    const char* GLProgram::UNIFORM_NAME_ALPHA_TEST_VALUE = "alpha_value";

    // Attribute names
    const char* GLProgram::ATTRIBUTE_NAME_COLOR = "a_color";
    const char* GLProgram::ATTRIBUTE_NAME_POSITION = "a_position";
    const char* GLProgram::ATTRIBUTE_NAME_TEX_COORD = "a_texCoord";
    const char* GLProgram::ATTRIBUTE_NAME_TEX_COORD1 = "a_texCoord1";
    const char* GLProgram::ATTRIBUTE_NAME_TEX_COORD2 = "a_texCoord2";
    const char* GLProgram::ATTRIBUTE_NAME_TEX_COORD3 = "a_texCoord3";
    const char* GLProgram::ATTRIBUTE_NAME_NORMAL = "a_normal";
    const char* GLProgram::ATTRIBUTE_NAME_BLEND_WEIGHT = "a_blendWeight";
    const char* GLProgram::ATTRIBUTE_NAME_BLEND_INDEX = "a_blendIndex";

    static const char * SHADER_UNIFORMS =
            "uniform mat4 PMatrix;\n"
            "uniform mat4 MVMatrix;\n"
            "uniform mat4 MVPMatrix;\n"
            "uniform mat3 NormalMatrix;\n"
            "uniform vec4 Time;\n"
            "uniform vec4 SinTime;\n"
            "uniform vec4 CosTime;\n"
            "uniform vec4 Random01;\n"
            "uniform sampler2D Texture0;\n"
            "uniform sampler2D Texture1;\n"
            "uniform sampler2D Texture2;\n"
            "uniform sampler2D Texture3;\n"
            "//INCLUDES END\n\n";

    static const std::string EMPTY_DEFINE;

    GLProgram* GLProgram::createWithByteArrays(const GLchar* vShaderByteArray, const GLchar* fShaderByteArray) {
        return createWithByteArrays(vShaderByteArray, fShaderByteArray, EMPTY_DEFINE);
    }

    GLProgram* GLProgram::createWithByteArrays(const GLchar* vShaderByteArray, const GLchar* fShaderByteArray, const std::string& compileTimeDefines) {
        auto ret = new (std::nothrow) GLProgram();
        if(ret && ret->initWithByteArrays(vShaderByteArray, fShaderByteArray, compileTimeDefines)) {
            ret->link();
            ret->updateUniforms();
            ret->autorelease();
            return ret;
        }

        SAFE_DELETE(ret);
        return nullptr;
    }


    GLProgram* GLProgram::createWithFilenames(const std::string& vShaderFilename, const std::string& fShaderFilename) {
        return createWithFilenames(vShaderFilename, fShaderFilename, EMPTY_DEFINE);
    }

    GLProgram* GLProgram::createWithFilenames(const std::string& vShaderFilename, const std::string& fShaderFilename, const std::string& compileTimeDefines) {
        auto ret = new (std::nothrow) GLProgram();
        if(ret && ret->initWithFilenames(vShaderFilename, fShaderFilename, compileTimeDefines)) {
            ret->link();
            ret->updateUniforms();
            ret->autorelease();
            return ret;
        }

        SAFE_DELETE(ret);
        return nullptr;
    }


    GLProgram::GLProgram()
        : _program(0)
        , _vertShader(0)
        , _fragShader(0)
        , _flags() {
        memset(_builtInUniforms, 0, sizeof(_builtInUniforms));
    }

    GLProgram::~GLProgram() {
        if (_vertShader) {
            glDeleteShader(_vertShader);
        }

        if (_fragShader) {
            glDeleteShader(_fragShader);
        }

        _vertShader = _fragShader = 0;

        if (_program) {
            GLStateCache::DeleteProgram(_program);
        }

        for (auto e : _hashForUniforms) {
            free(e.second.first);
        }
        _hashForUniforms.clear();
    }

    bool GLProgram::initWithByteArrays(const GLchar* vShaderByteArray, const GLchar* fShaderByteArray) {
        return initWithByteArrays(vShaderByteArray, fShaderByteArray, EMPTY_DEFINE);
    }

    bool GLProgram::initWithByteArrays(const GLchar* vShaderByteArray, const GLchar* fShaderByteArray, const std::string& compileTimeDefines) {
        _program = glCreateProgram();

        // convert defines here. If we do it in "compileShader" we will do it it twice.
        // a cache for the defines could be useful, but seems like overkill at this point
        std::string replacedDefines = "";
        ReplaceDefines(compileTimeDefines, replacedDefines);

        _vertShader = _fragShader = 0;

        if (vShaderByteArray) {
            if (!compileShader(&_vertShader, GL_VERTEX_SHADER, vShaderByteArray, replacedDefines)) {
                return false;
           }
        }

        // Create and compile fragment shader
        if (fShaderByteArray) {
            if (!compileShader(&_fragShader, GL_FRAGMENT_SHADER, fShaderByteArray, replacedDefines)) {
                return false;
            }
        }

        if (_vertShader) {
            glAttachShader(_program, _vertShader);
        }

        if (_fragShader) {
            glAttachShader(_program, _fragShader);
        }

        _hashForUniforms.clear();

        return true;
    }

    bool GLProgram::initWithFilenames(const std::string& vShaderFilename, const std::string& fShaderFilename) {
        return initWithFilenames(vShaderFilename, fShaderFilename, EMPTY_DEFINE);
    }

    bool GLProgram::initWithFilenames(const std::string& vShaderFilename, const std::string& fShaderFilename, const std::string& compileTimeDefines) {
        std::string vertexSource = IO::FileUtils::getInstance().getStringFromFile(IO::FileUtils::getInstance().fullPathForFilename(vShaderFilename));
        std::string fragmentSource = IO::FileUtils::getInstance().getStringFromFile(IO::FileUtils::getInstance().fullPathForFilename(fShaderFilename));

        return initWithByteArrays(vertexSource.c_str(), fragmentSource.c_str(), compileTimeDefines);
    }

    void GLProgram::bindPredefinedVertexAttribs() {
        static const struct {
            const char *attributeName;
            int location;
        } attribute_locations[] =
        {
            {GLProgram::ATTRIBUTE_NAME_POSITION, GLProgram::VERTEX_ATTRIB_POSITION},
            {GLProgram::ATTRIBUTE_NAME_COLOR, GLProgram::VERTEX_ATTRIB_COLOR},
            {GLProgram::ATTRIBUTE_NAME_TEX_COORD, GLProgram::VERTEX_ATTRIB_TEX_COORD},
            {GLProgram::ATTRIBUTE_NAME_TEX_COORD1, GLProgram::VERTEX_ATTRIB_TEX_COORD1},
            {GLProgram::ATTRIBUTE_NAME_TEX_COORD2, GLProgram::VERTEX_ATTRIB_TEX_COORD2},
            {GLProgram::ATTRIBUTE_NAME_TEX_COORD3, GLProgram::VERTEX_ATTRIB_TEX_COORD3},
            {GLProgram::ATTRIBUTE_NAME_NORMAL, GLProgram::VERTEX_ATTRIB_NORMAL},
        };

        const int size = sizeof(attribute_locations) / sizeof(attribute_locations[0]);

        for(int i=0; i<size;i++) {
            glBindAttribLocation(_program, attribute_locations[i].location, attribute_locations[i].attributeName);
        }
    }

    void GLProgram::parseVertexAttribs() {
        // Query and store vertex attribute meta-data from the program.
        GLint activeAttributes;
        GLint length;
        glGetProgramiv(_program, GL_ACTIVE_ATTRIBUTES, &activeAttributes);
        if(activeAttributes > 0) {
            VertexAttrib attribute;

            glGetProgramiv(_program, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &length);
            if(length > 0) {
                GLchar* attribName = (GLchar*) alloca(length + 1);

                for(int i = 0; i < activeAttributes; ++i) {
                    // Query attribute info.
                    glGetActiveAttrib(_program, i, length, nullptr, &attribute.size, &attribute.type, attribName);
                    attribName[length] = '\0';
                    attribute.name = std::string(attribName);

                    // Query the pre-assigned attribute location
                    attribute.index = glGetAttribLocation(_program, attribName);
                    _vertexAttribs[attribute.name] = attribute;
                }
            }
        }
        else {
            GLchar ErrorLog[1024];
            glGetProgramInfoLog(_program, sizeof(ErrorLog), NULL, ErrorLog);
        }
    }

    void GLProgram::parseUniforms() {
        // Query and store uniforms from the program.
        GLint activeUniforms;
        glGetProgramiv(_program, GL_ACTIVE_UNIFORMS, &activeUniforms);
        if(activeUniforms > 0) {
            GLint length;
            glGetProgramiv(_program, GL_ACTIVE_UNIFORM_MAX_LENGTH, &length);
            if(length > 0) {
                Uniform uniform;

                GLchar* uniformName = (GLchar*)alloca(length + 1);

                for(int i = 0; i < activeUniforms; ++i) {
                    // Query uniform info.
                    glGetActiveUniform(_program, i, length, nullptr, &uniform.size, &uniform.type, uniformName);
                    uniformName[length] = '\0';

                    // Only add uniforms that are not built-in.
                    // The ones that start with '' are built-ins
                    if(strncmp("", uniformName, 3) != 0) {
                        // remove possible array '[]' from uniform name
                        if(length > 3)
                        {
                            char* c = strrchr(uniformName, '[');
                            if(c)
                            {
                                *c = '\0';
                            }
                        }
                        uniform.name = std::string(uniformName);
                        uniform.location = glGetUniformLocation(_program, uniformName);
                        _userUniforms[uniform.name] = uniform;
                    }
                }
            }
        }
        else {
            GLchar ErrorLog[1024];
            glGetProgramInfoLog(_program, sizeof(ErrorLog), NULL, ErrorLog);
        }

    }

    Uniform* GLProgram::getUniform(const std::string &name) {
        const auto itr = _userUniforms.find(name);
        if( itr != _userUniforms.end())
            return &itr->second;
        return nullptr;
    }

    VertexAttrib* GLProgram::getVertexAttrib(const std::string &name) {
        const auto itr = _vertexAttribs.find(name);
        if( itr != _vertexAttribs.end())
            return &itr->second;
        return nullptr;
    }

    bool GLProgram::compileShader(GLuint * shader, GLenum type, const GLchar* source) {
        return compileShader(shader, type, source, "");
    }

    bool GLProgram::compileShader(GLuint* shader, GLenum type, const GLchar* source, const std::string& convertedDefines) {
        GLint status;

        if (!source) {
            return false;
        }

        const GLchar *sources[] = {
            SHADER_UNIFORMS,
            convertedDefines.c_str(),
            source};

        *shader = glCreateShader(type);
        glShaderSource(*shader, sizeof(sources)/sizeof(*sources), sources, nullptr);
        glCompileShader(*shader);

        glGetShaderiv(*shader, GL_COMPILE_STATUS, &status);

        if (! status) {
            GLsizei length;
            glGetShaderiv(*shader, GL_SHADER_SOURCE_LENGTH, &length);
            GLchar* src = (GLchar *)malloc(sizeof(GLchar) * length);

            glGetShaderSource(*shader, length, nullptr, src);
            free(src);

            return false;;
        }

        return (status == GL_TRUE);
    }

    GLint GLProgram::getAttribLocation(const std::string &attributeName) const {
        return glGetAttribLocation(_program, attributeName.c_str());
    }

    GLint GLProgram::getUniformLocation(const std::string &attributeName) const {
        return glGetUniformLocation(_program, attributeName.c_str());
    }

    void GLProgram::bindAttribLocation(const std::string &attributeName, GLuint index) const {
        glBindAttribLocation(_program, index, attributeName.c_str());
    }

    void GLProgram::updateUniforms() {
        _builtInUniforms[UNIFORM_AMBIENT_COLOR] = glGetUniformLocation(_program, UNIFORM_NAME_AMBIENT_COLOR);
        _builtInUniforms[UNIFORM_P_MATRIX] = glGetUniformLocation(_program, UNIFORM_NAME_P_MATRIX);
        _builtInUniforms[UNIFORM_MV_MATRIX] = glGetUniformLocation(_program, UNIFORM_NAME_MV_MATRIX);
        _builtInUniforms[UNIFORM_MVP_MATRIX] = glGetUniformLocation(_program, UNIFORM_NAME_MVP_MATRIX);
        _builtInUniforms[UNIFORM_NORMAL_MATRIX] = glGetUniformLocation(_program, UNIFORM_NAME_NORMAL_MATRIX);

        _builtInUniforms[UNIFORM_TIME] = glGetUniformLocation(_program, UNIFORM_NAME_TIME);
        _builtInUniforms[UNIFORM_SIN_TIME] = glGetUniformLocation(_program, UNIFORM_NAME_SIN_TIME);
        _builtInUniforms[UNIFORM_COS_TIME] = glGetUniformLocation(_program, UNIFORM_NAME_COS_TIME);

        _builtInUniforms[UNIFORM_RANDOM01] = glGetUniformLocation(_program, UNIFORM_NAME_RANDOM01);

        _builtInUniforms[UNIFORM_SAMPLER0] = glGetUniformLocation(_program, UNIFORM_NAME_SAMPLER0);
        _builtInUniforms[UNIFORM_SAMPLER1] = glGetUniformLocation(_program, UNIFORM_NAME_SAMPLER1);
        _builtInUniforms[UNIFORM_SAMPLER2] = glGetUniformLocation(_program, UNIFORM_NAME_SAMPLER2);
        _builtInUniforms[UNIFORM_SAMPLER3] = glGetUniformLocation(_program, UNIFORM_NAME_SAMPLER3);

        _flags.usesP = _builtInUniforms[UNIFORM_P_MATRIX] != -1;
        _flags.usesMV = _builtInUniforms[UNIFORM_MV_MATRIX] != -1;
        _flags.usesMVP = _builtInUniforms[UNIFORM_MVP_MATRIX] != -1;
        _flags.usesNormal = _builtInUniforms[UNIFORM_NORMAL_MATRIX] != -1;
        _flags.usesTime = (
                           _builtInUniforms[UNIFORM_TIME] != -1 ||
                           _builtInUniforms[UNIFORM_SIN_TIME] != -1 ||
                           _builtInUniforms[UNIFORM_COS_TIME] != -1
                           );
        _flags.usesRandom = _builtInUniforms[UNIFORM_RANDOM01] != -1;

        this->use();

        // Since sample most probably won't change, set it to 0,1,2,3 now.
        if(_builtInUniforms[UNIFORM_SAMPLER0] != -1)
           setUniformLocationWith1i(_builtInUniforms[UNIFORM_SAMPLER0], 0);
        if(_builtInUniforms[UNIFORM_SAMPLER1] != -1)
            setUniformLocationWith1i(_builtInUniforms[UNIFORM_SAMPLER1], 1);
        if(_builtInUniforms[UNIFORM_SAMPLER2] != -1)
            setUniformLocationWith1i(_builtInUniforms[UNIFORM_SAMPLER2], 2);
        if(_builtInUniforms[UNIFORM_SAMPLER3] != -1)
            setUniformLocationWith1i(_builtInUniforms[UNIFORM_SAMPLER3], 3);
    }

    bool GLProgram::link() {
        GLint status = GL_TRUE;

        bindPredefinedVertexAttribs();

        glLinkProgram(_program);

        parseVertexAttribs();
        parseUniforms();

        if (_vertShader) {
            glDeleteShader(_vertShader);
        }

        if (_fragShader) {
            glDeleteShader(_fragShader);
        }

        _vertShader = _fragShader = 0;

        return (status == GL_TRUE);
    }

    void GLProgram::use() {
        GLStateCache::UseProgram(_program);
    }

    // Uniform cache
    bool GLProgram::updateUniformLocation(GLint location, const GLvoid* data, unsigned int bytes) {
        if (location < 0) {
            return false;
        }

        bool updated = true;

        auto element = _hashForUniforms.find(location);
        if (element == _hashForUniforms.end()) {
            GLvoid* value = malloc(bytes);
            memcpy(value, data, bytes );
            _hashForUniforms.insert(std::make_pair(location, std::make_pair(value, bytes)));
        }
        else {
            if (memcmp(element->second.first, data, bytes) == 0) {
                updated = false;
            }
            else {
                if (element->second.second < bytes) {
                    GLvoid* value = realloc(element->second.first, bytes);
                    memcpy(value, data, bytes );
                    _hashForUniforms[location] = std::make_pair(value, bytes);
                }
                else
                    memcpy(element->second.first, data, bytes);
            }
        }

        return updated;
    }

    GLint GLProgram::getUniformLocationForName(const char* name) const {
        return glGetUniformLocation(_program, name);
    }

    void GLProgram::setUniformLocationWith1i(GLint location, GLint i1) {
        bool updated = updateUniformLocation(location, &i1, sizeof(i1)*1);

        if (updated)
        {
            glUniform1i( (GLint)location, i1);
        }
    }

    void GLProgram::setUniformLocationWith2i(GLint location, GLint i1, GLint i2) {
        GLint ints[2] = {i1,i2};
        bool updated = updateUniformLocation(location, ints, sizeof(ints));

        if (updated) {
            glUniform2i( (GLint)location, i1, i2);
        }
    }

    void GLProgram::setUniformLocationWith3i(GLint location, GLint i1, GLint i2, GLint i3) {
        GLint ints[3] = {i1,i2,i3};
        bool updated = updateUniformLocation(location, ints, sizeof(ints));

        if (updated) {
            glUniform3i( (GLint)location, i1, i2, i3);
        }
    }

    void GLProgram::setUniformLocationWith4i(GLint location, GLint i1, GLint i2, GLint i3, GLint i4) {
        GLint ints[4] = {i1,i2,i3,i4};
        bool updated = updateUniformLocation(location, ints, sizeof(ints));

        if (updated) {
            glUniform4i( (GLint)location, i1, i2, i3, i4);
        }
    }

    void GLProgram::setUniformLocationWith2iv(GLint location, GLint* ints, unsigned int numberOfArrays) {
        bool updated = updateUniformLocation(location, ints, sizeof(int)*2*numberOfArrays);

        if (updated) {
            glUniform2iv( (GLint)location, (GLsizei)numberOfArrays, ints );
        }
    }

    void GLProgram::setUniformLocationWith3iv(GLint location, GLint* ints, unsigned int numberOfArrays) {
        bool updated = updateUniformLocation(location, ints, sizeof(int)*3*numberOfArrays);

        if (updated) {
            glUniform3iv( (GLint)location, (GLsizei)numberOfArrays, ints );
        }
    }

    void GLProgram::setUniformLocationWith4iv(GLint location, GLint* ints, unsigned int numberOfArrays) {
        bool updated = updateUniformLocation(location, ints, sizeof(int)*4*numberOfArrays);

        if (updated) {
            glUniform4iv( (GLint)location, (GLsizei)numberOfArrays, ints );
        }
    }

    void GLProgram::setUniformLocationWith1f(GLint location, GLfloat f1) {
        bool updated = updateUniformLocation(location, &f1, sizeof(f1)*1);

        if (updated) {
            glUniform1f( (GLint)location, f1);
        }
    }

    void GLProgram::setUniformLocationWith2f(GLint location, GLfloat f1, GLfloat f2) {
        GLfloat floats[2] = {f1,f2};
        bool updated =  updateUniformLocation(location, floats, sizeof(floats));

        if (updated) {
            glUniform2f( (GLint)location, f1, f2);
        }
    }

    void GLProgram::setUniformLocationWith3f(GLint location, GLfloat f1, GLfloat f2, GLfloat f3) {
        GLfloat floats[3] = {f1,f2,f3};
        bool updated = updateUniformLocation(location, floats, sizeof(floats));

        if (updated) {
            glUniform3f( (GLint)location, f1, f2, f3);
        }
    }

    void GLProgram::setUniformLocationWith4f(GLint location, GLfloat f1, GLfloat f2, GLfloat f3, GLfloat f4) {
        GLfloat floats[4] = {f1,f2,f3,f4};
        bool updated = updateUniformLocation(location, floats, sizeof(floats));

        if (updated) {
            glUniform4f( (GLint)location, f1, f2, f3,f4);
        }
    }


    void GLProgram::setUniformLocationWith1fv( GLint location, const GLfloat* floats, unsigned int numberOfArrays ) {
        bool updated = updateUniformLocation(location, floats, sizeof(float)*numberOfArrays);

        if (updated) {
            glUniform1fv( (GLint)location, (GLsizei)numberOfArrays, floats );
        }
    }

    void GLProgram::setUniformLocationWith2fv(GLint location, const GLfloat* floats, unsigned int numberOfArrays) {
        bool updated = updateUniformLocation(location, floats, sizeof(float)*2*numberOfArrays);

        if (updated) {
            glUniform2fv( (GLint)location, (GLsizei)numberOfArrays, floats );
        }
    }

    void GLProgram::setUniformLocationWith3fv(GLint location, const GLfloat* floats, unsigned int numberOfArrays) {
        bool updated = updateUniformLocation(location, floats, sizeof(float)*3*numberOfArrays);

        if (updated) {
            glUniform3fv( (GLint)location, (GLsizei)numberOfArrays, floats );
        }
    }

    void GLProgram::setUniformLocationWith4fv(GLint location, const GLfloat* floats, unsigned int numberOfArrays) {
        bool updated = updateUniformLocation(location, floats, sizeof(float)*4*numberOfArrays);

        if (updated) {
            glUniform4fv( (GLint)location, (GLsizei)numberOfArrays, floats );
        }
    }

    void GLProgram::setUniformLocationWithMatrix2fv(GLint location, const GLfloat* matrixArray, unsigned int numberOfMatrices) {
        bool updated = updateUniformLocation(location, matrixArray, sizeof(float)*4*numberOfMatrices);

        if (updated) {
            glUniformMatrix2fv( (GLint)location, (GLsizei)numberOfMatrices, GL_FALSE, matrixArray);
        }
    }

    void GLProgram::setUniformLocationWithMatrix3fv(GLint location, const GLfloat* matrixArray, unsigned int numberOfMatrices) {
        bool updated = updateUniformLocation(location, matrixArray, sizeof(float)*9*numberOfMatrices);

        if (updated) {
            glUniformMatrix3fv( (GLint)location, (GLsizei)numberOfMatrices, GL_FALSE, matrixArray);
        }
    }


    void GLProgram::setUniformLocationWithMatrix4fv(GLint location, const GLfloat* matrixArray, unsigned int numberOfMatrices) {
        bool updated = updateUniformLocation(location, matrixArray, sizeof(float)*16*numberOfMatrices);

        if (updated) {
            glUniformMatrix4fv( (GLint)location, (GLsizei)numberOfMatrices, GL_FALSE, matrixArray);
        }
    }

    void GLProgram::setUniformsForBuiltins() {
        setUniformsForBuiltins(Director::getInstance().getMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW));
    }

    void GLProgram::setUniformsForBuiltins(const MATH::Matrix4 &matrixMV) {
        auto& matrixP = Director::getInstance().getMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_PROJECTION);

        if (_flags.usesP)
            setUniformLocationWithMatrix4fv(_builtInUniforms[UNIFORM_P_MATRIX], matrixP.m, 1);

        if (_flags.usesMV)
            setUniformLocationWithMatrix4fv(_builtInUniforms[UNIFORM_MV_MATRIX], matrixMV.m, 1);

        if (_flags.usesMVP) {
            MATH::Matrix4 matrixMVP = matrixP * matrixMV;
            setUniformLocationWithMatrix4fv(_builtInUniforms[UNIFORM_MVP_MATRIX], matrixMVP.m, 1);
        }

        if (_flags.usesNormal) {
            MATH::Matrix4 mvInverse = matrixMV;
            mvInverse.m[12] = mvInverse.m[13] = mvInverse.m[14] = 0.0f;
            mvInverse.inverse();
            mvInverse.transpose();
            GLfloat normalMat[9];
            normalMat[0] = mvInverse.m[0];normalMat[1] = mvInverse.m[1];normalMat[2] = mvInverse.m[2];
            normalMat[3] = mvInverse.m[4];normalMat[4] = mvInverse.m[5];normalMat[5] = mvInverse.m[6];
            normalMat[6] = mvInverse.m[8];normalMat[7] = mvInverse.m[9];normalMat[8] = mvInverse.m[10];
            setUniformLocationWithMatrix3fv(_builtInUniforms[UNIFORM_NORMAL_MATRIX], normalMat, 1);
        }

        if (_flags.usesTime) {
            throw _HException_Normal("Unsupport GlProgram the most accurate global time value.");
            float time = 0.0f;
            setUniformLocationWith4f(_builtInUniforms[GLProgram::UNIFORM_TIME], time/10.0, time, time*2, time*4);
            setUniformLocationWith4f(_builtInUniforms[GLProgram::UNIFORM_SIN_TIME], time/8.0, time/4.0, time/2.0, sinf(time));
            setUniformLocationWith4f(_builtInUniforms[GLProgram::UNIFORM_COS_TIME], time/8.0, time/4.0, time/2.0, cosf(time));
        }

        if (_flags.usesRandom)
            setUniformLocationWith4f(_builtInUniforms[GLProgram::UNIFORM_RANDOM01], UTILS::RANDOM::RANDOM_0_1(), UTILS::RANDOM::RANDOM_0_1(), UTILS::RANDOM::RANDOM_0_1(), UTILS::RANDOM::RANDOM_0_1());
    }

    void GLProgram::reset() {
        _vertShader = _fragShader = 0;
        memset(_builtInUniforms, 0, sizeof(_builtInUniforms));

        _program = 0;

        for (auto e: _hashForUniforms) {
            free(e.second.first);
        }

        _hashForUniforms.clear();
    }
}
