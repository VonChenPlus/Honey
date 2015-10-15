#include "GLShaderState.h"
#include "GRAPH/UNITY3D/GLShader.h"
#include "GRAPH/UNITY3D/GLStateCache.h"

namespace GRAPH
{
    UniformValue::UniformValue()
        : uniform_(nullptr)
        , glShader_(nullptr)
        , type_(Type::VALUE) {
    }

    UniformValue::UniformValue(Uniform *uniform, GLShader* glShader)
        : uniform_(uniform)
        , glShader_(glShader)
        , type_(Type::VALUE) {
    }

    UniformValue::~UniformValue() {
        if (type_ == Type::CALLBACK_FN)
            delete value_.callback;
    }

    void UniformValue::apply() {
        if (type_ == Type::CALLBACK_FN) {
            (*value_.callback)(glShader_, uniform_);
        }
        else if (type_ == Type::POINTER) {
            switch (uniform_->type) {
                case GL_FLOAT:
                    glShader_->setUniformLocationWith1fv(uniform_->location, value_.floatv.pointer, value_.floatv.size);
                    break;

                case GL_FLOAT_VEC2:
                    glShader_->setUniformLocationWith2fv(uniform_->location, value_.v2f.pointer, value_.v2f.size);
                    break;

                case GL_FLOAT_VEC3:
                    glShader_->setUniformLocationWith3fv(uniform_->location, value_.v3f.pointer, value_.v3f.size);
                    break;

                case GL_FLOAT_VEC4:
                    glShader_->setUniformLocationWith4fv(uniform_->location, value_.v4f.pointer, value_.v4f.size);
                    break;

                default:
                    break;
            }
        }
        else{
            switch (uniform_->type) {
                case GL_SAMPLER_2D:
                    glShader_->setUniformLocationWith1i(uniform_->location, value_.tex.textureUnit);
                    GLStateCache::BindTexture2DN(value_.tex.textureUnit, value_.tex.textureId);
                    break;

                case GL_SAMPLER_CUBE:
                    glShader_->setUniformLocationWith1i(uniform_->location, value_.tex.textureUnit);
                    GLStateCache::BindTextureN(value_.tex.textureUnit, value_.tex.textureId, GL_TEXTURE_CUBE_MAP);
                    break;

                case GL_INT:
                    glShader_->setUniformLocationWith1i(uniform_->location, value_.intValue);
                    break;

                case GL_FLOAT:
                    glShader_->setUniformLocationWith1f(uniform_->location, value_.floatValue);
                    break;

                case GL_FLOAT_VEC2:
                    glShader_->setUniformLocationWith2f(uniform_->location, value_.v2Value[0], value_.v2Value[1]);
                    break;

                case GL_FLOAT_VEC3:
                    glShader_->setUniformLocationWith3f(uniform_->location, value_.v3Value[0], value_.v3Value[1], value_.v3Value[2]);
                    break;

                case GL_FLOAT_VEC4:
                    glShader_->setUniformLocationWith4f(uniform_->location, value_.v4Value[0], value_.v4Value[1], value_.v4Value[2], value_.v4Value[3]);
                    break;

                case GL_FLOAT_MAT4:
                    glShader_->setUniformLocationWithMatrix4fv(uniform_->location, (GLfloat*)&value_.matrixValue, 1);
                    break;

                default:
                    break;
            }
        }
    }

    void UniformValue::setCallback(const std::function<void(GLShader*, Uniform*)> &callback) {
        if (type_ == Type::CALLBACK_FN)
            delete value_.callback;

        value_.callback = new std::function<void(GLShader*, Uniform*)>();
        *value_.callback = callback;

        type_ = Type::CALLBACK_FN;
    }

    void UniformValue::setTexture(GLuint textureId, GLuint textureUnit) {
        value_.tex.textureId = textureId;
        value_.tex.textureUnit = textureUnit;
        type_ = Type::VALUE;
    }

    void UniformValue::setInt(int value) {
        value_.intValue = value;
        type_ = Type::VALUE;
    }

