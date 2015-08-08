#ifndef TYPES_H
#define TYPES_H

#include "MATH/Vector.h"
#include "GRAPH/BASE/Color.h"

namespace GRAPH
{
    enum class TextVAlignment
    {
        TOP,
        CENTER,
        BOTTOM
    };

    /** @struct TextHAlignment
     * Horizontal text alignment type.
     *
     * @note If any of these enums are edited and/or reordered, update Texture2D.m.
     */
    enum class TextHAlignment
    {
        LEFT,
        CENTER,
        RIGHT
    };

    struct Tex2F
    {
        Tex2F(float _u, float _v): u(_u), v(_v) {}

        Tex2F(): u(0.f), v(0.f) {}

        GLfloat u;
        GLfloat v;
    };

    /** @struct V2F_C4B_T2F
     * A Vec2 with a vertex point, a tex coord point and a color 4B.
     */
    struct V2F_C4B_T2F
    {
        /// vertices (2F)
        MATH::Vector2f  vertices;
        /// colors (4B)
        Color4B        colors;
        /// tex coords (2F)
        Tex2F          texCoords;
    };

    /** @struct V2F_C4B_PF
     *
     */
    struct V2F_C4B_PF
    {
        /// vertices (2F)
        MATH::Vector2f  vertices;
        /// colors (4B)
        Color4B        colors;
        /// pointsize
        float      pointSize;
    };

    /** @struct V2F_C4F_T2F
     * A Vec2 with a vertex point, a tex coord point and a color 4F.
     */
    struct V2F_C4F_T2F
    {
        /// vertices (2F)
        MATH::Vector2f       vertices;
        /// colors (4F)
        Color4F        colors;
        /// tex coords (2F)
        Tex2F          texCoords;
    };

    /** @struct V3F_C4B_T2F
     * A Vec2 with a vertex point, a tex coord point and a color 4B.
     */
    struct V3F_C4B_T2F
    {
        /// vertices (3F)
        MATH::Vector3f     vertices;            // 12 bytes

        /// colors (4B)
        Color4B      colors;              // 4 bytes

        // tex coords (2F)
        Tex2F        texCoords;           // 8 bytes
    };

    /** @struct V3F_T2F
     * A Vec2 with a vertex point, a tex coord point.
     */
    struct V3F_T2F
    {
        /// vertices (2F)
        MATH::Vector3f       vertices;
        /// tex coords (2F)
        Tex2F          texCoords;
    };

    /** @struct V2F_C4B_T2F_Triangle
     * A Triangle of V2F_C4B_T2F.
     */
    struct V2F_C4B_T2F_Triangle
    {
        V2F_C4B_T2F a;
        V2F_C4B_T2F b;
        V2F_C4B_T2F c;
    };

    /** @struct V2F_C4B_T2F_Quad
     * A Quad of V2F_C4B_T2F.
     */
    struct V2F_C4B_T2F_Quad
    {
        /// bottom left
        V2F_C4B_T2F    bl;
        /// bottom right
        V2F_C4B_T2F    br;
        /// top left
        V2F_C4B_T2F    tl;
        /// top right
        V2F_C4B_T2F    tr;
    };

    /** @struct V3F_C4B_T2F_Quad
     * 4 Vertex3FTex2FColor4B.
     */
    struct V3F_C4B_T2F_Quad
    {
        /// top left
        V3F_C4B_T2F    tl;
        /// bottom left
        V3F_C4B_T2F    bl;
        /// top right
        V3F_C4B_T2F    tr;
        /// bottom right
        V3F_C4B_T2F    br;
    };

    /** @struct V2F_C4F_T2F_Quad
     * 4 Vertex2FTex2FColor4F Quad.
     */
    struct V2F_C4F_T2F_Quad
    {
        /// bottom left
        V2F_C4F_T2F    bl;
        /// bottom right
        V2F_C4F_T2F    br;
        /// top left
        V2F_C4F_T2F    tl;
        /// top right
        V2F_C4F_T2F    tr;
    };

    /** @struct V3F_T2F_Quad
     *
     */
    struct V3F_T2F_Quad
    {
        /// bottom left
        V3F_T2F    bl;
        /// bottom right
        V3F_T2F    br;
        /// top left
        V3F_T2F    tl;
        /// top right
        V3F_T2F    tr;
    };

    struct BlendFunc
    {
        /** source blend function */
        GLenum src;
        /** destination blend function */
        GLenum dst;

        /** Blending disabled. Uses {GL_ONE, GL_ZERO} */
        static const BlendFunc DISABLE;
        /** Blending enabled for textures with Alpha premultiplied. Uses {GL_ONE, GL_ONE_MINUS_SRC_ALPHA} */
        static const BlendFunc ALPHA_PREMULTIPLIED;
        /** Blending enabled for textures with Alpha NON premultiplied. Uses {GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA} */
        static const BlendFunc ALPHA_NON_PREMULTIPLIED;
        /** Enables Additive blending. Uses {GL_SRC_ALPHA, GL_ONE} */
        static const BlendFunc ADDITIVE;

        bool operator==(const BlendFunc &a) const
        {
            return src == a.src && dst == a.dst;
        }

        bool operator!=(const BlendFunc &a) const
        {
            return src != a.src || dst != a.dst;
        }

        bool operator<(const BlendFunc &a) const
        {
            return src < a.src || (src == a.src && dst < a.dst);
        }
    };
}

#endif // TYPES_H
