#include "GRAPH/RENDERER/GLProgramState.h"
#include "GRAPH/RENDERER/Texture2D.h"
#include "GRAPH/BASE/Director.h"
#include "GRAPH/BASE/Camera.h"

namespace GRAPH
{
    // static vector with all the registered custom binding resolvers
    std::vector<GLProgramState::AutoBindingResolver*> GLProgramState::_customAutoBindingResolvers;

    //
    //
    // UniformValue
    //
    //

    UniformValue::UniformValue()
    : _uniform(nullptr)
    , _glprogram(nullptr)
    , _type(Type::VALUE)
    {
    }

    UniformValue::UniformValue(Uniform *uniform, GLProgram* glprogram)
    : _uniform(uniform)
    , _glprogram(glprogram)
    , _type(Type::VALUE)
    {
    }

    UniformValue::~UniformValue()
    {
        if (_type == Type::CALLBACK_FN)
            delete _value.callback;
    }

    void UniformValue::apply()
    {
        if (_type == Type::CALLBACK_FN)
        {
            (*_value.callback)(_glprogram, _uniform);
        }
        else if (_type == Type::POINTER)
        {
            switch (_uniform->type) {
                case GL_FLOAT:
                    _glprogram->setUniformLocationWith1fv(_uniform->location, _value.floatv.pointer, _value.floatv.size);
                    break;

                case GL_FLOAT_VEC2:
                    _glprogram->setUniformLocationWith2fv(_uniform->location, _value.v2f.pointer, _value.v2f.size);
                    break;

                case GL_FLOAT_VEC3:
                    _glprogram->setUniformLocationWith3fv(_uniform->location, _value.v3f.pointer, _value.v3f.size);
                    break;

                case GL_FLOAT_VEC4:
                    _glprogram->setUniformLocationWith4fv(_uniform->location, _value.v4f.pointer, _value.v4f.size);
                    break;

                default:
                    break;
            }
        }
        else /* _type == VALUE */
        {
            switch (_uniform->type) {
                case GL_SAMPLER_2D:
                    _glprogram->setUniformLocationWith1i(_uniform->location, _value.tex.textureUnit);
                    GL::bindTexture2DN(_value.tex.textureUnit, _value.tex.textureId);
                    break;

                case GL_SAMPLER_CUBE:
                    _glprogram->setUniformLocationWith1i(_uniform->location, _value.tex.textureUnit);
                    GL::bindTextureN(_value.tex.textureUnit, _value.tex.textureId, GL_TEXTURE_CUBE_MAP);
                    break;

                case GL_INT:
                    _glprogram->setUniformLocationWith1i(_uniform->location, _value.intValue);
                    break;

                case GL_FLOAT:
                    _glprogram->setUniformLocationWith1f(_uniform->location, _value.floatValue);
                    break;

                case GL_FLOAT_VEC2:
                    _glprogram->setUniformLocationWith2f(_uniform->location, _value.v2Value[0], _value.v2Value[1]);
                    break;

                case GL_FLOAT_VEC3:
                    _glprogram->setUniformLocationWith3f(_uniform->location, _value.v3Value[0], _value.v3Value[1], _value.v3Value[2]);
                    break;

                case GL_FLOAT_VEC4:
                    _glprogram->setUniformLocationWith4f(_uniform->location, _value.v4Value[0], _value.v4Value[1], _value.v4Value[2], _value.v4Value[3]);
                    break;

                case GL_FLOAT_MAT4:
                    _glprogram->setUniformLocationWithMatrix4fv(_uniform->location, (GLfloat*)&_value.matrixValue, 1);
                    break;

                default:
                    CCASSERT(false, "Invalid UniformValue");
                    break;
            }
        }
    }

    void UniformValue::setCallback(const std::function<void(GLProgram*, Uniform*)> &callback)
    {
        // delete previously set callback
        // TODO: memory will leak if the user does:
        //    value->setCallback();
        //    value->setFloat();
        if (_type == Type::CALLBACK_FN)
            delete _value.callback;

        _value.callback = new std::function<void(GLProgram*, Uniform*)>();
        *_value.callback = callback;

        _type = Type::CALLBACK_FN;
    }

