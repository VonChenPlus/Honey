#include "IO/FileUtils.h"
#include "GRAPH/Director.h"
#include "GRAPH/UNITY3D/Unity3DGLShader.h"
#include "GRAPH/UNITY3D/Unity3DGLState.h"
#include "GRAPH/UNITY3D/GLStateCache.h"
#include "UTILS/RANDOM/RandomUtils.h"
#include "UTILS/STRING/StringUtils.h"

namespace GRAPH
{
    // helper functions
    static void ReplaceDefines(const std::string& compileTimeDefines, std::string& out) {
        // Replace semicolons with '#define ... \n'
        if (compileTimeDefines.size() > 0) {
            uint64 pos;
            out = compileTimeDefines;
            out.insert(0, "#define ");
            while ((pos = out.find(';')) != std::string::npos) {
                out.replace(pos, 1, "\n#define ");
            }
            out += "\n";
        }
    }

    // uniform names
    const char* Unity3DGLShaderSet::UNIFORM_NAME_AMBIENT_COLOR = "_AmbientColor";
    const char* Unity3DGLShaderSet::UNIFORM_NAME_P_MATRIX = "_PMatrix";
    const char* Unity3DGLShaderSet::UNIFORM_NAME_MV_MATRIX = "_MVMatrix";
    const char* Unity3DGLShaderSet::UNIFORM_NAME_MVP_MATRIX = "_MVPMatrix";
    const char* Unity3DGLShaderSet::UNIFORM_NAME_NORMAL_MATRIX = "_NormalMatrix";
    const char* Unity3DGLShaderSet::UNIFORM_NAME_TIME = "_Time";
    const char* Unity3DGLShaderSet::UNIFORM_NAME_SIN_TIME = "_SinTime";
    const char* Unity3DGLShaderSet::UNIFORM_NAME_COS_TIME = "_CosTime";
    const char* Unity3DGLShaderSet::UNIFORM_NAME_RANDOM01 = "_Random01";
    const char* Unity3DGLShaderSet::UNIFORM_NAME_SAMPLER0 = "_Texture0";
    const char* Unity3DGLShaderSet::UNIFORM_NAME_SAMPLER1 = "_Texture1";
    const char* Unity3DGLShaderSet::UNIFORM_NAME_SAMPLER2 = "_Texture2";
    const char* Unity3DGLShaderSet::UNIFORM_NAME_SAMPLER3 = "_Texture3";
    const char* Unity3DGLShaderSet::UNIFORM_NAME_ALPHA_TEST_VALUE = "_alpha_value";

    // Attribute names
    const char* Unity3DGLShaderSet::ATTRIBUTE_NAME_COLOR = "a_color";
    const char* Unity3DGLShaderSet::ATTRIBUTE_NAME_POSITION = "a_position";
    const char* Unity3DGLShaderSet::ATTRIBUTE_NAME_TEX_COORD = "a_texCoord";
    const char* Unity3DGLShaderSet::ATTRIBUTE_NAME_TEX_COORD1 = "a_texCoord1";
    const char* Unity3DGLShaderSet::ATTRIBUTE_NAME_TEX_COORD2 = "a_texCoord2";
    const char* Unity3DGLShaderSet::ATTRIBUTE_NAME_TEX_COORD3 = "a_texCoord3";
    const char* Unity3DGLShaderSet::ATTRIBUTE_NAME_NORMAL = "a_normal";
    const char* Unity3DGLShaderSet::ATTRIBUTE_NAME_BLEND_WEIGHT = "a_blendWeight";
    const char* Unity3DGLShaderSet::ATTRIBUTE_NAME_BLEND_INDEX = "a_blendIndex";

    static const char * SHADER_UNIFORMS =
        "uniform mat4 _PMatrix;\n"
        "uniform mat4 _MVMatrix;\n"
        "uniform mat4 _MVPMatrix;\n"
        "uniform mat3 _NormalMatrix;\n"
        "uniform vec4 _Time;\n"
        "uniform vec4 _SinTime;\n"
        "uniform vec4 _CosTime;\n"
        "uniform vec4 _Random01;\n"
        "uniform sampler2D _Texture0;\n"
        "uniform sampler2D _Texture1;\n"
        "uniform sampler2D _Texture2;\n"
        "uniform sampler2D _Texture3;\n"
        "//INCLUDES END\n\n";

