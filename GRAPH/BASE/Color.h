#ifndef COLOR_H
#define COLOR_H

#include "GRAPH/BASE/GLCommon.h"

namespace GRAPH
{
    struct Color4B;
    struct Color4F;

    /**
     * RGB color composed of bytes 3 bytes.
     * @since v3.0
     */
    struct Color3B
    {
        Color3B();
        Color3B(GLubyte _r, GLubyte _g, GLubyte _b);
        explicit Color3B(const Color4B& color);
        explicit Color3B(const Color4F& color);

        bool operator==(const Color3B& right) const;
        bool operator==(const Color4B& right) const;
        bool operator==(const Color4F& right) const;
        bool operator!=(const Color3B& right) const;
        bool operator!=(const Color4B& right) const;
        bool operator!=(const Color4F& right) const;

        bool equals(const Color3B& other)
        {
            return (*this == other);
        }

        GLubyte r;
        GLubyte g;
        GLubyte b;

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

    /**
     * RGBA color composed of 4 bytes.
     * @since v3.0
     */
    struct Color4B
    {
        Color4B();
        Color4B(GLubyte _r, GLubyte _g, GLubyte _b, GLubyte _a);
        explicit Color4B(const Color3B& color);
        explicit Color4B(const Color4F& color);

        bool operator==(const Color4B& right) const;
        bool operator==(const Color3B& right) const;
        bool operator==(const Color4F& right) const;
        bool operator!=(const Color4B& right) const;
        bool operator!=(const Color3B& right) const;
        bool operator!=(const Color4F& right) const;

        GLubyte r;
        GLubyte g;
        GLubyte b;
        GLubyte a;

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


    /**
     * RGBA color composed of 4 floats.
     * @since v3.0
     */
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

        bool equals(const Color4F &other)
        {
            return (*this == other);
        }

        GLfloat r;
        GLfloat g;
        GLfloat b;
        GLfloat a;

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
