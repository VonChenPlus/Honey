#ifndef COLOR_H
#define COLOR_H

#include "BASE/Honey.h"

namespace GRAPH
{
    struct Color4B;
    struct Color4F;

    struct Color3B
    {
        Color3B();
        Color3B(HBYTE _r, HBYTE _g, HBYTE _b);
        explicit Color3B(const Color4B& color);
        explicit Color3B(const Color4F& color);

        bool operator==(const Color3B& right) const;
        bool operator==(const Color4B& right) const;
        bool operator==(const Color4F& right) const;
        bool operator!=(const Color3B& right) const;
        bool operator!=(const Color4B& right) const;
        bool operator!=(const Color4F& right) const;

        bool equals(const Color3B& other) {
            return (*this == other);
        }

        HBYTE red;
        HBYTE green;
        HBYTE blue;

        static const Color3B WHITE;
        static const Color3B YELLOW;
        static const Color3B BLUE;
        static const Color3B GREEN;
        static const Color3B RED;
        static const Color3B MAGENTA;
        static const Color3B BLACK;
        static const Color3B ORANGE;
        static const Color3B GRAY;
    };

    struct Color4B
    {
        Color4B();
        Color4B(HBYTE _r, HBYTE _g, HBYTE _b, HBYTE _a);
        Color4B(uint32 color);
        explicit Color4B(const Color3B& color);
        explicit Color4B(const Color4F& color);

        bool operator==(const Color4B& right) const;
        bool operator==(const Color3B& right) const;
        bool operator==(const Color4F& right) const;
        bool operator!=(const Color4B& right) const;
        bool operator!=(const Color3B& right) const;
        bool operator!=(const Color4F& right) const;
        inline operator uint32() {
            return alpha << 24 | blue << 16 | green << 8 | red;
        }

        HBYTE red;
        HBYTE green;
        HBYTE blue;
        HBYTE alpha;

        static Color4B DarkenColor(Color4B);
        static Color4B WhiteColor(Color4B);
        static Color4B WhiteAlpha(float alpha);
        static Color4B BlackAlpha(float alpha);

        static const Color4B WHITE;
        static const Color4B YELLOW;
        static const Color4B BLUE;
        static const Color4B GREEN;
        static const Color4B RED;
        static const Color4B MAGENTA;
        static const Color4B BLACK;
        static const Color4B ORANGE;
        static const Color4B GRAY;
    };

    struct Color4F
    {
        Color4F();
        Color4F(float _r, float _g, float _b, float _a);
        explicit Color4F(const Color3B& color);
        explicit Color4F(const Color4B& color);

        bool operator==(const Color4F& right) const;
        bool operator==(const Color3B& right) const;
        bool operator==(const Color4B& right) const;
        bool operator!=(const Color4F& right) const;
        bool operator!=(const Color3B& right) const;
        bool operator!=(const Color4B& right) const;

        bool equals(const Color4F &other) {
            return (*this == other);
        }

        float red;
        float green;
        float blue;
        float alpha;

        static const Color4F WHITE;
        static const Color4F YELLOW;
        static const Color4F BLUE;
        static const Color4F GREEN;
        static const Color4F RED;
        static const Color4F MAGENTA;
        static const Color4F BLACK;
        static const Color4F ORANGE;
        static const Color4F GRAY;
    };
}

#endif // COLOR_H
