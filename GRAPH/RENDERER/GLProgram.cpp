#include "GRAPH/RENDERER/GLProgram.h"
#include "GRAPH/Director.h"
#include "GRAPH/RENDERER/GLStateCache.h"
#include "GRAPH/RENDERER/Shaders.h"
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
        : program_(0)
        , vertShader_(0)
        , fragShader_(0)
        , uniformsFlags_() {
        memset(builtInUniforms_, 0, sizeof(builtInUniforms_));
    }

    GLProgram::~GLProgram() {
        if (vertShader_) {
            glDeleteShader(vertShader_);
        }

        if (fragShader_) {
            glDeleteShader(fragShader_);
        }

        vertShader_ = fragShader_ = 0;

        if (program_) {
            GLStateCache::DeleteProgram(program_);
        }

        for (auto e : hashForUniforms_) {
            free(e.second.first);
        }
        hashForUniforms_.clear();
    }

    bool GLProgram::initWithByteArrays(const GLchar* vShaderByteArray, const GLchar* fShaderByteArray) {
        return initWithByteArrays(vShaderByteArray, fShaderByteArray, EMPTY_DEFINE);
    }

    bool GLProgram::initWithByteArrays(const GLchar* vShaderByteArray, const GLchar* fShaderByteArray, const std::string& compileTimeDefines) {
        program_ = glCreateProgram();

        // convert defines here. If we do it in "compileShader" we will do it it twice.
        // a cache for the defines could be useful, but seems like overkill at this point
        std::string replacedDefines = "";
        ReplaceDefines(compileTimeDefines, replacedDefines);

        vertShader_ = fragShader_ = 0;

        if (vShaderByteArray) {
            if (!compileShader(&vertShader_, GL_VERTEX_SHADER, vShaderByteArray, replacedDefines)) {
                return false;
           }
        }

        // Create and compile fragment shader
        if (fShaderByteArray) {
            if (!compileShader(&fragShader_, GL_FRAGMENT_SHADER, fShaderByteArray, replacedDefines)) {
                return false;
            }
        }

        if (vertShader_) {
            glAttachShader(program_, vertShader_);
        }

        if (fragShader_) {
            glAttachShader(program_, fragShader_);
        }

        hashForUniforms_.clear();

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
            glBindAttribLocation(program_, attribute_locations[i].location, attribute_locations[i].attributeName);
        }
    }

    void GLProgram::parseVertexAttribs() {
        // Query and store vertex attribute meta-data from the program.
        GLint activeAttributes;
        GLint length;
        glGetProgramiv(program_, GL_ACTIVE_ATTRIBUTES, &activeAttributes);
        if(activeAttributes > 0) {
            VertexAttrib attribute;

            glGetProgramiv(program_, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &length);
            if(length > 0) {
                GLchar* attribName = (GLchar*) alloca(length + 1);

                for(int i = 0; i < activeAttributes; ++i) {
                    // Query attribute info.
                    glGetActiveAttrib(program_, i, length, nullptr, &attribute.size, &attribute.type, attribName);
                    attribName[length] = '\0';
                    attribute.name = std::string(attribName);

                    // Query the pre-assigned attribute location
                    attribute.index = glGetAttribLocation(program_, attribName);
                    vertexAttribs_[attribute.name] = attribute;
                }
            }
        }
        else {
            GLchar ErrorLog[1024];
            glGetProgramInfoLog(program_, sizeof(ErrorLog), NULL, ErrorLog);
        }
    }

    void GLProgram::parseUniforms() {
        // Query and store uniforms from the program.
        GLint activeUniforms;
        glGetProgramiv(program_, GL_ACTIVE_UNIFORMS, &activeUniforms);
        if(activeUniforms > 0) {
            GLint length;
            glGetProgramiv(program_, GL_ACTIVE_UNIFORM_MAX_LENGTH, &length);
            if(length > 0) {
                Uniform uniform;

                GLchar* uniformName = (GLchar*)alloca(length + 1);

                for(int i = 0; i < activeUniforms; ++i) {
                    // Query uniform info.
                    glGetActiveUniform(program_, i, length, nullptr, &uniform.size, &uniform.type, uniformName);
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
                        uniform.location = glGetUniformLocation(program_, uniformName);
                        userUniforms_[uniform.name] = uniform;
                    }
                }
            }
        }
        else {
            GLchar ErrorLog[1024];
            glGetProgramInfoLog(program_, sizeof(ErrorLog), NULL, ErrorLog);
        }

    }

    Uniform* GLProgram::getUniform(const std::string &name) {
        const auto itr = userUniforms_.find(name);
        if( itr != userUniforms_.end())
            return &itr->second;
        return nullptr;
    }

    VertexAttrib* GLProgram::getVertexAttrib(const std::string &name) {
        const auto itr = vertexAttribs_.find(name);
        if( itr != vertexAttribs_.end())
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
        return glGetAttribLocation(program_, attributeName.c_str());
    }

    GLint GLProgram::getUniformLocation(const std::string &attributeName) const {
        return glGetUniformLocation(program_, attributeName.c_str());
    }

    void GLProgram::bindAttribLocation(const std::string &attributeName, GLuint index) const {
        glBindAttribLocation(program_, index, attributeName.c_str());
    }

    void GLProgram::updateUniforms() {
        builtInUniforms_[UNIFORM_AMBIENT_COLOR] = glGetUniformLocation(program_, UNIFORM_NAME_AMBIENT_COLOR);
        builtInUniforms_[UNIFORM_P_MATRIX] = glGetUniformLocation(program_, UNIFORM_NAME_P_MATRIX);
        builtInUniforms_[UNIFORM_MV_MATRIX] = glGetUniformLocation(program_, UNIFORM_NAME_MV_MATRIX);
        builtInUniforms_[UNIFORM_MVP_MATRIX] = glGetUniformLocation(program_, UNIFORM_NAME_MVP_MATRIX);
        builtInUniforms_[UNIFORM_NORMAL_MATRIX] = glGetUniformLocation(program_, UNIFORM_NAME_NORMAL_MATRIX);

        builtInUniforms_[UNIFORM_TIME] = glGetUniformLocation(program_, UNIFORM_NAME_TIME);
        builtInUniforms_[UNIFORM_SIN_TIME] = glGetUniformLocation(program_, UNIFORM_NAME_SIN_TIME);
        builtInUniforms_[UNIFORM_COS_TIME] = glGetUniformLocation(program_, UNIFORM_NAME_COS_TIME);

        builtInUniforms_[UNIFORM_RANDOM01] = glGetUniformLocation(program_, UNIFORM_NAME_RANDOM01);

        builtInUniforms_[UNIFORM_SAMPLER0] = glGetUniformLocation(program_, UNIFORM_NAME_SAMPLER0);
        builtInUniforms_[UNIFORM_SAMPLER1] = glGetUniformLocation(program_, UNIFORM_NAME_SAMPLER1);
        builtInUniforms_[UNIFORM_SAMPLER2] = glGetUniformLocation(program_, UNIFORM_NAME_SAMPLER2);
        builtInUniforms_[UNIFORM_SAMPLER3] = glGetUniformLocation(program_, UNIFORM_NAME_SAMPLER3);

        uniformsFlags_.usesP = builtInUniforms_[UNIFORM_P_MATRIX] != -1;
        uniformsFlags_.usesMV = builtInUniforms_[UNIFORM_MV_MATRIX] != -1;
        uniformsFlags_.usesMVP = builtInUniforms_[UNIFORM_MVP_MATRIX] != -1;
        uniformsFlags_.usesNormal = builtInUniforms_[UNIFORM_NORMAL_MATRIX] != -1;
        uniformsFlags_.usesTime = (
                           builtInUniforms_[UNIFORM_TIME] != -1 ||
                           builtInUniforms_[UNIFORM_SIN_TIME] != -1 ||
                           builtInUniforms_[UNIFORM_COS_TIME] != -1
                           );
        uniformsFlags_.usesRandom = builtInUniforms_[UNIFORM_RANDOM01] != -1;

        this->use();

        // Since sample most probably won't change, set it to 0,1,2,3 now.
        if(builtInUniforms_[UNIFORM_SAMPLER0] != -1)
           setUniformLocationWith1i(builtInUniforms_[UNIFORM_SAMPLER0], 0);
        if(builtInUniforms_[UNIFORM_SAMPLER1] != -1)
            setUniformLocationWith1i(builtInUniforms_[UNIFORM_SAMPLER1], 1);
        if(builtInUniforms_[UNIFORM_SAMPLER2] != -1)
            setUniformLocationWith1i(builtInUniforms_[UNIFORM_SAMPLER2], 2);
        if(builtInUniforms_[UNIFORM_SAMPLER3] != -1)
            setUniformLocationWith1i(builtInUniforms_[UNIFORM_SAMPLER3], 3);
    }

    bool GLProgram::link() {
        GLint status = GL_TRUE;

        bindPredefinedVertexAttribs();

        glLinkProgram(program_);

        parseVertexAttribs();
        parseUniforms();

        if (vertShader_) {
            glDeleteShader(vertShader_);
        }

        if (fragShader_) {
            glDeleteShader(fragShader_);
        }

        vertShader_ = fragShader_ = 0;

        return (status == GL_TRUE);
    }

    void GLProgram::use() {
        GLStateCache::UseProgram(program_);
    }

    // Uniform cache
    bool GLProgram::updateUniformLocation(GLint location, const GLvoid* data, unsigned int bytes) {
        if (location < 0) {
            return false;
        }

        bool updated = true;

        auto element = hashForUniforms_.find(location);
        if (element == hashForUniforms_.end()) {
            GLvoid* value = malloc(bytes);
            memcpy(value, data, bytes );
            hashForUniforms_.insert(std::make_pair(location, std::make_pair(value, bytes)));
        }
        else {
            if (memcmp(element->second.first, data, bytes) == 0) {
                updated = false;
            }
            else {
                if (element->second.second < bytes) {
                    GLvoid* value = realloc(element->second.first, bytes);
                    memcpy(value, data, bytes );
                    hashForUniforms_[location] = std::make_pair(value, bytes);
                }
                else
                    memcpy(element->second.first, data, bytes);
            }
        }

        return updated;
    }

    GLint GLProgram::getUniformLocationForName(const char* name) const {
        return glGetUniformLocation(program_, name);
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

        if (uniformsFlags_.usesP)
            setUniformLocationWithMatrix4fv(builtInUniforms_[UNIFORM_P_MATRIX], matrixP.m, 1);

        if (uniformsFlags_.usesMV)
            setUniformLocationWithMatrix4fv(builtInUniforms_[UNIFORM_MV_MATRIX], matrixMV.m, 1);

        if (uniformsFlags_.usesMVP) {
            MATH::Matrix4 matrixMVP = matrixP * matrixMV;
            setUniformLocationWithMatrix4fv(builtInUniforms_[UNIFORM_MVP_MATRIX], matrixMVP.m, 1);
        }

        if (uniformsFlags_.usesNormal) {
            MATH::Matrix4 mvInverse = matrixMV;
            mvInverse.m[12] = mvInverse.m[13] = mvInverse.m[14] = 0.0f;
            mvInverse.inverse();
            mvInverse.transpose();
            GLfloat normalMat[9];
            normalMat[0] = mvInverse.m[0];normalMat[1] = mvInverse.m[1];normalMat[2] = mvInverse.m[2];
            normalMat[3] = mvInverse.m[4];normalMat[4] = mvInverse.m[5];normalMat[5] = mvInverse.m[6];
            normalMat[6] = mvInverse.m[8];normalMat[7] = mvInverse.m[9];normalMat[8] = mvInverse.m[10];
            setUniformLocationWithMatrix3fv(builtInUniforms_[UNIFORM_NORMAL_MATRIX], normalMat, 1);
        }

        if (uniformsFlags_.usesTime) {
            throw _HException_Normal("Unsupport GlProgram the most accurate global time value.");
            float time = 0.0f;
            setUniformLocationWith4f(builtInUniforms_[GLProgram::UNIFORM_TIME], time/10.0, time, time*2, time*4);
            setUniformLocationWith4f(builtInUniforms_[GLProgram::UNIFORM_SIN_TIME], time/8.0, time/4.0, time/2.0, sinf(time));
            setUniformLocationWith4f(builtInUniforms_[GLProgram::UNIFORM_COS_TIME], time/8.0, time/4.0, time/2.0, cosf(time));
        }

        if (uniformsFlags_.usesRandom)
            setUniformLocationWith4f(builtInUniforms_[GLProgram::UNIFORM_RANDOM01], UTILS::RANDOM::RANDOM_0_1(), UTILS::RANDOM::RANDOM_0_1(), UTILS::RANDOM::RANDOM_0_1(), UTILS::RANDOM::RANDOM_0_1());
    }

    void GLProgram::reset() {
        vertShader_ = fragShader_ = 0;
        memset(builtInUniforms_, 0, sizeof(builtInUniforms_));

        program_ = 0;

        for (auto e: hashForUniforms_) {
            free(e.second.first);
        }

        hashForUniforms_.clear();
    }

    enum
        {
            kShaderType_PositionTextureColor,
            kShaderType_PositionTextureColor_noMVP,
            kShaderType_PositionTextureColorAlphaTest,
            kShaderType_PositionTextureColorAlphaTestNoMV,
            kShaderType_PositionColor,
            kShaderType_PositionColorTextureAsPointsize,
            kShaderType_PositionColor_noMVP,
            kShaderType_PositionTexture,
            kShaderType_PositionTexture_uColor,
            kShaderType_PositionTextureA8Color,
            kShaderType_Position_uColor,
            kShaderType_PositionLengthTexureColor,
            kShaderType_LabelDistanceFieldNormal,
            kShaderType_LabelDistanceFieldGlow,
            kShaderType_UIGrayScale,
            kShaderType_LabelNormal,
            kShaderType_LabelOutline,
            kShaderType_MAX,
        };

        static GLProgramCache *_sharedGLProgramCache = 0;

        GLProgramCache* GLProgramCache::getInstance()
        {
            if (!_sharedGLProgramCache) {
                _sharedGLProgramCache = new (std::nothrow) GLProgramCache();
                if (!_sharedGLProgramCache->init())
                {
                    SAFE_DELETE(_sharedGLProgramCache);
                }
            }
            return _sharedGLProgramCache;
        }

        void GLProgramCache::destroyInstance()
        {
            SAFE_RELEASE_NULL(_sharedGLProgramCache);
        }

        GLProgramCache::GLProgramCache()
            : programs_() {

        }

        GLProgramCache::~GLProgramCache() {
            for( auto it = programs_.begin(); it != programs_.end(); ++it ) {
                (it->second)->release();
            }
        }

        bool GLProgramCache::init() {
            loadDefaultGLPrograms();
            return true;
        }

        void GLProgramCache::loadDefaultGLPrograms() {
            GLProgram *p = new (std::nothrow) GLProgram();
            loadDefaultGLProgram(p, kShaderType_PositionTextureColor);
            programs_.insert( std::make_pair( GLProgram::SHADER_NAME_POSITION_TEXTURE_COLOR, p ) );

            p = new (std::nothrow) GLProgram();
            loadDefaultGLProgram(p, kShaderType_PositionTextureColor_noMVP);
            programs_.insert( std::make_pair( GLProgram::SHADER_NAME_POSITION_TEXTURE_COLOR_NO_MVP, p ) );

            p = new (std::nothrow) GLProgram();
            loadDefaultGLProgram(p, kShaderType_PositionTextureColorAlphaTest);
            programs_.insert( std::make_pair(GLProgram::SHADER_NAME_POSITION_TEXTURE_ALPHA_TEST, p) );

            p = new (std::nothrow) GLProgram();
            loadDefaultGLProgram(p, kShaderType_PositionTextureColorAlphaTestNoMV);
            programs_.insert( std::make_pair(GLProgram::SHADER_NAME_POSITION_TEXTURE_ALPHA_TEST_NO_MV, p) );

            p = new (std::nothrow) GLProgram();
            loadDefaultGLProgram(p, kShaderType_PositionColor);
            programs_.insert( std::make_pair(GLProgram::SHADER_NAME_POSITION_COLOR, p) );

            p = new (std::nothrow) GLProgram();
            loadDefaultGLProgram(p, kShaderType_PositionColorTextureAsPointsize);
            programs_.insert( std::make_pair(GLProgram::SHADER_NAME_POSITION_COLOR_TEXASPOINTSIZE, p) );

            p = new (std::nothrow) GLProgram();
            loadDefaultGLProgram(p, kShaderType_PositionColor_noMVP);
            programs_.insert( std::make_pair(GLProgram::SHADER_NAME_POSITION_COLOR_NO_MVP, p) );

            p = new (std::nothrow) GLProgram();
            loadDefaultGLProgram(p, kShaderType_PositionTexture);
            programs_.insert( std::make_pair( GLProgram::SHADER_NAME_POSITION_TEXTURE, p) );

            p = new (std::nothrow) GLProgram();
            loadDefaultGLProgram(p, kShaderType_PositionTexture_uColor);
            programs_.insert( std::make_pair( GLProgram::SHADER_NAME_POSITION_TEXTURE_U_COLOR, p) );

            p = new (std::nothrow) GLProgram();
            loadDefaultGLProgram(p, kShaderType_PositionTextureA8Color);
            programs_.insert( std::make_pair(GLProgram::SHADER_NAME_POSITION_TEXTURE_A8_COLOR, p) );

            p = new (std::nothrow) GLProgram();
            loadDefaultGLProgram(p, kShaderType_Position_uColor);
            programs_.insert( std::make_pair(GLProgram::SHADER_NAME_POSITION_U_COLOR, p) );

            p = new (std::nothrow) GLProgram();
            loadDefaultGLProgram(p, kShaderType_PositionLengthTexureColor);
            programs_.insert( std::make_pair(GLProgram::SHADER_NAME_POSITION_LENGTH_TEXTURE_COLOR, p) );

            p = new (std::nothrow) GLProgram();
            loadDefaultGLProgram(p, kShaderType_LabelDistanceFieldNormal);
            programs_.insert( std::make_pair(GLProgram::SHADER_NAME_LABEL_DISTANCEFIELD_NORMAL, p) );

            p = new (std::nothrow) GLProgram();
            loadDefaultGLProgram(p, kShaderType_LabelDistanceFieldGlow);
            programs_.insert( std::make_pair(GLProgram::SHADER_NAME_LABEL_DISTANCEFIELD_GLOW, p) );

            p = new (std::nothrow) GLProgram();
            loadDefaultGLProgram(p, kShaderType_UIGrayScale);
            programs_.insert(std::make_pair(GLProgram::SHADER_NAME_POSITION_GRAYSCALE, p));

            p = new (std::nothrow) GLProgram();
            loadDefaultGLProgram(p, kShaderType_LabelNormal);
            programs_.insert( std::make_pair(GLProgram::SHADER_NAME_LABEL_NORMAL, p) );

            p = new (std::nothrow) GLProgram();
            loadDefaultGLProgram(p, kShaderType_LabelOutline);
            programs_.insert( std::make_pair(GLProgram::SHADER_NAME_LABEL_OUTLINE, p) );
        }

        void GLProgramCache::reloadDefaultGLPrograms() {
            GLProgram *p = getGLProgram(GLProgram::SHADER_NAME_POSITION_TEXTURE_COLOR);
            p->reset();
            loadDefaultGLProgram(p, kShaderType_PositionTextureColor);

            p = getGLProgram(GLProgram::SHADER_NAME_POSITION_TEXTURE_COLOR_NO_MVP);
            p->reset();
            loadDefaultGLProgram(p, kShaderType_PositionTextureColor_noMVP);

            p = getGLProgram(GLProgram::SHADER_NAME_POSITION_TEXTURE_ALPHA_TEST);
            p->reset();
            loadDefaultGLProgram(p, kShaderType_PositionTextureColorAlphaTest);

            p = getGLProgram(GLProgram::SHADER_NAME_POSITION_TEXTURE_ALPHA_TEST_NO_MV);
            p->reset();
            loadDefaultGLProgram(p, kShaderType_PositionTextureColorAlphaTestNoMV);

            p = getGLProgram(GLProgram::SHADER_NAME_POSITION_COLOR);
            p->reset();
            loadDefaultGLProgram(p, kShaderType_PositionColor);

            p = getGLProgram(GLProgram::SHADER_NAME_POSITION_COLOR_TEXASPOINTSIZE);
            p->reset();
            loadDefaultGLProgram(p, kShaderType_PositionColorTextureAsPointsize);

            p = getGLProgram(GLProgram::SHADER_NAME_POSITION_COLOR_NO_MVP);
            loadDefaultGLProgram(p, kShaderType_PositionColor_noMVP);

            p = getGLProgram(GLProgram::SHADER_NAME_POSITION_TEXTURE);
            p->reset();
            loadDefaultGLProgram(p, kShaderType_PositionTexture);

            p = getGLProgram(GLProgram::SHADER_NAME_POSITION_TEXTURE_U_COLOR);
            p->reset();
            loadDefaultGLProgram(p, kShaderType_PositionTexture_uColor);

            p = getGLProgram(GLProgram::SHADER_NAME_POSITION_TEXTURE_A8_COLOR);
            p->reset();
            loadDefaultGLProgram(p, kShaderType_PositionTextureA8Color);

            p = getGLProgram(GLProgram::SHADER_NAME_POSITION_U_COLOR);
            p->reset();
            loadDefaultGLProgram(p, kShaderType_Position_uColor);

            p = getGLProgram(GLProgram::SHADER_NAME_POSITION_LENGTH_TEXTURE_COLOR);
            p->reset();
            loadDefaultGLProgram(p, kShaderType_PositionLengthTexureColor);

            p = getGLProgram(GLProgram::SHADER_NAME_LABEL_DISTANCEFIELD_NORMAL);
            p->reset();
            loadDefaultGLProgram(p, kShaderType_LabelDistanceFieldNormal);

            p = getGLProgram(GLProgram::SHADER_NAME_LABEL_DISTANCEFIELD_GLOW);
            p->reset();
            loadDefaultGLProgram(p, kShaderType_LabelDistanceFieldGlow);

            p = getGLProgram(GLProgram::SHADER_NAME_LABEL_NORMAL);
            p->reset();
            loadDefaultGLProgram(p, kShaderType_LabelNormal);

            p = getGLProgram(GLProgram::SHADER_NAME_LABEL_OUTLINE);
            p->reset();
            loadDefaultGLProgram(p, kShaderType_LabelOutline);
        }

        void GLProgramCache::loadDefaultGLProgram(GLProgram *p, int type) {
            switch (type) {
                case kShaderType_PositionTextureColor:
                    p->initWithByteArrays(PositionTextureColor_vert, PositionTextureColor_frag);
                    break;
                case kShaderType_PositionTextureColor_noMVP:
                    p->initWithByteArrays(PositionTextureColor_noMVP_vert, PositionTextureColor_noMVP_frag);
                    break;
                case kShaderType_PositionTextureColorAlphaTest:
                    p->initWithByteArrays(PositionTextureColor_vert, PositionTextureColorAlphaTest_frag);
                    break;
                case kShaderType_PositionTextureColorAlphaTestNoMV:
                    p->initWithByteArrays(PositionTextureColor_noMVP_vert, PositionTextureColorAlphaTest_frag);
                    break;
                case kShaderType_PositionColor:
                    p->initWithByteArrays(PositionColor_vert ,PositionColor_frag);
                    break;
                case kShaderType_PositionColorTextureAsPointsize:
                    p->initWithByteArrays(PositionColorTextureAsPointsize_vert ,PositionColor_frag);
                    break;
                case kShaderType_PositionColor_noMVP:
                    p->initWithByteArrays(PositionTextureColor_noMVP_vert ,PositionColor_frag);
                    break;
                case kShaderType_PositionTexture:
                    p->initWithByteArrays(PositionTexture_vert ,PositionTexture_frag);
                    break;
                case kShaderType_PositionTexture_uColor:
                    p->initWithByteArrays(PositionTexture_uColor_vert, PositionTexture_uColor_frag);
                    break;
                case kShaderType_PositionTextureA8Color:
                    p->initWithByteArrays(PositionTextureA8Color_vert, PositionTextureA8Color_frag);
                    break;
                case kShaderType_Position_uColor:
                    p->initWithByteArrays(Position_uColor_vert, Position_uColor_frag);
                    p->bindAttribLocation("aVertex", GLProgram::VERTEX_ATTRIB_POSITION);
                    break;
                case kShaderType_PositionLengthTexureColor:
                    p->initWithByteArrays(PositionColorLengthTexture_vert, PositionColorLengthTexture_frag);
                    break;
                case kShaderType_LabelDistanceFieldNormal:
                    p->initWithByteArrays(Label_vert, LabelDistanceFieldNormal_frag);
                    break;
                case kShaderType_LabelDistanceFieldGlow:
                    p->initWithByteArrays(Label_vert, LabelDistanceFieldGlow_frag);
                    break;
                case kShaderType_UIGrayScale:
                    p->initWithByteArrays(PositionTextureColor_noMVP_vert,
                                          PositionTexture_GrayScale_frag);
                    break;
                case kShaderType_LabelNormal:
                    p->initWithByteArrays(Label_vert, LabelNormal_frag);
                    break;
                case kShaderType_LabelOutline:
                    p->initWithByteArrays(Label_vert, LabelOutline_frag);
                    break;
                default:
                    return;
            }

            p->link();
            p->updateUniforms();
        }

        GLProgram* GLProgramCache::getGLProgram(const std::string &key) {
            auto it = programs_.find(key);
            if( it != programs_.end() )
                return it->second;
            return nullptr;
        }

        void GLProgramCache::addGLProgram(GLProgram* program, const std::string &key) {
            auto prev = getGLProgram(key);
            if( prev == program )
                return;

            programs_.erase(key);
            SAFE_RELEASE_NULL(prev);

            if (program)
                program->retain();
            programs_[key] = program;
        }
}
