#ifndef GLCOMMON_H
#define GLCOMMON_H

#ifdef IOS
#include <OpenGLES/ES3/gl.h>
#include <OpenGLES/ES3/glext.h>
#elif defined(USING_GLES2)
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include "EGL/egl.h"
// At least Nokia platforms need the three below
#include <KHR/khrplatform.h>
typedef char GLchar;
#define GL_BGRA_EXT 0x80E1
#else // OpenGL
#include "EXTERNALS/glew/GL/glew.h"
#if defined(__APPLE__)
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif
#endif

#include "MATH/Vector.h"
#include "GRAPH/Color.h"

namespace GRAPH
{
    struct Tex2F
    {
        Tex2F(float _u, float _v): u(_u), v(_v) {}
        Tex2F(const MATH::Vector2f value) :u(value.x), v(value.y){}
        Tex2F(): u(0.f), v(0.f) {}

        GLfloat u;
        GLfloat v;
    };

    struct V2F_C4B_T2F
    {
        MATH::Vector2f  vertices;
        Color4B        colors;
        Tex2F          texCoords;
    };

    struct V2F_C4B_PF
    {
        MATH::Vector2f  vertices;
        Color4B        colors;
        float      pointSize;
    };

    struct V2F_C4F_T2F
    {
        MATH::Vector2f       vertices;
        Color4F        colors;
        Tex2F          texCoords;
    };

    struct V3F_C4B_T2F
    {
        MATH::Vector3f     vertices;
        Color4B      colors;
        Tex2F        texCoords;
    };

    struct V3F_T2F
    {
        MATH::Vector3f       vertices;
        Tex2F          texCoords;
    };

    struct V2F_C4B_T2F_Triangle
    {
        V2F_C4B_T2F a;
        V2F_C4B_T2F b;
        V2F_C4B_T2F c;
    };

    struct V2F_C4B_T2F_Quad
    {
        V2F_C4B_T2F    bl;
        V2F_C4B_T2F    br;
        V2F_C4B_T2F    tl;
        V2F_C4B_T2F    tr;
    };

    struct V3F_C4B_T2F_Quad
    {
        V3F_C4B_T2F    tl;
        V3F_C4B_T2F    bl;
        V3F_C4B_T2F    tr;
        V3F_C4B_T2F    br;
    };

    struct V2F_C4F_T2F_Quad
    {
        V2F_C4F_T2F    bl;
        V2F_C4F_T2F    br;
        V2F_C4F_T2F    tl;
        V2F_C4F_T2F    tr;
    };

    struct V3F_T2F_Quad
    {
        V3F_T2F    bl;
        V3F_T2F    br;
        V3F_T2F    tl;
        V3F_T2F    tr;
    };

    template <typename T>
    class VertexBufferObject
    {
    public:
        union
        {
            struct {
                GLuint objectID;
                uint64 bufferCapacity;
                uint64 bufferCount;
                T *bufferData;
            } u1;
            struct{
                GLuint objectID[2];
                uint64 bufferCapacity;
                uint64 bufferCount;
                T *bufferData;
                GLushort *indexData;
                uint64 indexCapacity;
                uint64 indexCount;
            } u2;
        };
    };
}

#endif // GLCOMMON_H

