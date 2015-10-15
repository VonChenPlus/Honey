#ifndef GLSTATE_H
#define GLSTATE_H

#include <cstdint>
#include "GRAPH/UNITY3D/GLCommon.h"

namespace GRAPH
{
    enum
    {
        VERTEX_ATTRIB_FLAG_NONE       = 0,

        VERTEX_ATTRIB_FLAG_POSITION   = 1 << 0,
        VERTEX_ATTRIB_FLAG_COLOR      = 1 << 1,
        VERTEX_ATTRIB_FLAG_TEX_COORD = 1 << 2,
        VERTEX_ATTRIB_FLAG_NORMAL = 1 << 3,
        VERTEX_ATTRIB_FLAG_BLEND_WEIGHT = 1 << 4,
        VERTEX_ATTRIB_FLAG_BLEND_INDEX = 1 << 5,

        VERTEX_ATTRIB_FLAG_POS_COLOR_TEX = (VERTEX_ATTRIB_FLAG_POSITION | VERTEX_ATTRIB_FLAG_COLOR | VERTEX_ATTRIB_FLAG_TEX_COORD),
    };

    class GLStateCache
    {
    public:
        static void Invalidate(void);

        static void UseProgram(GLuint program);
        static void DeleteProgram(GLuint program);

        static void BlendFunc(GLenum sfactor, GLenum dfactor);
        static void BlendResetToCache(void);

        static void EnableVertexAttribs(uint32_t flags);

        static void BindTexture2D(GLuint textureId);
        static void BindTexture2DN(GLuint textureUnit, GLuint textureId);
        static void BindTextureN(GLuint textureUnit, GLuint textureId, GLuint textureType = GL_TEXTURE_2D);
        static void DeleteTexture(GLuint textureId);
        static void ActiveTexture(GLenum texture);
    };
}

#endif // GLSTATE_H
