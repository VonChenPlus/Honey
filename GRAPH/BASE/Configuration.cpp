#include "GRAPH/BASE/Configuration.h"

namespace GRAPH
{
    extern const char* cocos2dVersion();

    Configuration* Configuration::s_sharedConfiguration = nullptr;

    Configuration::Configuration()
    : _maxTextureSize(0)
    , _maxModelviewStackDepth(0)
    , _supportsPVRTC(false)
    , _supportsETC1(false)
    , _supportsS3TC(false)
    , _supportsATITC(false)
    , _supportsNPOT(false)
    , _supportsBGRA8888(false)
    , _supportsDiscardFramebuffer(false)
    , _supportsShareableVAO(false)
    , _maxSamplesAllowed(0)
    , _maxTextureUnits(0)
    , _glExtensions(nullptr)
    , _maxDirLightInShader(1)
    , _maxPointLightInShader(1)
    , _maxSpotLightInShader(1)
    {
    }

    bool Configuration::init()
    {
        _valueDict["cocos2d.x.version"] = HValue(cocos2dVersion());


    #if CC_ENABLE_PROFILERS
        _valueDict["cocos2d.x.compiled_with_profiler"] = HValue(true);
    #else
        _valueDict["cocos2d.x.compiled_with_profiler"] = HValue(false);
    #endif

    #if CC_ENABLE_GL_STATE_CACHE == 0
        _valueDict["cocos2d.x.compiled_with_gl_state_cache"] = HValue(false);
    #else
        _valueDict["cocos2d.x.compiled_with_gl_state_cache"] = HValue(true);
    #endif

    #if COCOS2D_DEBUG
        _valueDict["cocos2d.x.build_type"] = HValue("DEBUG");
    #else
        _valueDict["cocos2d.x.build_type"] = HValue("RELEASE");
    #endif

        return true;
    }

    Configuration::~Configuration()
    {
    }

    void Configuration::gatherGPUInfo()
    {
        _valueDict["gl.vendor"] = HValue((const char*)glGetString(GL_VENDOR));
        _valueDict["gl.renderer"] = HValue((const char*)glGetString(GL_RENDERER));
        _valueDict["gl.version"] = HValue((const char*)glGetString(GL_VERSION));

        _glExtensions = (char *)glGetString(GL_EXTENSIONS);

        glGetIntegerv(GL_MAX_TEXTURE_SIZE, &_maxTextureSize);
        _valueDict["gl.max_texture_size"] = HValue((int)_maxTextureSize);

        glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &_maxTextureUnits);
        _valueDict["gl.max_texture_units"] = HValue((int)_maxTextureUnits);

        _supportsETC1 = checkForGLExtension("GL_OES_compressed_ETC1_RGB8_texture");
        _valueDict["gl.supports_ETC1"] = HValue(_supportsETC1);

        _supportsS3TC = checkForGLExtension("GL_EXT_texture_compression_s3tc");
        _valueDict["gl.supports_S3TC"] = HValue(_supportsS3TC);

        _supportsATITC = checkForGLExtension("GL_AMD_compressed_ATC_texture");
        _valueDict["gl.supports_ATITC"] = HValue(_supportsATITC);

        _supportsPVRTC = checkForGLExtension("GL_IMG_texture_compression_pvrtc");
        _valueDict["gl.supports_PVRTC"] = HValue(_supportsPVRTC);

        _supportsNPOT = true;
        _valueDict["gl.supports_NPOT"] = HValue(_supportsNPOT);

        _supportsBGRA8888 = checkForGLExtension("GL_IMG_texture_format_BGRA888");
        _valueDict["gl.supports_BGRA8888"] = HValue(_supportsBGRA8888);

        _supportsDiscardFramebuffer = checkForGLExtension("GL_EXT_discard_framebuffer");
        _valueDict["gl.supports_discard_framebuffer"] = HValue(_supportsDiscardFramebuffer);

