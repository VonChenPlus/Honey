#include "GRAPH/UNITY3D/ShaderState.h"
#include "GRAPH/UNITY3D/ShaderCache.h"

namespace GRAPH
{
    UniformValue::UniformValue()
        : uniformForamt_(nullptr)
        , type_(Type::VALUE) {
    }

    UniformValue::UniformValue(U3DUniform *uniform, Unity3DShaderSet* u3dShader)
        : uniform_(uniform)
        , uniformForamt_(Unity3DCreator::CreateUniformFormat(u3dShader, U3DuniformComponent(uniform->location, uniform->type)))
        , type_(Type::VALUE) {
    }

    UniformValue::~UniformValue() {
        SAFE_RELEASE(uniformForamt_);
    }

    void UniformValue::apply() {
        uniformForamt_->component().location = uniform_->location;
        uniformForamt_->component().type = uniform_->type;
        if (type_ == Type::POINTER) {
            uniformForamt_->applyArray();
        }
        else{
            uniformForamt_->applyValue();
        }
    }

    void UniformValue::setTexture(uint32 textureId, uint32 textureUnit) {
        uniformForamt_->component().value.tex.textureId = textureId;
        uniformForamt_->component().value.tex.textureUnit = textureUnit;
        type_ = Type::VALUE;
    }

    void UniformValue::setInt(int value) {
        uniformForamt_->component().value.intValue = value;
        type_ = Type::VALUE;
    }

    void UniformValue::setFloat(float value) {
        uniformForamt_->component().value.floatValue = value;
        type_ = Type::VALUE;
    }

    void UniformValue::setFloatv(int32 size, const float* pointer) {
        uniformForamt_->component().value.floatv.pointer = (const float*) pointer;
        uniformForamt_->component().value.floatv.size = size;
        type_ = Type::POINTER;
    }

    void UniformValue::setVec2(const MATH::Vector2f& value) {
        memcpy(uniformForamt_->component().value.v2Value, &value, sizeof(uniformForamt_->component().value.v2Value));
        type_ = Type::VALUE;
    }

    void UniformValue::setVec2v(int32 size, const MATH::Vector2f* pointer) {
        uniformForamt_->component().value.v2f.pointer = (const float*) pointer;
        uniformForamt_->component().value.v2f.size = size;
        type_ = Type::POINTER;
    }

    void UniformValue::setVec3(const MATH::Vector3f& value) {
        memcpy(uniformForamt_->component().value.v3Value, &value, sizeof(uniformForamt_->component().value.v3Value));
        type_ = Type::VALUE;

    }

    void UniformValue::setVec3v(int32 size, const MATH::Vector3f* pointer) {
        uniformForamt_->component().value.v3f.pointer = (const float*) pointer;
        uniformForamt_->component().value.v3f.size = size;
        type_ = Type::POINTER;

    }

    void UniformValue::setVec4(const MATH::Vector4f& value) {
        memcpy(uniformForamt_->component().value.v4Value, &value, sizeof(uniformForamt_->component().value.v4Value));
        type_ = Type::VALUE;
    }

    void UniformValue::setVec4v(int32 size, const MATH::Vector4f* pointer) {
        uniformForamt_->component().value.v4f.pointer = (const float*) pointer;
        uniformForamt_->component().value.v4f.size = size;
        type_ = Type::POINTER;
    }

    void UniformValue::setMat4(const MATH::Matrix4& value) {
        memcpy(uniformForamt_->component().value.matrixValue, &value, sizeof(uniformForamt_->component().value.matrixValue));
        type_ = Type::VALUE;
    }

    VertexAttribValue::VertexAttribValue()
        : vertexFormat_(nullptr)
        , enabled_(false) {
    }

    VertexAttribValue::VertexAttribValue(uint8 semantic)
        : vertexFormat_(nullptr)
        , enabled_(false) {
        vertexFormat_ = Unity3DCreator::CreateVertexFormat(U3DVertexComponent((U3DSemantic) semantic, CUSTOM));
    }

    VertexAttribValue::~VertexAttribValue() {
        SAFE_RELEASE(vertexFormat_);
    }

    void VertexAttribValue::apply() {
        if(enabled_) {
            vertexFormat_->apply();
        }
    }

    void VertexAttribValue::setPointer(uint8 size, U3DVertexDataType type, bool normalized, int stride, intptr pointer) {
        if (vertexFormat_->components().size() != 1) {
            throw _HException_Normal("UnException VertexFormat Component Number!");
        }
          
        vertexFormat_->components()[0].size = size;
        vertexFormat_->components()[0].type = type;
        vertexFormat_->components()[0].normalized = normalized;
        vertexFormat_->components()[0].stride = stride;
        vertexFormat_->components()[0].offset = pointer;
        enabled_ = true;
    }

    ShaderState* ShaderState::create(Unity3DShaderSet *u3dShader) {
        ShaderState* ret = nullptr;
        ret = new (std::nothrow) ShaderState();
        if(ret && ret->init(u3dShader))
        {
            ret->autorelease();
            return ret;
        }
        SAFE_DELETE(ret);
        return nullptr;
    }

