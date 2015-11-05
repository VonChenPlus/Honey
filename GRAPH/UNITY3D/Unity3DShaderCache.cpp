#include "GRAPH/UNITY3D/Unity3DShaderCache.h"
#include "GRAPH/UNITY3D/GLSL.h"

namespace GRAPH
{
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

    Unity3DShaderCache& Unity3DShaderCache::getInstance() {
        static Unity3DShaderCache instance;
        return instance;
    }

    Unity3DShaderCache::Unity3DShaderCache()
        : programs_() {
        init();
    }

    Unity3DShaderCache::~Unity3DShaderCache() {
        for (auto it = programs_.begin(); it != programs_.end(); ++it) {
            (it->second)->release();
        }
        programs_.clear();
    }

    bool Unity3DShaderCache::init() {
        loadDefaultShaders();
        return true;
    }

    void Unity3DShaderCache::loadDefaultShaders() {
        Unity3DShaderSet *p = Unity3DCreator::CreateShaderSetWithByteArray(PositionTextureColor_vert, PositionTextureColor_frag);
        programs_.insert(std::make_pair(Unity3DShader::SHADER_NAME_POSITION_TEXTURE_COLOR, p));

        p = Unity3DCreator::CreateShaderSetWithByteArray(PositionTextureColor_noMVP_vert, PositionTextureColor_noMVP_frag);
        programs_.insert(std::make_pair(Unity3DShader::SHADER_NAME_POSITION_TEXTURE_COLOR_NO_MVP, p));

        p = Unity3DCreator::CreateShaderSetWithByteArray(PositionTextureColor_vert, PositionTextureColorAlphaTest_frag);
        programs_.insert(std::make_pair(Unity3DShader::SHADER_NAME_POSITION_TEXTURE_ALPHA_TEST, p));

        p = Unity3DCreator::CreateShaderSetWithByteArray(PositionTextureColor_noMVP_vert, PositionTextureColorAlphaTest_frag);
        programs_.insert(std::make_pair(Unity3DShader::SHADER_NAME_POSITION_TEXTURE_ALPHA_TEST_NO_MV, p));

        p = Unity3DCreator::CreateShaderSetWithByteArray(PositionColor_vert, PositionColor_frag);
        programs_.insert(std::make_pair(Unity3DShader::SHADER_NAME_POSITION_COLOR, p));

        p = Unity3DCreator::CreateShaderSetWithByteArray(PositionColorTextureAsPointsize_vert, PositionColor_frag);
        programs_.insert(std::make_pair(Unity3DShader::SHADER_NAME_POSITION_COLOR_TEXASPOINTSIZE, p));

        p = Unity3DCreator::CreateShaderSetWithByteArray(PositionTextureColor_noMVP_vert, PositionColor_frag);
        programs_.insert(std::make_pair(Unity3DShader::SHADER_NAME_POSITION_COLOR_NO_MVP, p));

        p = Unity3DCreator::CreateShaderSetWithByteArray(PositionTexture_vert, PositionTexture_frag);
        programs_.insert(std::make_pair(Unity3DShader::SHADER_NAME_POSITION_TEXTURE, p));

        p = Unity3DCreator::CreateShaderSetWithByteArray(PositionTexture_uColor_vert, PositionTexture_uColor_frag);
        programs_.insert(std::make_pair(Unity3DShader::SHADER_NAME_POSITION_TEXTURE_U_COLOR, p));

        p = Unity3DCreator::CreateShaderSetWithByteArray(PositionTextureA8Color_vert, PositionTextureA8Color_frag);
        programs_.insert(std::make_pair(Unity3DShader::SHADER_NAME_POSITION_TEXTURE_A8_COLOR, p));

        p = Unity3DCreator::CreateShaderSetWithByteArray(Position_uColor_vert, Position_uColor_frag);
        p->bindAttribLocation("aVertex", SEM_POSITION);
        programs_.insert(std::make_pair(Unity3DShader::SHADER_NAME_POSITION_U_COLOR, p));

        p = Unity3DCreator::CreateShaderSetWithByteArray(PositionColorLengthTexture_vert, PositionColorLengthTexture_frag);
        programs_.insert(std::make_pair(Unity3DShader::SHADER_NAME_POSITION_LENGTH_TEXTURE_COLOR, p));

        p = Unity3DCreator::CreateShaderSetWithByteArray(Label_vert, LabelDistanceFieldNormal_frag);
        programs_.insert(std::make_pair(Unity3DShader::SHADER_NAME_LABEL_DISTANCEFIELD_NORMAL, p));

        p = Unity3DCreator::CreateShaderSetWithByteArray(Label_vert, LabelDistanceFieldGlow_frag);
        programs_.insert(std::make_pair(Unity3DShader::SHADER_NAME_LABEL_DISTANCEFIELD_GLOW, p));

        p = Unity3DCreator::CreateShaderSetWithByteArray(PositionTextureColor_noMVP_vert,
            PositionTexture_GrayScale_frag);
        programs_.insert(std::make_pair(Unity3DShader::SHADER_NAME_POSITION_GRAYSCALE, p));

        p = Unity3DCreator::CreateShaderSetWithByteArray(Label_vert, LabelNormal_frag);
        programs_.insert(std::make_pair(Unity3DShader::SHADER_NAME_LABEL_NORMAL, p));

        p = Unity3DCreator::CreateShaderSetWithByteArray(Label_vert, LabelOutline_frag);
        programs_.insert(std::make_pair(Unity3DShader::SHADER_NAME_LABEL_OUTLINE, p));
    }

    Unity3DShaderSet* Unity3DShaderCache::getU3DShader(const std::string &key) {
        auto it = programs_.find(key);
        if (it != programs_.end())
            return it->second;
        return nullptr;
    }

    void Unity3DShaderCache::addU3DShader(Unity3DShaderSet* program, const std::string &key) {
        auto prev = getU3DShader(key);
        if (prev == program)
            return;

        programs_.erase(key);
        SAFE_RELEASE_NULL(prev);

        if (program)
            program->retain();
        programs_[key] = program;
    }
}