    static const std::string EMPTY_DEFINE;

    Unity3DGLShader::Unity3DGLShader(bool isFragmentShader)
        : shader_(0)
        , type_(0) {
        type_ = isFragmentShader ? GL_FRAGMENT_SHADER : GL_VERTEX_SHADER;
    }

    Unity3DGLShader::~Unity3DGLShader() {
        glDeleteShader(shader_);
    }

    void Unity3DGLShader::compile(const char *source, const char *compileTimeDefines) {
        shader_ = glCreateShader(type_);

        const GLchar *sources [] = {
            SHADER_UNIFORMS,
            compileTimeDefines,
            source };

        glShaderSource(shader_, sizeof(sources) / sizeof(*sources), sources, 0);
        glCompileShader(shader_);
        GLint success = 0;
        glGetShaderiv(shader_, GL_COMPILE_STATUS, &success);
        if (!success) {
            #define MAX_INFO_LOG_SIZE 2048
            GLchar infoLog[MAX_INFO_LOG_SIZE];
            GLsizei len = 0;
            glGetShaderInfoLog(shader_, MAX_INFO_LOG_SIZE, &len, infoLog);
            infoLog[len] = '\0';
            glDeleteShader(shader_);
            shader_ = 0;
            throw _HException_(UTILS::STRING::StringFromFormat("%s Shader compile error:\n%s", type_ == GL_FRAGMENT_SHADER ? "Fragment" : "Vertex", infoLog), HException::GRAPH);
        }
    }

    Unity3DGLShaderSet::Unity3DGLShaderSet()
        : program_(0)
        , vertShader_(0)
        , fragShader_(0) {
        memset(builtInUniforms_, 0, sizeof(builtInUniforms_));
    }

    Unity3DGLShaderSet::Unity3DGLShaderSet(Unity3DShader *vertShader, Unity3DShader *fragShader) 
        : program_(0)
        , vertShader_(dynamic_cast<Unity3DGLShader*>(vertShader))
        , fragShader_(dynamic_cast<Unity3DGLShader*>(fragShader)) {
        vertShader_->retain();
        fragShader_->retain();
        memset(builtInUniforms_, 0, sizeof(builtInUniforms_));
    }

    Unity3DGLShaderSet::~Unity3DGLShaderSet() {
        if (vertShader_) {
            vertShader_->release();
        }
        if (fragShader_) {
            fragShader_->release();
        }
        glDeleteProgram(program_);

        for (auto e : hashForUniforms_) {
            free(e.second.first);
        }
        hashForUniforms_.clear();
    }

    Unity3DGLShaderSet* Unity3DGLShaderSet::createWithByteArrays(const GLchar* vShaderByteArray, const GLchar* fShaderByteArray) {
        return createWithByteArrays(vShaderByteArray, fShaderByteArray, EMPTY_DEFINE);
    }

    Unity3DGLShaderSet* Unity3DGLShaderSet::createWithByteArrays(const GLchar* vShaderByteArray, const GLchar* fShaderByteArray, const std::string& compileTimeDefines) {
        auto ret = new (std::nothrow) Unity3DGLShaderSet();
        if (ret && ret->initWithByteArrays(vShaderByteArray, fShaderByteArray, compileTimeDefines)) {
            ret->link();
            ret->updateUniforms();
            ret->autorelease();
            return ret;
        }

        SAFE_DELETE(ret);
        return nullptr;
    }

    bool Unity3DGLShaderSet::initWithByteArrays(const GLchar* vShaderByteArray, const GLchar* fShaderByteArray) {
        return initWithByteArrays(vShaderByteArray, fShaderByteArray, EMPTY_DEFINE);
    }