    void UniformValue::setFloat(float value) {
        value_.floatValue = value;
        type_ = Type::VALUE;
    }

    void UniformValue::setFloatv(int64 size, const float* pointer) {
        value_.floatv.pointer = (const float*)pointer;
        value_.floatv.size = (GLsizei)size;
        type_ = Type::POINTER;
    }

    void UniformValue::setVec2(const MATH::Vector2f& value) {
        memcpy(value_.v2Value, &value, sizeof(value_.v2Value));
        type_ = Type::VALUE;
    }

    void UniformValue::setVec2v(int64 size, const MATH::Vector2f* pointer) {
        value_.v2f.pointer = (const float*)pointer;
        value_.v2f.size = (GLsizei)size;
        type_ = Type::POINTER;
    }

    void UniformValue::setVec3(const MATH::Vector3f& value) {
        memcpy(value_.v3Value, &value, sizeof(value_.v3Value));
        type_ = Type::VALUE;

    }

    void UniformValue::setVec3v(int64 size, const MATH::Vector3f* pointer) {
        value_.v3f.pointer = (const float*)pointer;
        value_.v3f.size = (GLsizei)size;
        type_ = Type::POINTER;

    }

    void UniformValue::setVec4(const MATH::Vector4f& value) {
        memcpy(value_.v4Value, &value, sizeof(value_.v4Value));
        type_ = Type::VALUE;
    }

    void UniformValue::setVec4v(int64 size, const MATH::Vector4f* pointer) {
        value_.v4f.pointer = (const float*)pointer;
        value_.v4f.size = (GLsizei)size;
        type_ = Type::POINTER;
    }

    void UniformValue::setMat4(const MATH::Matrix4& value) {
        memcpy(value_.matrixValue, &value, sizeof(value_.matrixValue));
        type_ = Type::VALUE;
    }

    VertexAttribValue::VertexAttribValue()
        : vertexAttrib_(nullptr)
        , useCallback_(false)
        , enabled_(false) {
    }

    VertexAttribValue::VertexAttribValue(VertexAttrib *vertexAttrib)
        : vertexAttrib_(vertexAttrib)
        , useCallback_(false)
        , enabled_(false) {
    }

    VertexAttribValue::~VertexAttribValue() {
        if (useCallback_)
            delete value_.callback;
    }

    void VertexAttribValue::apply() {
        if(enabled_) {
            if(useCallback_) {
                (*value_.callback)(vertexAttrib_);
            }
            else {
                glVertexAttribPointer(vertexAttrib_->index,
                                      value_.pointer.size,
                                      value_.pointer.type,
                                      value_.pointer.normalized,
                                      value_.pointer.stride,
                                      value_.pointer.pointer);
            }
        }
    }

    void VertexAttribValue::setCallback(const std::function<void(VertexAttrib*)> &callback) {
        value_.callback = new std::function<void(VertexAttrib*)>();
        *value_.callback = callback;
        useCallback_ = true;
        enabled_ = true;
    }

    void VertexAttribValue::setPointer(GLint size, GLenum type, GLboolean normalized, GLsizei stride, GLvoid *pointer) {
        value_.pointer.size = size;
        value_.pointer.type = type;
        value_.pointer.normalized = normalized;
        value_.pointer.stride = stride;
        value_.pointer.pointer = pointer;
        enabled_ = true;
    }

    GLShaderState* GLShaderState::create(GLShader *glShader) {
        GLShaderState* ret = nullptr;
        ret = new (std::nothrow) GLShaderState();
        if(ret && ret->init(glShader))
        {
            ret->autorelease();
            return ret;
        }
        SAFE_DELETE(ret);
        return nullptr;
    }

    GLShaderState* GLShaderState::getOrCreateWithGLShaderName(const std::string& glShaderName ) {
        GLShader *glShader = GLShaderCache::getInstance().getGLShader(glShaderName);
        if( glShader )
            return getOrCreateWithGLShader(glShader);

        return nullptr;
    }

    GLShaderState* GLShaderState::getOrCreateWithGLShader(GLShader *glShader) {
        GLShaderState* ret = GLShaderStateCache::getInstance().getGLShaderState(glShader);
        return ret;
    }