    void UniformValue::setTexture(GLuint textureId, GLuint textureUnit)
    {
        //CCASSERT(_uniform->type == GL_SAMPLER_2D, "Wrong type. expecting GL_SAMPLER_2D");
        _value.tex.textureId = textureId;
        _value.tex.textureUnit = textureUnit;
        _type = Type::VALUE;
    }
    void UniformValue::setInt(int value)
    {
        _value.intValue = value;
        _type = Type::VALUE;
    }

    void UniformValue::setFloat(float value)
    {
        _value.floatValue = value;
        _type = Type::VALUE;
    }

    void UniformValue::setFloatv(ssize_t size, const float* pointer)
    {
        _value.floatv.pointer = (const float*)pointer;
        _value.floatv.size = (GLsizei)size;
        _type = Type::POINTER;
    }

    void UniformValue::setVec2(const Vec2& value)
    {
        memcpy(_value.v2Value, &value, sizeof(_value.v2Value));
        _type = Type::VALUE;
    }

    void UniformValue::setVec2v(ssize_t size, const Vec2* pointer)
    {
        _value.v2f.pointer = (const float*)pointer;
        _value.v2f.size = (GLsizei)size;
        _type = Type::POINTER;
    }

    void UniformValue::setVec3(const Vec3& value)
    {
        memcpy(_value.v3Value, &value, sizeof(_value.v3Value));
        _type = Type::VALUE;

    }

    void UniformValue::setVec3v(ssize_t size, const Vec3* pointer)
    {
        _value.v3f.pointer = (const float*)pointer;
        _value.v3f.size = (GLsizei)size;
        _type = Type::POINTER;

    }

    void UniformValue::setVec4(const Vec4& value)
    {
        memcpy(_value.v4Value, &value, sizeof(_value.v4Value));
        _type = Type::VALUE;
    }

    void UniformValue::setVec4v(ssize_t size, const Vec4* pointer)
    {
        _value.v4f.pointer = (const float*)pointer;
        _value.v4f.size = (GLsizei)size;
        _type = Type::POINTER;
    }

    void UniformValue::setMat4(const Mat4& value)
    {
        memcpy(_value.matrixValue, &value, sizeof(_value.matrixValue));
        _type = Type::VALUE;
    }

    //
    //
    // VertexAttribValue
    //
    //

    VertexAttribValue::VertexAttribValue()
    : _vertexAttrib(nullptr)
    , _useCallback(false)
    , _enabled(false)
    {
    }

    VertexAttribValue::VertexAttribValue(VertexAttrib *vertexAttrib)
    : _vertexAttrib(vertexAttrib)
    , _useCallback(false)
    , _enabled(false)
    {
    }

    VertexAttribValue::~VertexAttribValue()
    {
        if (_useCallback)
            delete _value.callback;
    }

    void VertexAttribValue::apply()
    {
        if(_enabled) {
            if(_useCallback) {
                (*_value.callback)(_vertexAttrib);
            }
            else
            {
                glVertexAttribPointer(_vertexAttrib->index,
                                      _value.pointer.size,
                                      _value.pointer.type,
                                      _value.pointer.normalized,
                                      _value.pointer.stride,
                                      _value.pointer.pointer);
            }
        }
    }

    void VertexAttribValue::setCallback(const std::function<void(VertexAttrib*)> &callback)
    {
        _value.callback = new std::function<void(VertexAttrib*)>();
        *_value.callback = callback;
        _useCallback = true;
        _enabled = true;
    }

    void VertexAttribValue::setPointer(GLint size, GLenum type, GLboolean normalized, GLsizei stride, GLvoid *pointer)
    {
        _value.pointer.size = size;
        _value.pointer.type = type;
        _value.pointer.normalized = normalized;
        _value.pointer.stride = stride;
        _value.pointer.pointer = pointer;
        _enabled = true;
    }

    //
    //
    // GLProgramState
    //
    //

    GLProgramState* GLProgramState::create(GLProgram *glprogram)
    {
        GLProgramState* ret = nullptr;
        ret = new (std::nothrow) GLProgramState();
        if(ret && ret->init(glprogram))
        {
            ret->autorelease();
            return ret;
        }
        SAFE_DELETE(ret);
        return nullptr;
    }

    GLProgramState* GLProgramState::getOrCreateWithGLProgramName(const std::string& glProgramName )
    {
        GLProgram *glProgram = GLProgramCache::getInstance()->getGLProgram(glProgramName);
        if( glProgram )
            return getOrCreateWithGLProgram(glProgram);

        return nullptr;
    }


