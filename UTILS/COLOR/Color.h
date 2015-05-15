#ifndef COLOR_H
#define COLOR_H

#include "BASE/Native.h"

namespace UTILS
{
    namespace COLOR
    {
        //have to use a define to ensure constant folding.. with an inline I don't get that, sucks
        #define COLOR(i) (((i&0xFF) << 16) | (i & 0xFF00) | ((i & 0xFF0000) >> 16) | 0xFF000000)
        Color DarkenColor(Color color);
        Color WhitenColor(Color color);

        uint32 WhiteAlpha(float alpha);
        uint32 BlackAlpha(float alpha);
        uint32 ColorAlpha(uint32 color, float alpha);
        uint32 ColorBlend(uint32 color, uint32 color2, float alpha);
        uint32 AlphaMul(uint32 color, float alphaMul);
        uint32 RGBA(float r, float g, float b, float alpha);
        uint32 RGBAClamp(float r, float g, float b, float alpha);
        uint32 HSVA(float h, float s, float v, float alpha);
    }
}

#endif // COLOR_H