    GLShaderState* GLShaderState::getOrCreateWithShaders(const std::string& vertexShader, const std::string& fragShader, const std::string& compileTimeDefines) {
        const std::string key = vertexShader + "+" + fragShader + "+" + compileTimeDefines;
        auto glShader = GLShaderCache::getInstance().getGLShader(key);

        if (!glShader) {
            glShader = GLShader::createWithFilenames(vertexShader, fragShader, compileTimeDefines);
            GLShaderCache::getInstance().addGLShader(glShader, key);
        }

        return create(glShader);
    }

    GLShaderState::GLShaderState()
        : uniformAttributeValueDirty_(true)
        , textureUnitIndex_(4)  // first 4 textures unites are reserved for _Texture0-3
        , vertexAttribsFlags_(0)
        , glShader_(nullptr) {
    }

    GLShaderState::~GLShaderState() {
        SAFE_RELEASE(glShader_);
    }

    bool GLShaderState::init(GLShader* glShader) {
        glShader_ = glShader;
        glShader_->retain();

        for(auto &attrib : glShader_->vertexAttribs_) {
            VertexAttribValue value(&attrib.second);
            attributes_[attrib.first] = value;
        }

        for(auto &uniform : glShader_->userUniforms_) {
            UniformValue value(&uniform.second, glShader_);
            uniforms_[uniform.second.location] = value;
            uniformsByName_[uniform.first] = uniform.second.location;
        }

        return true;
    }

    void GLShaderState::resetGLShader() {
        SAFE_RELEASE(glShader_);
        glShader_ = nullptr;
        uniforms_.clear();
        attributes_.clear();
        textureUnitIndex_ = 1;
    }

    void GLShaderState::apply(const MATH::Matrix4& modelView) {
        applyGLShader(modelView);
        applyAttributes();
        applyUniforms();
    }

    void GLShaderState::updateUniformsAndAttributes() {
        if(uniformAttributeValueDirty_) {
            for(auto& uniformLocation : uniformsByName_) {
                uniforms_[uniformLocation.second].uniform_ = glShader_->getUniform(uniformLocation.first);
            }

            vertexAttribsFlags_ = 0;
            for(auto& attributeValue : attributes_) {
                attributeValue.second.vertexAttrib_ = glShader_->getVertexAttrib(attributeValue.first);;
                if(attributeValue.second.enabled_)
                    vertexAttribsFlags_ |= 1 << attributeValue.second.vertexAttrib_->index;
            }

            uniformAttributeValueDirty_ = false;

        }
    }

    void GLShaderState::applyGLShader(const MATH::Matrix4& modelView) {
        updateUniformsAndAttributes();
        glShader_->use();
        glShader_->setUniformsForBuiltins(modelView);
    }

    void GLShaderState::applyAttributes(bool applyAttribFlags) {
        updateUniformsAndAttributes();
        if(vertexAttribsFlags_) {
            if (applyAttribFlags)
                GLStateCache::EnableVertexAttribs(vertexAttribsFlags_);
            for(auto &attribute : attributes_) {
                attribute.second.apply();
            }
        }
    }
    void GLShaderState::applyUniforms() {
        updateUniformsAndAttributes();
        for(auto& uniform : uniforms_) {
            uniform.second.apply();
        }
    }

    void GLShaderState::setGLShader(GLShader *glShader) {
        if( glShader_ != glShader) {
            resetGLShader();
            init(glShader);
        }
    }

    uint32_t GLShaderState::getVertexAttribsFlags() const {
        return vertexAttribsFlags_;
    }

    int64 GLShaderState::getVertexAttribCount() const {
        return attributes_.size();
    }

    UniformValue* GLShaderState::getUniformValue(GLint uniformLocation) {
        updateUniformsAndAttributes();
        const auto itr = uniforms_.find(uniformLocation);
        if (itr != uniforms_.end())
            return &itr->second;
        return nullptr;
    }