    GLProgramState* GLProgramState::getOrCreateWithGLProgram(GLProgram *glprogram)
    {
        GLProgramState* ret = GLProgramStateCache::getInstance()->getGLProgramState(glprogram);
        return ret;
    }

    GLProgramState* GLProgramState::getOrCreateWithShaders(const std::string& vertexShader, const std::string& fragShader, const std::string& compileTimeDefines)
    {
        auto glprogramcache = GLProgramCache::getInstance();
        const std::string key = vertexShader + "+" + fragShader + "+" + compileTimeDefines;
        auto glprogram = glprogramcache->getGLProgram(key);

        if (!glprogram) {
            glprogram = GLProgram::createWithFilenames(vertexShader, fragShader, compileTimeDefines);
            glprogramcache->addGLProgram(glprogram, key);
        }

        return create(glprogram);
    }


    GLProgramState::GLProgramState()
    : _uniformAttributeValueDirty(true)
    , _textureUnitIndex(4)  // first 4 textures unites are reserved for CC_Texture0-3
    , _vertexAttribsFlags(0)
    , _glprogram(nullptr)
    , _nodeBinding(nullptr)
    {
    }

    GLProgramState::~GLProgramState()
    {
        SAFE_RELEASE(_glprogram);
    }

    GLProgramState* GLProgramState::clone() const
    {
        auto glprogramstate = new (std::nothrow) GLProgramState();

        // copy everything manually, instead of calling init since this is faster

        glprogramstate->_glprogram = this->_glprogram;
        SAFE_RETAIN(glprogramstate->_glprogram);

        glprogramstate->_attributes = this->_attributes;
        glprogramstate->_vertexAttribsFlags = this->_vertexAttribsFlags;

        // copy uniforms
        glprogramstate->_uniformsByName = this->_uniformsByName;
        glprogramstate->_uniforms = this->_uniforms;
        glprogramstate->_uniformAttributeValueDirty = this->_uniformAttributeValueDirty;

        // copy textures
        glprogramstate->_textureUnitIndex = this->_textureUnitIndex;
        glprogramstate->_boundTextureUnits = this->_boundTextureUnits;

        // _nodeBinding is null since a node can only have one state.
        // making the null explict to avoid possible bugs in the future
        glprogramstate->_nodeBinding = nullptr;

        // copy autobindings... rebound them once a target is set again
        glprogramstate->_autoBindings = _autoBindings;

        glprogramstate->autorelease();
        return glprogramstate;
    }

    bool GLProgramState::init(GLProgram* glprogram)
    {
        _glprogram = glprogram;
        _glprogram->retain();

        for(auto &attrib : _glprogram->_vertexAttribs) {
            VertexAttribValue value(&attrib.second);
            _attributes[attrib.first] = value;
        }

        for(auto &uniform : _glprogram->_userUniforms) {
            UniformValue value(&uniform.second, _glprogram);
            _uniforms[uniform.second.location] = value;
            _uniformsByName[uniform.first] = uniform.second.location;
        }

        return true;
    }

    void GLProgramState::resetGLProgram()
    {
        SAFE_RELEASE(_glprogram);
        _glprogram = nullptr;
        _uniforms.clear();
        _attributes.clear();
        // first texture is GL_TEXTURE1
        _textureUnitIndex = 1;
        _nodeBinding = nullptr;
    }

    void GLProgramState::apply(const Mat4& modelView)
    {
        applyGLProgram(modelView);

        applyAttributes();

        applyUniforms();
    }

    void GLProgramState::updateUniformsAndAttributes()
    {
        if(_uniformAttributeValueDirty)
        {
            for(auto& uniformLocation : _uniformsByName)
            {
                _uniforms[uniformLocation.second]._uniform = _glprogram->getUniform(uniformLocation.first);
            }

            _vertexAttribsFlags = 0;
            for(auto& attributeValue : _attributes)
            {
                attributeValue.second._vertexAttrib = _glprogram->getVertexAttrib(attributeValue.first);;
                if(attributeValue.second._enabled)
                    _vertexAttribsFlags |= 1 << attributeValue.second._vertexAttrib->index;
            }

            _uniformAttributeValueDirty = false;

        }
    }

