#ifndef TYPES_H
#define TYPES_H

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
}

#endif // TYPES_H
