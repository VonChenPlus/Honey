#ifndef SHADER_H
#define SHADER_H

#include "GRAPH/BASE/GLCommon.h"

namespace GRAPH
{
    extern const GLchar * ccPosition_uColor_frag;
    extern const GLchar * ccPosition_uColor_vert;

    extern  const GLchar * ccPositionColor_frag;
    extern  const GLchar * ccPositionColor_vert;

    extern  const GLchar * ccPositionColorTextureAsPointsize_vert;

    extern  const GLchar * ccPositionTexture_frag;
    extern  const GLchar * ccPositionTexture_vert;

    extern  const GLchar * ccPositionTextureA8Color_frag;
    extern  const GLchar * ccPositionTextureA8Color_vert;

    extern  const GLchar * ccPositionTextureColor_frag;
    extern  const GLchar * ccPositionTextureColor_vert;

    extern  const GLchar * ccPositionTextureColor_noMVP_frag;
    extern  const GLchar * ccPositionTextureColor_noMVP_vert;

    extern  const GLchar * ccPositionTextureColorAlphaTest_frag;

    extern  const GLchar * ccPositionTexture_uColor_frag;
    extern  const GLchar * ccPositionTexture_uColor_vert;

    extern  const GLchar * ccPositionColorLengthTexture_frag;
    extern  const GLchar * ccPositionColorLengthTexture_vert;

    extern  const GLchar * ccPositionTexture_GrayScale_frag;

    extern  const GLchar * ccLabelDistanceFieldNormal_frag;
    extern  const GLchar * ccLabelDistanceFieldGlow_frag;
    extern  const GLchar * ccLabelNormal_frag;
    extern  const GLchar * ccLabelOutline_frag;

    extern  const GLchar * ccLabel_vert;

    extern  const GLchar * ccCameraClearVert;
    extern  const GLchar * ccCameraClearFrag;
}

#endif // SHADER_H
