#include "GRAPH/RENDERER/Shaders.h"

namespace GRAPH
{
    #define STRINGIFY(A)  #A

    #include "SHADER/Shader_Position_uColor.frag"
    #include "SHADER/Shader_Position_uColor.vert"

    #include "SHADER/Shader_PositionColor.frag"
    #include "SHADER/Shader_PositionColor.vert"

    #include "SHADER/Shader_PositionColorTextureAsPointsize.vert"

    //
    #include "SHADER/Shader_PositionTexture.frag"
    #include "SHADER/Shader_PositionTexture.vert"

    //
    #include "SHADER/Shader_PositionTextureA8Color.frag"
    #include "SHADER/Shader_PositionTextureA8Color.vert"

    //
    #include "SHADER/Shader_PositionTextureColor.frag"
    #include "SHADER/Shader_PositionTextureColor.vert"

    //
    #include "SHADER/Shader_PositionTextureColor_noMVP.frag"
    #include "SHADER/Shader_PositionTextureColor_noMVP.vert"

    //
    #include "SHADER/Shader_PositionTextureColorAlphaTest.frag"

    //
    #include "SHADER/Shader_PositionTexture_uColor.frag"
    #include "SHADER/Shader_PositionTexture_uColor.vert"

    #include "SHADER/Shader_PositionColorLengthTexture.frag"
    #include "SHADER/Shader_PositionColorLengthTexture.vert"

    #include "SHADER/Shader_UI_Gray.frag"
    //
    #include "SHADER/Shader_Label.vert"
    #include "SHADER/Shader_Label_df.frag"
    #include "SHADER/Shader_Label_df_glow.frag"
    #include "SHADER/Shader_Label_normal.frag"
    #include "SHADER/Shader_Label_outline.frag"

    #include "SHADER/Shader_CameraClear.vert"
    #include "SHADER/Shader_CameraClear.frag"
}
