#ifndef TYPES_H
#define TYPES_H

#include "MATH/Vector.h"
#include "MATH/Size.h"
#include "GRAPH/Color.h"

namespace GRAPH
{
    struct Tex2F
    {
        Tex2F(float _u, float _v) : u(_u), v(_v) {}
        Tex2F(const MATH::Vector2f value) :u(value.x), v(value.y){}
        Tex2F() : u(0.f), v(0.f) {}

        float u;
        float v;
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
                uint64 bufferCapacity;
                uint64 bufferCount;
                T *bufferData;
            } u1;
            struct{
                uint64 bufferCapacity;
                uint64 bufferCount;
                T *bufferData;
                uint16 *indexData;
                uint64 indexCapacity;
                uint64 indexCount;
            } u2;
        };
    };

    struct BlendFunc
    {
        uint32 src;
        uint32 dst;

        static const BlendFunc DISABLE;
        static const BlendFunc ALPHA_PREMULTIPLIED;
        static const BlendFunc ALPHA_NON_PREMULTIPLIED;
        static const BlendFunc ADDITIVE;

        bool operator==(const BlendFunc &a) const {
            return src == a.src && dst == a.dst;
        }

        bool operator!=(const BlendFunc &a) const {
            return src != a.src || dst != a.dst;
        }

        bool operator<(const BlendFunc &a) const {
            return src < a.src || (src == a.src && dst < a.dst);
        }
    };
}

#endif // TYPES_H