    bool Unity3DGLShaderSet::initWithByteArrays(const GLchar* vShaderByteArray, const GLchar* fShaderByteArray, const std::string& compileTimeDefines) {
        program_ = glCreateProgram();

        std::string replacedDefines = "";
        ReplaceDefines(compileTimeDefines, replacedDefines);

        vertShader_ = fragShader_ = 0;

        if (vShaderByteArray) {
            vertShader_ = new Unity3DGLShader(false);
            vertShader_->compile(vShaderByteArray, replacedDefines.c_str());
        }

        if (fShaderByteArray) {
            fragShader_ = new Unity3DGLShader(true);
            fragShader_->compile(fShaderByteArray, replacedDefines.c_str());
        }

        if (vertShader_) {
            glAttachShader(program_, vertShader_->getShader());
        }

        if (fragShader_) {
            glAttachShader(program_, fragShader_->getShader());
        }

        hashForUniforms_.clear();

        return true;
    }

    Unity3DGLShaderSet* Unity3DGLShaderSet::createWithFilenames(const std::string& vShaderFilename, const std::string& fShaderFilename) {
        return createWithFilenames(vShaderFilename, fShaderFilename, EMPTY_DEFINE);
    }

    Unity3DGLShaderSet* Unity3DGLShaderSet::createWithFilenames(const std::string& vShaderFilename, const std::string& fShaderFilename, const std::string& compileTimeDefines) {
        auto ret = new (std::nothrow) Unity3DGLShaderSet();
        if (ret && ret->initWithFilenames(vShaderFilename, fShaderFilename, compileTimeDefines)) {
            ret->link();
            ret->updateUniforms();
            ret->autorelease();
            return ret;
        }

        SAFE_DELETE(ret);
        return nullptr;
    }

    bool Unity3DGLShaderSet::initWithFilenames(const std::string& vShaderFilename, const std::string& fShaderFilename) {
        return initWithFilenames(vShaderFilename, fShaderFilename, EMPTY_DEFINE);
    }

    bool Unity3DGLShaderSet::initWithFilenames(const std::string& vShaderFilename, const std::string& fShaderFilename, const std::string& compileTimeDefines) {
        std::string vertexSource = IO::FileUtils::getInstance().getStringFromFile(IO::FileUtils::getInstance().fullPathForFilename(vShaderFilename));
        std::string fragmentSource = IO::FileUtils::getInstance().getStringFromFile(IO::FileUtils::getInstance().fullPathForFilename(fShaderFilename));

        return initWithByteArrays(vertexSource.c_str(), fragmentSource.c_str(), compileTimeDefines);
    }

    void Unity3DGLShaderSet::link() {
        program_ = glCreateProgram();
        glAttachShader(program_, vertShader_->getShader());
        glAttachShader(program_, fragShader_->getShader());

        bindPredefinedVertexAttribs();

        glLinkProgram(program_);

        parseVertexAttribs();
        parseUniforms();

        GLint linkStatus = GL_FALSE;
        glGetProgramiv(program_, GL_LINK_STATUS, &linkStatus);
        if (linkStatus != GL_TRUE) {
            throw _HException_("Could not link program", HException::GRAPH);
        }
    }

    void Unity3DGLShaderSet::apply() {
        Unity3DGLState::OpenGLState().useProgram.set(program_);
    }

    void Unity3DGLShaderSet::unApply() {
        Unity3DGLState::OpenGLState().useProgram.set(0);
    }

    void Unity3DGLShaderSet::reset() {
        vertShader_ = fragShader_ = 0;
        memset(builtInUniforms_, 0, sizeof(builtInUniforms_));

        program_ = 0;

        for (auto e : hashForUniforms_) {
            free(e.second.first);
        }

        hashForUniforms_.clear();
    }

    int32 Unity3DGLShaderSet::getAttribLocation(const std::string &attributeName) const {
        return glGetAttribLocation(program_, attributeName.c_str());
    }

    int32 Unity3DGLShaderSet::getUniformLocation(const std::string &attributeName) const {
        return glGetUniformLocation(program_, attributeName.c_str());
    }

    void Unity3DGLShaderSet::bindAttribLocation(const std::string &attributeName, uint32 index) const {
        glBindAttribLocation(program_, index, attributeName.c_str());
    }