    UniformValue* GLShaderState::getUniformValue(const std::string& name) {
        updateUniformsAndAttributes();
        const auto itr = uniformsByName_.find(name);
        if (itr != uniformsByName_.end())
            return &uniforms_[itr->second];
        return nullptr;
    }

    VertexAttribValue* GLShaderState::getVertexAttribValue(const std::string& name) {
        updateUniformsAndAttributes();
        const auto itr = attributes_.find(name);
        if( itr != attributes_.end())
            return &itr->second;
        return nullptr;
    }

    void GLShaderState::setVertexAttribCallback(const std::string& name, const std::function<void(VertexAttrib*)> &callback) {
        VertexAttribValue *v = getVertexAttribValue(name);
        if(v) {
            v->setCallback(callback);
            vertexAttribsFlags_ |= 1 << v->vertexAttrib_->index;
        }
    }

    void GLShaderState::setVertexAttribPointer(const std::string& name, GLint size, GLenum type, GLboolean normalized, GLsizei stride, GLvoid *pointer) {
        auto v = getVertexAttribValue(name);
        if(v) {
            v->setPointer(size, type, normalized, stride, pointer);
            vertexAttribsFlags_ |= 1 << v->vertexAttrib_->index;
        }
    }

    void GLShaderState::setUniformCallback(const std::string& uniformName, const std::function<void(GLShader*, Uniform*)> &callback) {
        auto v = getUniformValue(uniformName);
        if (v)
            v->setCallback(callback);
    }

    void GLShaderState::setUniformCallback(GLint uniformLocation, const std::function<void(GLShader*, Uniform*)> &callback) {
        auto v = getUniformValue(uniformLocation);
        if (v)
            v->setCallback(callback);
    }

    void GLShaderState::setUniformFloat(const std::string& uniformName, float value) {
        auto v = getUniformValue(uniformName);
        if (v)
            v->setFloat(value);
    }

    void GLShaderState::setUniformFloat(GLint uniformLocation, float value) {
        auto v = getUniformValue(uniformLocation);
        if (v)
            v->setFloat(value);
    }

    void GLShaderState::setUniformInt(const std::string& uniformName, int value) {
        auto v = getUniformValue(uniformName);
        if(v)
            v->setInt(value);
    }

    void GLShaderState::setUniformInt(GLint uniformLocation, int value) {
        auto v = getUniformValue(uniformLocation);
        if (v)
            v->setInt(value);

    }

    void GLShaderState::setUniformFloatv(const std::string& uniformName, int64 size, const float* pointer) {
        auto v = getUniformValue(uniformName);
        if (v)
            v->setFloatv(size, pointer);
    }

    void GLShaderState::setUniformFloatv(GLint uniformLocation, int64 size, const float* pointer) {
        auto v = getUniformValue(uniformLocation);
        if (v)
            v->setFloatv(size, pointer);
    }

    void GLShaderState::setUniformVec2(const std::string& uniformName, const MATH::Vector2f& value) {
        auto v = getUniformValue(uniformName);
        if (v)
            v->setVec2(value);
    }

    void GLShaderState::setUniformVec2(GLint uniformLocation, const MATH::Vector2f& value) {
        auto v = getUniformValue(uniformLocation);
        if (v)
            v->setVec2(value);
    }

    void GLShaderState::setUniformVec2v(const std::string& uniformName, int64 size, const MATH::Vector2f* pointer) {
        auto v = getUniformValue(uniformName);
        if (v)
            v->setVec2v(size, pointer);
    }

    void GLShaderState::setUniformVec2v(GLint uniformLocation, int64 size, const MATH::Vector2f* pointer) {
        auto v = getUniformValue(uniformLocation);
        if (v)
            v->setVec2v(size, pointer);
    }

    void GLShaderState::setUniformVec3(const std::string& uniformName, const MATH::Vector3f& value) {
        auto v = getUniformValue(uniformName);
        if (v)
            v->setVec3(value);
    }

    void GLShaderState::setUniformVec3(GLint uniformLocation, const MATH::Vector3f& value) {
        auto v = getUniformValue(uniformLocation);
        if (v)
            v->setVec3(value);
    }

