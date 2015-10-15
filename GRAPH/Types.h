#ifndef TYPES_H
#define TYPES_H

#include "MATH/Vector.h"
#include "MATH/Size.h"
#include "GRAPH/Color.h"
#include "GRAPH/UNITY3D/GLCommon.h"

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