    void Unity3DGLShaderSet::updateUniforms() {
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

        apply();

        // Since sample most probably won't change, set it to 0,1,2,3 now.
        if (builtInUniforms_[UNIFORM_SAMPLER0] != -1)
            setUniformLocationWith1i(builtInUniforms_[UNIFORM_SAMPLER0], 0);
        if (builtInUniforms_[UNIFORM_SAMPLER1] != -1)
            setUniformLocationWith1i(builtInUniforms_[UNIFORM_SAMPLER1], 1);
        if (builtInUniforms_[UNIFORM_SAMPLER2] != -1)
            setUniformLocationWith1i(builtInUniforms_[UNIFORM_SAMPLER2], 2);
        if (builtInUniforms_[UNIFORM_SAMPLER3] != -1)
            setUniformLocationWith1i(builtInUniforms_[UNIFORM_SAMPLER3], 3);
    }

    void Unity3DGLShaderSet::setUniformLocationWith1i(int location, int i1) {
        bool updated = updateUniformLocation(location, &i1, sizeof(i1) * 1);

        if (updated)
        {
            glUniform1i((GLint) location, i1);
        }
    }

    void Unity3DGLShaderSet::setUniformLocationWith2i(int location, int i1, int i2) {
        GLint ints[2] = { i1, i2 };
        bool updated = updateUniformLocation(location, ints, sizeof(ints));

        if (updated) {
            glUniform2i((GLint) location, i1, i2);
        }
    }

    void Unity3DGLShaderSet::setUniformLocationWith3i(int location, int i1, int i2, int i3) {
        GLint ints[3] = { i1, i2, i3 };
        bool updated = updateUniformLocation(location, ints, sizeof(ints));

        if (updated) {
            glUniform3i((GLint) location, i1, i2, i3);
        }
    }

    void Unity3DGLShaderSet::setUniformLocationWith4i(int location, int i1, int i2, int i3, int i4) {
        GLint ints[4] = { i1, i2, i3, i4 };
        bool updated = updateUniformLocation(location, ints, sizeof(ints));

        if (updated) {
            glUniform4i((GLint) location, i1, i2, i3, i4);
        }
    }

    void Unity3DGLShaderSet::setUniformLocationWith2iv(int location, int* ints, unsigned int numberOfArrays) {
        bool updated = updateUniformLocation(location, ints, sizeof(int) * 2 * numberOfArrays);

        if (updated) {
            glUniform2iv((GLint) location, (GLsizei) numberOfArrays, ints);
        }
    }

    void Unity3DGLShaderSet::setUniformLocationWith3iv(int location, int* ints, unsigned int numberOfArrays) {
        bool updated = updateUniformLocation(location, ints, sizeof(int) * 3 * numberOfArrays);

        if (updated) {
            glUniform3iv((GLint) location, (GLsizei) numberOfArrays, ints);
        }
    }

    void Unity3DGLShaderSet::setUniformLocationWith4iv(int location, int* ints, unsigned int numberOfArrays) {
        bool updated = updateUniformLocation(location, ints, sizeof(int) * 4 * numberOfArrays);

        if (updated) {
            glUniform4iv((GLint) location, (GLsizei) numberOfArrays, ints);
        }
    }

    void Unity3DGLShaderSet::setUniformLocationWith1f(int location, float f1) {
        bool updated = updateUniformLocation(location, &f1, sizeof(f1) * 1);

        if (updated) {
            glUniform1f((GLint) location, f1);
        }
    }

    void Unity3DGLShaderSet::setUniformLocationWith2f(int location, float f1, float f2) {
        float floats[2] = { f1, f2 };
        bool updated = updateUniformLocation(location, floats, sizeof(floats));

        if (updated) {
            glUniform2f((GLint) location, f1, f2);
        }
    }

    void Unity3DGLShaderSet::setUniformLocationWith3f(int location, float f1, float f2, float f3) {
        float floats[3] = { f1, f2, f3 };
        bool updated = updateUniformLocation(location, floats, sizeof(floats));

        if (updated) {
            glUniform3f((GLint) location, f1, f2, f3);
        }
    }

    void Unity3DGLShaderSet::setUniformLocationWith4f(int location, float f1, float f2, float f3, float f4) {
        float floats[4] = { f1, f2, f3, f4 };
        bool updated = updateUniformLocation(location, floats, sizeof(floats));

        if (updated) {
            glUniform4f((GLint) location, f1, f2, f3, f4);
        }
    }