    ShaderState* ShaderState::getOrCreateWithShaderName(const std::string& shaderName ) {
        Unity3DShaderSet *u3dShader = ShaderCache::getInstance().getU3DShader(shaderName);
        if( u3dShader )
            return getOrCreateWithShader(u3dShader);

        return nullptr;
    }

    ShaderState* ShaderState::getOrCreateWithShader(Unity3DShaderSet *u3dShader) {
        ShaderState* ret = ShaderStateCache::getInstance().getShaderState(u3dShader);
        return ret;
    }

    ShaderState* ShaderState::getOrCreateWithShaders(const std::string& vertexShader, const std::string& fragShader, const std::string& compileTimeDefines) {
        const std::string key = vertexShader + "+" + fragShader + "+" + compileTimeDefines;
        auto u3dShader = ShaderCache::getInstance().getU3DShader(key);

        if (!u3dShader) {
            u3dShader = Unity3DCreator::CreateShaderSetWithFileName(vertexShader, fragShader, compileTimeDefines);
            ShaderCache::getInstance().addU3DShader(u3dShader, key);
        }

        return create(u3dShader);
    }

    ShaderState::ShaderState()
        : uniformAttributeValueDirty_(true)
        , textureUnitIndex_(4)  // first 4 textures unites are reserved for _Texture0-3
        , u3dShader_(nullptr) {
    }

    ShaderState::~ShaderState() {
        SAFE_RELEASE(u3dShader_);
    }

    bool ShaderState::init(Unity3DShaderSet* u3dShader) {
        u3dShader_ = u3dShader;
        u3dShader_->retain();

        for(auto &attrib : u3dShader_->vertexAttribs()) {
            VertexAttribValue value(attrib.second.semantic);
            attributes_[attrib.first] = value;
        }

        for(auto &uniform : u3dShader_->userUniforms()) {
            UniformValue value(&uniform.second, u3dShader_);
            uniforms_[uniform.second.location] = value;
            uniformsByName_[uniform.first] = uniform.second.location;
        }

        return true;
    }

    void ShaderState::resetU3DShader() {
        SAFE_RELEASE(u3dShader_);
        u3dShader_ = nullptr;
        uniforms_.clear();
        attributes_.clear();
        textureUnitIndex_ = 1;
    }

    void ShaderState::apply(const MATH::Matrix4& modelView) {
        applyU3DShader(modelView);
        applyAttributes();
        applyUniforms();
    }

    void ShaderState::updateUniformsAndAttributes() {
        if(uniformAttributeValueDirty_) {
            for(auto& uniformLocation : uniformsByName_) {
                uniforms_[uniformLocation.second].uniform_ = u3dShader_->userUniform(uniformLocation.first);
            }

            for(auto& attributeValue : attributes_) {
                attributeValue.second.vertexFormat_->components()[0].semantic = u3dShader_->vertexAttrib(attributeValue.first)->semantic;
            }

            uniformAttributeValueDirty_ = false;
        }
    }

    void ShaderState::applyU3DShader(const MATH::Matrix4& modelView) {
        updateUniformsAndAttributes();
        u3dShader_->apply();
        u3dShader_->setUniformsForBuiltins(modelView);
    }

    void ShaderState::applyAttributes() {
        updateUniformsAndAttributes();
        for(auto &attribute : attributes_) {
            attribute.second.apply();
        }
    }
    void ShaderState::applyUniforms() {
        updateUniformsAndAttributes();
        for(auto& uniform : uniforms_) {
            uniform.second.apply();
        }
    }

    void ShaderState::setU3DShader(Unity3DShaderSet *u3dShader) {
        if( u3dShader_ != u3dShader) {
            resetU3DShader();
            init(u3dShader);
        }
    }

    UniformValue* ShaderState::getUniformValue(int32 uniformLocation) {
        updateUniformsAndAttributes();
        const auto itr = uniforms_.find(uniformLocation);
        if (itr != uniforms_.end())
            return &itr->second;
        return nullptr;
    }

    UniformValue* ShaderState::getUniformValue(const std::string& name) {
        updateUniformsAndAttributes();
        const auto itr = uniformsByName_.find(name);
        if (itr != uniformsByName_.end())
            return &uniforms_[itr->second];
        return nullptr;
    }

    VertexAttribValue* ShaderState::getVertexAttribValue(const std::string& name) {
        updateUniformsAndAttributes();
        const auto itr = attributes_.find(name);
        if( itr != attributes_.end())
            return &itr->second;
        return nullptr;
    }

    void ShaderState::setVertexAttribPointer(const std::string& name, uint8 size, U3DVertexDataType type, bool normalized, int stride, intptr pointer) {
        auto v = getVertexAttribValue(name);
        if(v) {
            v->setPointer(size, type, normalized, stride, pointer);
        }
    }

    void ShaderState::setUniformFloat(const std::string& uniformName, float value) {
        auto v = getUniformValue(uniformName);
        if (v)
            v->setFloat(value);
    }

    void ShaderState::setUniformFloat(int32 uniformLocation, float value) {
        auto v = getUniformValue(uniformLocation);
        if (v)
            v->setFloat(value);
    }