    void GLShaderState::setUniformVec3v(const std::string& uniformName, int64 size, const MATH::Vector3f* pointer) {
        auto v = getUniformValue(uniformName);
        if (v)
            v->setVec3v(size, pointer);
    }

    void GLShaderState::setUniformVec3v(GLint uniformLocation, int64 size, const MATH::Vector3f* pointer) {
        auto v = getUniformValue(uniformLocation);
        if (v)
            v->setVec3v(size, pointer);
    }

    void GLShaderState::setUniformVec4(const std::string& uniformName, const MATH::Vector4f& value) {
        auto v = getUniformValue(uniformName);
        if (v)
            v->setVec4(value);
    }

    void GLShaderState::setUniformVec4(GLint uniformLocation, const MATH::Vector4f& value) {
        auto v = getUniformValue(uniformLocation);
        if (v)
            v->setVec4(value);
    }

    void GLShaderState::setUniformVec4v(const std::string& uniformName, int64 size, const MATH::Vector4f* value) {
        auto v = getUniformValue(uniformName);
        if (v)
            v->setVec4v(size, value);
    }

    void GLShaderState::setUniformVec4v(GLint uniformLocation, int64 size, const MATH::Vector4f* pointer) {
        auto v = getUniformValue(uniformLocation);
        if (v)
            v->setVec4v(size, pointer);
    }

    void GLShaderState::setUniformMat4(const std::string& uniformName, const MATH::Matrix4& value) {
        auto v = getUniformValue(uniformName);
        if (v)
            v->setMat4(value);
    }

    void GLShaderState::setUniformMat4(GLint uniformLocation, const MATH::Matrix4& value) {
        auto v = getUniformValue(uniformLocation);
        if (v)
            v->setMat4(value);
    }

    void GLShaderState::setUniformTexture(const std::string& uniformName, GLuint textureId) {
        auto v = getUniformValue(uniformName);
        if (v) {
            if (boundTextureUnits_.find(uniformName) != boundTextureUnits_.end()) {
                v->setTexture(textureId, boundTextureUnits_[uniformName]);
            }
            else {
                v->setTexture(textureId, textureUnitIndex_);
                boundTextureUnits_[uniformName] = textureUnitIndex_++;
            }
        }
    }

    void GLShaderState::setUniformTexture(GLint uniformLocation, GLuint textureId) {
        auto v = getUniformValue(uniformLocation);
        if (v) {
            if (boundTextureUnits_.find(v->uniform_->name) != boundTextureUnits_.end()) {
                v->setTexture(textureId, boundTextureUnits_[v->uniform_->name]);
            }
            else {
                v->setTexture(textureId, textureUnitIndex_);
                boundTextureUnits_[v->uniform_->name] = textureUnitIndex_++;
            }
        }
    }

    GLShaderStateCache::GLShaderStateCache() {
    }

    GLShaderStateCache::~GLShaderStateCache() {
        glShaderStates_.clear();
    }

    GLShaderStateCache& GLShaderStateCache::getInstance() {
        static GLShaderStateCache instance;
        return instance;
    }

    GLShaderState* GLShaderStateCache::getGLShaderState(GLShader* glShader) {
        const auto& itr = glShaderStates_.find(glShader);
        if (itr != glShaderStates_.end()) {
            return itr->second;
        }

        auto ret = new (std::nothrow) GLShaderState;
        if(ret && ret->init(glShader)) {
            glShaderStates_.insert(glShader, ret);
            ret->release();
            return ret;
        }

        SAFE_RELEASE(ret);
        return ret;
    }

    void GLShaderStateCache::removeUnusedGLShaderState() {
        for( auto it=glShaderStates_.cbegin(); it != glShaderStates_.cend(); /* nothing */) {
            auto value = it->second;
            if( value->getReferenceCount() == 1 ) {
                glShaderStates_.erase(it++);
            }
            else {
                ++it;
            }
        }
    }

    void GLShaderStateCache::removeAllGLShaderState() {
        glShaderStates_.clear();
    }
}