    void Unity3DGLShaderSet::setUniformLocationWith1fv(int location, const float* floats, unsigned int numberOfArrays) {
        bool updated = updateUniformLocation(location, floats, sizeof(float)*numberOfArrays);

        if (updated) {
            glUniform1fv((GLint) location, (GLsizei) numberOfArrays, floats);
        }
    }

    void Unity3DGLShaderSet::setUniformLocationWith2fv(int location, const float* floats, unsigned int numberOfArrays) {
        bool updated = updateUniformLocation(location, floats, sizeof(float) * 2 * numberOfArrays);

        if (updated) {
            glUniform2fv((GLint) location, (GLsizei) numberOfArrays, floats);
        }
    }

    void Unity3DGLShaderSet::setUniformLocationWith3fv(int location, const float* floats, unsigned int numberOfArrays) {
        bool updated = updateUniformLocation(location, floats, sizeof(float) * 3 * numberOfArrays);

        if (updated) {
            glUniform3fv((GLint) location, (GLsizei) numberOfArrays, floats);
        }
    }

    void Unity3DGLShaderSet::setUniformLocationWith4fv(int location, const float* floats, unsigned int numberOfArrays) {
        bool updated = updateUniformLocation(location, floats, sizeof(float) * 4 * numberOfArrays);

        if (updated) {
            glUniform4fv((GLint) location, (GLsizei) numberOfArrays, floats);
        }
    }

    void Unity3DGLShaderSet::setUniformLocationWithMatrix2fv(int location, const float* matrixArray, unsigned int numberOfMatrices) {
        bool updated = updateUniformLocation(location, matrixArray, sizeof(float) * 4 * numberOfMatrices);

        if (updated) {
            glUniformMatrix2fv((GLint) location, (GLsizei) numberOfMatrices, GL_FALSE, matrixArray);
        }
    }

    void Unity3DGLShaderSet::setUniformLocationWithMatrix3fv(int location, const float* matrixArray, unsigned int numberOfMatrices) {
        bool updated = updateUniformLocation(location, matrixArray, sizeof(float) * 9 * numberOfMatrices);

        if (updated) {
            glUniformMatrix3fv((GLint) location, (GLsizei) numberOfMatrices, GL_FALSE, matrixArray);
        }
    }

    void Unity3DGLShaderSet::setUniformLocationWithMatrix4fv(int location, const float* matrixArray, unsigned int numberOfMatrices) {
        bool updated = updateUniformLocation(location, matrixArray, sizeof(float) * 16 * numberOfMatrices);

        if (updated) {
            glUniformMatrix4fv((GLint) location, (GLsizei) numberOfMatrices, GL_FALSE, matrixArray);
        }
    }