    void ShaderState::setUniformInt(const std::string& uniformName, int value) {
        auto v = getUniformValue(uniformName);
        if(v)
            v->setInt(value);
    }

    void ShaderState::setUniformInt(int32 uniformLocation, int value) {
        auto v = getUniformValue(uniformLocation);
        if (v)
            v->setInt(value);

    }

    void ShaderState::setUniformFloatv(const std::string& uniformName, int32 size, const float* pointer) {
        auto v = getUniformValue(uniformName);
        if (v)
            v->setFloatv(size, pointer);
    }

    void ShaderState::setUniformFloatv(int32 uniformLocation, int32 size, const float* pointer) {
        auto v = getUniformValue(uniformLocation);
        if (v)
            v->setFloatv(size, pointer);
    }

    void ShaderState::setUniformVec2(const std::string& uniformName, const MATH::Vector2f& value) {
        auto v = getUniformValue(uniformName);
        if (v)
            v->setVec2(value);
    }

    void ShaderState::setUniformVec2(int32 uniformLocation, const MATH::Vector2f& value) {
        auto v = getUniformValue(uniformLocation);
        if (v)
            v->setVec2(value);
    }

    void ShaderState::setUniformVec2v(const std::string& uniformName, int32 size, const MATH::Vector2f* pointer) {
        auto v = getUniformValue(uniformName);
        if (v)
            v->setVec2v(size, pointer);
    }

    void ShaderState::setUniformVec2v(int32 uniformLocation, int32 size, const MATH::Vector2f* pointer) {
        auto v = getUniformValue(uniformLocation);
        if (v)
            v->setVec2v(size, pointer);
    }

    void ShaderState::setUniformVec3(const std::string& uniformName, const MATH::Vector3f& value) {
        auto v = getUniformValue(uniformName);
        if (v)
            v->setVec3(value);
    }

    void ShaderState::setUniformVec3(int32 uniformLocation, const MATH::Vector3f& value) {
        auto v = getUniformValue(uniformLocation);
        if (v)
            v->setVec3(value);
    }

    void ShaderState::setUniformVec3v(const std::string& uniformName, int32 size, const MATH::Vector3f* pointer) {
        auto v = getUniformValue(uniformName);
        if (v)
            v->setVec3v(size, pointer);
    }

    void ShaderState::setUniformVec3v(int32 uniformLocation, int32 size, const MATH::Vector3f* pointer) {
        auto v = getUniformValue(uniformLocation);
        if (v)
            v->setVec3v(size, pointer);
    }

    void ShaderState::setUniformVec4(const std::string& uniformName, const MATH::Vector4f& value) {
        auto v = getUniformValue(uniformName);
        if (v)
            v->setVec4(value);
    }

    void ShaderState::setUniformVec4(int32 uniformLocation, const MATH::Vector4f& value) {
        auto v = getUniformValue(uniformLocation);
        if (v)
            v->setVec4(value);
    }

    void ShaderState::setUniformVec4v(const std::string& uniformName, int32 size, const MATH::Vector4f* value) {
        auto v = getUniformValue(uniformName);
        if (v)
            v->setVec4v(size, value);
    }

    void ShaderState::setUniformVec4v(int32 uniformLocation, int32 size, const MATH::Vector4f* pointer) {
        auto v = getUniformValue(uniformLocation);
        if (v)
            v->setVec4v(size, pointer);
    }

    void ShaderState::setUniformMat4(const std::string& uniformName, const MATH::Matrix4& value) {
        auto v = getUniformValue(uniformName);
        if (v)
            v->setMat4(value);
    }

    void ShaderState::setUniformMat4(int32 uniformLocation, const MATH::Matrix4& value) {
        auto v = getUniformValue(uniformLocation);
        if (v)
            v->setMat4(value);
    }

    void ShaderState::setUniformTexture(const std::string& uniformName, uint32 textureId) {
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

    void ShaderState::setUniformTexture(int32 uniformLocation, uint32 textureId) {
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

    ShaderStateCache::ShaderStateCache() {
    }

    ShaderStateCache::~ShaderStateCache() {
        shaderStates_.clear();
    }

    ShaderStateCache& ShaderStateCache::getInstance() {
        static ShaderStateCache instance;
        return instance;
    }

    ShaderState* ShaderStateCache::getShaderState(Unity3DShaderSet* u3dShader) {
        const auto& itr = shaderStates_.find(u3dShader);
        if (itr != shaderStates_.end()) {
            return itr->second;
        }

        auto ret = new (std::nothrow) ShaderState;
        if(ret && ret->init(u3dShader)) {
            shaderStates_.insert(u3dShader, ret);
            ret->release();
            return ret;
        }

        SAFE_RELEASE(ret);
        return ret;
    }

    void ShaderStateCache::removeUnusedShaderState() {
        for( auto it=shaderStates_.cbegin(); it != shaderStates_.cend(); /* nothing */) {
            auto value = it->second;
            if( value->getReferenceCount() == 1 ) {
                shaderStates_.erase(it++);
            }
            else {
                ++it;
            }
        }
    }

    void ShaderStateCache::removeAllShaderState() {
        shaderStates_.clear();
    }
}
