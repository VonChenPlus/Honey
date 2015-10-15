#ifndef SHADER_H
#define SHADER_H

#include "GRAPH/UNITY3D/GLCommon.h"

namespace GRAPH
{
    extern const GLchar * Position_uColor_frag;
    extern const GLchar * Position_uColor_vert;

    extern  const GLchar * PositionColor_frag;
    extern  const GLchar * PositionColor_vert;

    extern  const GLchar * PositionColorTextureAsPointsize_vert;

    extern  const GLchar * PositionTexture_frag;
    extern  const GLchar * PositionTexture_vert;

    extern  const GLchar * PositionTextureA8Color_frag;
    extern  const GLchar * PositionTextureA8Color_vert;

    extern  const GLchar * PositionTextureColor_frag;
    extern  const GLchar * PositionTextureColor_vert;

    extern  const GLchar * PositionTextureColor_noMVP_frag;
    extern  const GLchar * PositionTextureColor_noMVP_vert;

    extern  const GLchar * PositionTextureColorAlphaTest_frag;

    extern  const GLchar * PositionTexture_uColor_frag;
    extern  const GLchar * PositionTexture_uColor_vert;

    extern  const GLchar * PositionColorLengthTexture_frag;
    extern  const GLchar * PositionColorLengthTexture_vert;

    extern  const GLchar * PositionTexture_GrayScale_frag;

    extern  const GLchar * LabelDistanceFieldNormal_frag;
    extern  const GLchar * LabelDistanceFieldGlow_frag;
    extern  const GLchar * LabelNormal_frag;
    extern  const GLchar * LabelOutline_frag;

    extern  const GLchar * Label_vert;

    extern  const GLchar * CameraClearVert;
    extern  const GLchar * CameraClearFrag;
}

#endif // SHADER_H