    void Unity3DGLShaderSet::setUniformsForBuiltins() {
        setUniformsForBuiltins(Director::getInstance().getMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW));
    }

    void Unity3DGLShaderSet::setUniformsForBuiltins(const MATH::Matrix4 &matrixMV) {
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
            normalMat[0] = mvInverse.m[0]; normalMat[1] = mvInverse.m[1]; normalMat[2] = mvInverse.m[2];
            normalMat[3] = mvInverse.m[4]; normalMat[4] = mvInverse.m[5]; normalMat[5] = mvInverse.m[6];
            normalMat[6] = mvInverse.m[8]; normalMat[7] = mvInverse.m[9]; normalMat[8] = mvInverse.m[10];
            setUniformLocationWithMatrix3fv(builtInUniforms_[UNIFORM_NORMAL_MATRIX], normalMat, 1);
        }

        if (uniformsFlags_.usesTime) {
            throw _HException_Normal("Unsupport GLShader the most accurate global time value.");
            float time = 0.0f;
            setUniformLocationWith4f(builtInUniforms_[Unity3DGLShaderSet::UNIFORM_TIME], time / 10.0, time, time * 2, time * 4);
            setUniformLocationWith4f(builtInUniforms_[Unity3DGLShaderSet::UNIFORM_SIN_TIME], time / 8.0, time / 4.0, time / 2.0, sinf(time));
            setUniformLocationWith4f(builtInUniforms_[Unity3DGLShaderSet::UNIFORM_COS_TIME], time / 8.0, time / 4.0, time / 2.0, cosf(time));
        }

        if (uniformsFlags_.usesRandom)
            setUniformLocationWith4f(builtInUniforms_[Unity3DGLShaderSet::UNIFORM_RANDOM01], UTILS::RANDOM::RANDOM_0_1(), UTILS::RANDOM::RANDOM_0_1(), UTILS::RANDOM::RANDOM_0_1(), UTILS::RANDOM::RANDOM_0_1());
    }

    void Unity3DGLShaderSet::bindPredefinedVertexAttribs() {
        static const struct {
            const char *attributeName;
            int location;
        } attribute_locations [] =
        {
            { Unity3DGLShaderSet::ATTRIBUTE_NAME_POSITION, SEM_POSITION },
            { Unity3DGLShaderSet::ATTRIBUTE_NAME_COLOR, SEM_COLOR0 },
            { Unity3DGLShaderSet::ATTRIBUTE_NAME_TEX_COORD, SEM_TEXCOORD0 },
            { Unity3DGLShaderSet::ATTRIBUTE_NAME_TEX_COORD1, SEM_TEXCOORD1 },
            { Unity3DGLShaderSet::ATTRIBUTE_NAME_TEX_COORD2, SEM_TEXCOORD2 },
            { Unity3DGLShaderSet::ATTRIBUTE_NAME_TEX_COORD3, SEM_TEXCOORD3 },
            { Unity3DGLShaderSet::ATTRIBUTE_NAME_NORMAL, SEM_NORMAL },
        };

        const int size = sizeof(attribute_locations) / sizeof(attribute_locations[0]);

        for (int i = 0; i<size; i++) {
            glBindAttribLocation(program_, attribute_locations[i].location, attribute_locations[i].attributeName);
        }
    }

    void Unity3DGLShaderSet::parseVertexAttribs() {
        GLint activeAttributes;
        GLint length;
        glGetProgramiv(program_, GL_ACTIVE_ATTRIBUTES, &activeAttributes);
        if (activeAttributes > 0) {
            U3DVertexAttrib attribute;

            glGetProgramiv(program_, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &length);
            if (length > 0) {
                GLchar* attribName = (GLchar*) alloca(length + 1);

                for (int i = 0; i < activeAttributes; ++i) {
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

    void Unity3DGLShaderSet::parseUniforms() {
        // Query and store uniforms from the program.
        GLint activeUniforms;
        glGetProgramiv(program_, GL_ACTIVE_UNIFORMS, &activeUniforms);
        if (activeUniforms > 0) {
            GLint length;
            glGetProgramiv(program_, GL_ACTIVE_UNIFORM_MAX_LENGTH, &length);
            if (length > 0) {
                U3DUniform uniform;

                GLchar* uniformName = (GLchar*) alloca(length + 1);

                for (int i = 0; i < activeUniforms; ++i) {
                    glGetActiveUniform(program_, i, length, nullptr, &uniform.size, &uniform.type, uniformName);
                    uniformName[length] = '\0';

                    if (strncmp("_", uniformName, 1) != 0) {
                        if (length > 3)
                        {
                            char* c = strrchr(uniformName, '[');
                            if (c)
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

    bool Unity3DGLShaderSet::updateUniformLocation(GLint location, const GLvoid* data, unsigned int bytes) {
        if (location < 0) {
            return false;
        }

        bool updated = true;

        auto element = hashForUniforms_.find(location);
        if (element == hashForUniforms_.end()) {
            GLvoid* value = malloc(bytes);
            memcpy(value, data, bytes);
            hashForUniforms_.insert(std::make_pair(location, std::make_pair(value, bytes)));
        }
        else {
            if (memcmp(element->second.first, data, bytes) == 0) {
                updated = false;
            }
            else {
                if (element->second.second < bytes) {
                    GLvoid* value = realloc(element->second.first, bytes);
                    memcpy(value, data, bytes);
                    hashForUniforms_[location] = std::make_pair(value, bytes);
                }
                else
                    memcpy(element->second.first, data, bytes);
            }
        }

        return updated;
    }
}
