#ifndef FONTS_H
#define FONTS_H

#include "MATH/Size.h"
#include "GRAPH/BASE/Color.h"
#include "GRAPH/BASE/Types.h"

namespace GRAPH
{
    /**
     types used for defining fonts properties (i.e. font name, size, stroke or shadow)
     */

    /** @struct FontShadow
     * Shadow attributes.
     */
    struct FontShadow
    {
    public:

        // shadow is not enabled by default
        FontShadow()
            : _shadowEnabled(false)
            , _shadowBlur(0)
            , _shadowOpacity(0)
        {}

        /// true if shadow enabled
        bool   _shadowEnabled;
        /// shadow x and y offset
        MATH::Sizef _shadowOffset;
        /// shadow blurrines
        float  _shadowBlur;
        /// shadow opacity
        float  _shadowOpacity;
    };

    /** @struct FontStroke
     * Stroke attributes.
     */
    struct FontStroke
    {
    public:

        // stroke is disabled by default
        FontStroke()
            : _strokeEnabled(false)
            , _strokeColor(Color3B::BLACK)
            , _strokeAlpha(255)
            , _strokeSize(0)
        {}

        /// true if stroke enabled
        bool      _strokeEnabled;
        /// stroke color
        Color3B   _strokeColor;
        /// stroke alpha
        GLubyte   _strokeAlpha;
        /// stroke size
        float     _strokeSize;

    };

    /** @struct FontDefinition
     * Font attributes.
     */
    struct FontDefinition
    {
    public:
        /**
         * @js NA
         * @lua NA
         */
        FontDefinition()
            : _fontSize(0)
            , _alignment(TextHAlignment::CENTER)
            , _vertAlignment(TextVAlignment::TOP)
            , _dimensions(Size::ZERO)
            , _fontFillColor(Color3B::WHITE)
            , _fontAlpha(255)
        {}

        /// font name
        std::string           _fontName;
        /// font size
        int                   _fontSize;
        /// horizontal alignment
        TextHAlignment        _alignment;
        /// vertical alignment
        TextVAlignment _vertAlignment;
        /// renering box
        MATH::Sizef         _dimensions;
        /// font color
        Color3B               _fontFillColor;
        /// font alpha
        GLubyte               _fontAlpha;
        /// font shadow
        FontShadow            _shadow;
        /// font stroke
        FontStroke            _stroke;

    };
}

#endif // FONTS_H