    void GLProgramState::applyGLProgram(const Mat4& modelView)
    {
        updateUniformsAndAttributes();
        // set shader
        _glprogram->use();
        _glprogram->setUniformsForBuiltins(modelView);
    }

    void GLProgramState::applyAttributes(bool applyAttribFlags)
    {
        // Don't set attributes if they weren't set
        // Use Case: Auto-batching
        updateUniformsAndAttributes();
        if(_vertexAttribsFlags) {
            // enable/disable vertex attribs
            if (applyAttribFlags)
                GL::enableVertexAttribs(_vertexAttribsFlags);
            // set attributes
            for(auto &attribute : _attributes)
            {
                attribute.second.apply();
            }
        }
    }
    void GLProgramState::applyUniforms()
    {
        // set uniforms
        updateUniformsAndAttributes();
        for(auto& uniform : _uniforms) {
            uniform.second.apply();
        }
    }

    void GLProgramState::setGLProgram(GLProgram *glprogram)
    {
        if( _glprogram != glprogram) {
            resetGLProgram();
            init(glprogram);
        }
    }

    uint32_t GLProgramState::getVertexAttribsFlags() const
    {
        return _vertexAttribsFlags;
    }

    ssize_t GLProgramState::getVertexAttribCount() const
    {
        return _attributes.size();
    }

    UniformValue* GLProgramState::getUniformValue(GLint uniformLocation)
    {
        updateUniformsAndAttributes();
        const auto itr = _uniforms.find(uniformLocation);
        if (itr != _uniforms.end())
            return &itr->second;
        return nullptr;
    }

    UniformValue* GLProgramState::getUniformValue(const std::string& name)
    {
        updateUniformsAndAttributes();
        const auto itr = _uniformsByName.find(name);
        if (itr != _uniformsByName.end())
            return &_uniforms[itr->second];
        return nullptr;
    }

    VertexAttribValue* GLProgramState::getVertexAttribValue(const std::string& name)
    {
        updateUniformsAndAttributes();
        const auto itr = _attributes.find(name);
        if( itr != _attributes.end())
            return &itr->second;
        return nullptr;
    }

    // VertexAttrib Setters
    void GLProgramState::setVertexAttribCallback(const std::string& name, const std::function<void(VertexAttrib*)> &callback)
    {
        VertexAttribValue *v = getVertexAttribValue(name);
        if(v) {
            v->setCallback(callback);
            _vertexAttribsFlags |= 1 << v->_vertexAttrib->index;
        }
    }

    void GLProgramState::setVertexAttribPointer(const std::string& name, GLint size, GLenum type, GLboolean normalized, GLsizei stride, GLvoid *pointer)
    {
        auto v = getVertexAttribValue(name);
        if(v) {
            v->setPointer(size, type, normalized, stride, pointer);
            _vertexAttribsFlags |= 1 << v->_vertexAttrib->index;
        }
    }

    // Uniform Setters

    void GLProgramState::setUniformCallback(const std::string& uniformName, const std::function<void(GLProgram*, Uniform*)> &callback)
    {
        auto v = getUniformValue(uniformName);
        if (v)
            v->setCallback(callback);
    }

    void GLProgramState::setUniformCallback(GLint uniformLocation, const std::function<void(GLProgram*, Uniform*)> &callback)
    {
        auto v = getUniformValue(uniformLocation);
        if (v)
            v->setCallback(callback);
    }

    void GLProgramState::setUniformFloat(const std::string& uniformName, float value)
    {
        auto v = getUniformValue(uniformName);
        if (v)
            v->setFloat(value);
    }

    void GLProgramState::setUniformFloat(GLint uniformLocation, float value)
    {
        auto v = getUniformValue(uniformLocation);
        if (v)
            v->setFloat(value);
    }

    void GLProgramState::setUniformInt(const std::string& uniformName, int value)
    {
        auto v = getUniformValue(uniformName);
        if(v)
            v->setInt(value);
    }

    void GLProgramState::setUniformInt(GLint uniformLocation, int value)
    {
        auto v = getUniformValue(uniformLocation);
        if (v)
            v->setInt(value);

    }

