#ifndef FONTS_H
#define FONTS_H

#include "BASE/HObject.h"
#include "MATH/Size.h"
#include "GRAPH/BASE/Color.h"
#include "GRAPH/BASE/Types.h"

namespace GRAPH
{
    struct FontShadow
    {
    public:

        // shadow is not enabled by default
        FontShadow()
            : _shadowEnabled(false)
            , _shadowBlur(0)
            , _shadowOpacity(0)
        {}

        bool   _shadowEnabled;
        MATH::Sizef _shadowOffset;
        float  _shadowBlur;
        float  _shadowOpacity;
    };

    struct FontStroke
    {
    public:
        FontStroke()
            : _strokeEnabled(false)
            , _strokeColor(Color3B::BLACK)
            , _strokeAlpha(255)
            , _strokeSize(0)
        {}

        bool      _strokeEnabled;
        Color3B   _strokeColor;
        GLubyte   _strokeAlpha;
        float     _strokeSize;
    };

    struct FontDefinition
    {
    public:
        FontDefinition()
            : _fontSize(0)
            , _alignment(TextHAlignment::CENTER)
            , _vertAlignment(TextVAlignment::TOP)
            , _dimensions(MATH::SizefZERO)
            , _fontFillColor(Color3B::WHITE)
            , _fontAlpha(255)
        {}

        std::string           _fontName;
        int                   _fontSize;
        TextHAlignment        _alignment;
        TextVAlignment _vertAlignment;
        MATH::Sizef         _dimensions;
        Color3B               _fontFillColor;
        GLubyte               _fontAlpha;
        FontShadow            _shadow;
        FontStroke            _stroke;
    };
}

#endif // FONTS_H