        _supportsShareableVAO = checkForGLExtension("vertex_array_object");
        _valueDict["gl.supports_vertex_array_object"] = HValue(_supportsShareableVAO);
    }

    Configuration* Configuration::getInstance()
    {
        if (! s_sharedConfiguration)
        {
            s_sharedConfiguration = new (std::nothrow) Configuration();
            s_sharedConfiguration->init();
        }

        return s_sharedConfiguration;
    }

    void Configuration::destroyInstance()
    {
        SAFE_RELEASE_NULL(s_sharedConfiguration);
    }

    bool Configuration::checkForGLExtension(const std::string &searchName) const
    {
       return  (_glExtensions && strstr(_glExtensions, searchName.c_str() ) ) ? true : false;
    }

    //
    // getters for specific variables.
    // Mantained for backward compatiblity reasons only.
    //
    int Configuration::getMaxTextureSize() const
    {
        return _maxTextureSize;
    }

    int Configuration::getMaxModelviewStackDepth() const
    {
        return _maxModelviewStackDepth;
    }

    int Configuration::getMaxTextureUnits() const
    {
        return _maxTextureUnits;
    }

    bool Configuration::supportsNPOT() const
    {
        return _supportsNPOT;
    }

    bool Configuration::supportsPVRTC() const
    {
        return _supportsPVRTC;
    }

    bool Configuration::supportsETC() const
    {
        //GL_ETC1_RGB8_OES is not defined in old opengl version
    #ifdef GL_ETC1_RGB8_OES
        return _supportsETC1;
    #else
        return false;
    #endif
    }

    bool Configuration::supportsS3TC() const
    {
    #ifdef GL_EXT_texture_compression_s3tc
        return _supportsS3TC;
    #else
        return false;
    #endif
    }

    bool Configuration::supportsATITC() const
    {
        return _supportsATITC;
    }

    bool Configuration::supportsBGRA8888() const
    {
        return _supportsBGRA8888;
    }

    bool Configuration::supportsDiscardFramebuffer() const
    {
        return _supportsDiscardFramebuffer;
    }

    bool Configuration::supportsShareableVAO() const
    {
    #if CC_TEXTURE_ATLAS_USE_VAO
        return _supportsShareableVAO;
    #else
        return false;
    #endif
    }

    int Configuration::getMaxSupportDirLightInShader() const
    {
        return _maxDirLightInShader;
    }

    int Configuration::getMaxSupportPointLightInShader() const
    {
        return _maxPointLightInShader;
    }

    int Configuration::getMaxSupportSpotLightInShader() const
    {
        return _maxSpotLightInShader;
    }

    //
    // generic getters for properties
    //
    const HValue& Configuration::getValue(const std::string& key, const HValue& defaultValue) const
    {
        auto iter = _valueDict.find(key);
        if (iter != _valueDict.cend())
            return _valueDict.at(key);
        return defaultValue;
    }

    void Configuration::setValue(const std::string& key, const HValue& value)
    {
        _valueDict[key] = value;
    }


    //
    // load file
    //
    void Configuration::loadConfigFile(const std::string& filename)
    {
        ValueMap dict = FileUtils::getInstance()->getValueMapFromFile(filename);
        // search for metadata
        bool validMetadata = false;
        auto metadataIter = dict.find("metadata");
        if (metadataIter != dict.cend() && metadataIter->second.getType() == HValue::Type::MAP)
        {

            const auto& metadata = metadataIter->second.asValueMap();
            auto formatIter = metadata.find("format");

            if (formatIter != metadata.cend())
            {
                int format = formatIter->second.asInt();

                // Support format: 1
                if (format == 1)
                {
                    validMetadata = true;
                }
            }
        }

        if (! validMetadata)
        {
            return;
        }

        auto dataIter = dict.find("data");
        if (dataIter == dict.cend() || dataIter->second.getType() != HValue::Type::MAP)
        {
            return;
        }

        // Add all keys in the existing dictionary

        const auto& dataMap = dataIter->second.asValueMap();
        for (auto dataMapIter = dataMap.cbegin(); dataMapIter != dataMap.cend(); ++dataMapIter)
        {
            if (_valueDict.find(dataMapIter->first) == _valueDict.cend())
                _valueDict[dataMapIter->first] = dataMapIter->second;
        }

        //light info
        std::string name = "cocos2d.x.3d.max_dir_light_in_shader";
        if (_valueDict.find(name) != _valueDict.end())
            _maxDirLightInShader = _valueDict[name].asInt();
        else
            _valueDict[name] = HValue(_maxDirLightInShader);

        name = "cocos2d.x.3d.max_point_light_in_shader";
        if (_valueDict.find(name) != _valueDict.end())
            _maxPointLightInShader = _valueDict[name].asInt();
        else
            _valueDict[name] = HValue(_maxPointLightInShader);

        name = "cocos2d.x.3d.max_spot_light_in_shader";
        if (_valueDict.find(name) != _valueDict.end())
            _maxSpotLightInShader = _valueDict[name].asInt();
        else
            _valueDict[name] = HValue(_maxSpotLightInShader);
    }
}