    void GLProgramState::setUniformFloatv(const std::string& uniformName, ssize_t size, const float* pointer)
    {
        auto v = getUniformValue(uniformName);
        if (v)
            v->setFloatv(size, pointer);
    }

    void GLProgramState::setUniformFloatv(GLint uniformLocation, ssize_t size, const float* pointer)
    {
        auto v = getUniformValue(uniformLocation);
        if (v)
            v->setFloatv(size, pointer);
    }

    void GLProgramState::setUniformVec2(const std::string& uniformName, const Vec2& value)
    {
        auto v = getUniformValue(uniformName);
        if (v)
            v->setVec2(value);
    }

    void GLProgramState::setUniformVec2(GLint uniformLocation, const Vec2& value)
    {
        auto v = getUniformValue(uniformLocation);
        if (v)
            v->setVec2(value);
    }

    void GLProgramState::setUniformVec2v(const std::string& uniformName, ssize_t size, const Vec2* pointer)
    {
        auto v = getUniformValue(uniformName);
        if (v)
            v->setVec2v(size, pointer);
    }

    void GLProgramState::setUniformVec2v(GLint uniformLocation, ssize_t size, const Vec2* pointer)
    {
        auto v = getUniformValue(uniformLocation);
        if (v)
            v->setVec2v(size, pointer);
    }

    void GLProgramState::setUniformVec3(const std::string& uniformName, const Vec3& value)
    {
        auto v = getUniformValue(uniformName);
        if (v)
            v->setVec3(value);
    }

    void GLProgramState::setUniformVec3(GLint uniformLocation, const Vec3& value)
    {
        auto v = getUniformValue(uniformLocation);
        if (v)
            v->setVec3(value);
    }

    void GLProgramState::setUniformVec3v(const std::string& uniformName, ssize_t size, const Vec3* pointer)
    {
        auto v = getUniformValue(uniformName);
        if (v)
            v->setVec3v(size, pointer);
    }

    void GLProgramState::setUniformVec3v(GLint uniformLocation, ssize_t size, const Vec3* pointer)
    {
        auto v = getUniformValue(uniformLocation);
        if (v)
            v->setVec3v(size, pointer);
    }

    void GLProgramState::setUniformVec4(const std::string& uniformName, const Vec4& value)
    {
        auto v = getUniformValue(uniformName);
        if (v)
            v->setVec4(value);
    }

    void GLProgramState::setUniformVec4(GLint uniformLocation, const Vec4& value)
    {
        auto v = getUniformValue(uniformLocation);
        if (v)
            v->setVec4(value);
    }

    void GLProgramState::setUniformVec4v(const std::string& uniformName, ssize_t size, const Vec4* value)
    {
        auto v = getUniformValue(uniformName);
        if (v)
            v->setVec4v(size, value);
    }

    void GLProgramState::setUniformVec4v(GLint uniformLocation, ssize_t size, const Vec4* pointer)
    {
        auto v = getUniformValue(uniformLocation);
        if (v)
            v->setVec4v(size, pointer);
    }


    void GLProgramState::setUniformMat4(const std::string& uniformName, const Mat4& value)
    {
        auto v = getUniformValue(uniformName);
        if (v)
            v->setMat4(value);
    }

    void GLProgramState::setUniformMat4(GLint uniformLocation, const Mat4& value)
    {
        auto v = getUniformValue(uniformLocation);
        if (v)
            v->setMat4(value);
    }

    // Textures

    void GLProgramState::setUniformTexture(const std::string& uniformName, Texture2D *texture)
    {
        setUniformTexture(uniformName, texture->getName());
    }

    void GLProgramState::setUniformTexture(GLint uniformLocation, Texture2D *texture)
    {
        setUniformTexture(uniformLocation, texture->getName());
    }

    void GLProgramState::setUniformTexture(const std::string& uniformName, GLuint textureId)
    {
        auto v = getUniformValue(uniformName);
        if (v)
        {
            if (_boundTextureUnits.find(uniformName) != _boundTextureUnits.end())
            {
                v->setTexture(textureId, _boundTextureUnits[uniformName]);
            }
            else
            {
                v->setTexture(textureId, _textureUnitIndex);
                _boundTextureUnits[uniformName] = _textureUnitIndex++;
            }
        }
    }

