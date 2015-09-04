#ifndef TYPES_H
#define TYPES_H

#include "MATH/Vector.h"
#include "MATH/Size.h"
#include "GRAPH/Color.h"
#include "GRAPH/RENDERER/GLCommon.h"

namespace GRAPH
{
    enum class TextVAlignment
    {
        TOP,
        CENTER,
        BOTTOM
    };

    enum class TextHAlignment
    {
        LEFT,
        CENTER,
        RIGHT
    };

    enum class TextAlign
    {
        CENTER        = 0x33, /** Horizontal center and vertical center. */
        TOP           = 0x13, /** Horizontal center and vertical top. */
        TOP_RIGHT     = 0x12, /** Horizontal right and vertical top. */
        RIGHT         = 0x32, /** Horizontal right and vertical center. */
        BOTTOM_RIGHT  = 0x22, /** Horizontal right and vertical bottom. */
        BOTTOM        = 0x23, /** Horizontal center and vertical bottom. */
        BOTTOM_LEFT   = 0x21, /** Horizontal left and vertical bottom. */
        LEFT          = 0x31, /** Horizontal left and vertical center. */
        TOP_LEFT      = 0x11, /** Horizontal left and vertical top. */
    };

    struct FontShadow
    {
    public:
        FontShadow()
            : shadowEnabled(false)
            , shadowBlur(0)
            , shadowOpacity(0)
        {}

        bool   shadowEnabled;
        MATH::Sizef  shadowOffset;
        float  shadowBlur;
        float  shadowOpacity;
    };

    struct FontStroke
    {
    public:
        FontStroke()
            : strokeEnabled(false)
            , strokeColor(Color3B::BLACK)
            , strokeAlpha(255)
            , strokeSize(0)
        {}

        bool      strokeEnabled;
        Color3B   strokeColor;
        GLubyte   strokeAlpha;
        float     strokeSize;
    };

    struct FontDefinition
    {
    public:
        FontDefinition()
            : fontSize(0)
            , alignment(TextHAlignment::CENTER)
            , vertAlignment(TextVAlignment::TOP)
            , dimensions(MATH::SizefZERO)
            , fontFillColor(Color3B::WHITE)
            , fontAlpha(255)
        {}
        std::string           fontName;
        int                   fontSize;
        TextHAlignment        alignment;
        TextVAlignment vertAlignment;
        MATH::Sizef           dimensions;
        Color3B               fontFillColor;
        GLubyte               fontAlpha;
        FontShadow            shadow;
        FontStroke            stroke;
    };

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

    struct BlendFunc
    {
        GLenum src;
        GLenum dst;

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