    void GLProgramState::setUniformTexture(GLint uniformLocation, GLuint textureId)
    {
        auto v = getUniformValue(uniformLocation);
        if (v)
        {
            if (_boundTextureUnits.find(v->_uniform->name) != _boundTextureUnits.end())
            {
                v->setTexture(textureId, _boundTextureUnits[v->_uniform->name]);
            }
            else
            {
                v->setTexture(textureId, _textureUnitIndex);
                _boundTextureUnits[v->_uniform->name] = _textureUnitIndex++;
            }
        }
    }

    // Auto bindings
    void GLProgramState::setParameterAutoBinding(const std::string& uniformName, const std::string& autoBinding)
    {
        _autoBindings[uniformName] = autoBinding;

        if (_nodeBinding)
            applyAutoBinding(uniformName, autoBinding);
    }

    void GLProgramState::applyAutoBinding(const std::string& uniformName, const std::string& autoBinding)
    {
        // This code tries to replace GLProgram::setUniformsForBuiltins. But it is unfinished ATM.
        // The idea is that users will be able to use variables from cocos2d-x without hardcoding the
        // information on GLProgram and other objects.
        // Instead, the Cocos2d uniform variables will be callbacks.
        // As an example of how bad the current design is, the ModelView matrix is being passed from Node, to the Commands, to the GLProgram.
        // Instead, the GLProgramState should obtain it from its target.

        bool resolved = false;
        for (const auto resolver: _customAutoBindingResolvers)
        {
            resolved = resolver->resolveAutoBinding(this, _nodeBinding, uniformName, autoBinding);
            if (resolved)
                break;
        }

        if (!resolved)
        {
            // add cocos2d-x variables here like:
            // PROJECT_MATRIX
            // MODEL_MATRIX
            // MODEL_VIEW
            // MODEL_VIEW_PROJECTION
            // etc...
            //
            // and remove them from GLProgram::setUniformsForBuiltins
        }
    }

    void GLProgramState::setNodeBinding(Node* target)
    {
        // weak ref
        _nodeBinding = target;

        for (const auto autobinding: _autoBindings)
            applyAutoBinding(autobinding.first, autobinding.second);
    }

    Node* GLProgramState::getNodeBinding() const
    {
        return _nodeBinding;
    }

    //
    // MARK: AutoBindingResolver
    //
    GLProgramState::AutoBindingResolver::AutoBindingResolver()
    {
        _customAutoBindingResolvers.push_back(this);
    }

    GLProgramState::AutoBindingResolver::~AutoBindingResolver()
    {
        std::vector<GLProgramState::AutoBindingResolver*>::iterator itr = std::find(_customAutoBindingResolvers.begin(), _customAutoBindingResolvers.end(), this);
        if (itr != _customAutoBindingResolvers.end())
            _customAutoBindingResolvers.erase(itr);
    }

    GLProgramStateCache* GLProgramStateCache::s_instance = nullptr;

    GLProgramStateCache::GLProgramStateCache()
    {
    }

    GLProgramStateCache::~GLProgramStateCache()
    {
        _glProgramStates.clear();
    }

    GLProgramStateCache* GLProgramStateCache::getInstance()
    {
        if (s_instance == nullptr)
            s_instance = new (std::nothrow) GLProgramStateCache();

        return s_instance;
    }

    void GLProgramStateCache::destroyInstance()
    {
        SAFE_DELETE(s_instance);
    }

    GLProgramState* GLProgramStateCache::getGLProgramState(GLProgram* glprogram)
    {
        const auto& itr = _glProgramStates.find(glprogram);
        if (itr != _glProgramStates.end())
        {
            return itr->second;
        }

        auto ret = new (std::nothrow) GLProgramState;
        if(ret && ret->init(glprogram)) {
            _glProgramStates.insert(glprogram, ret);
            ret->release();
            return ret;
        }

        SAFE_RELEASE(ret);
        return ret;
    }

    void GLProgramStateCache::removeUnusedGLProgramState()
    {
        for( auto it=_glProgramStates.cbegin(); it!=_glProgramStates.cend(); /* nothing */) {
            auto value = it->second;
            if( value->getReferenceCount() == 1 ) {
                //value->release();
                _glProgramStates.erase(it++);
            } else {
                ++it;
            }
        }
    }

    void GLProgramStateCache::removeAllGLProgramState()
    {
        _glProgramStates.clear();
    }
}
