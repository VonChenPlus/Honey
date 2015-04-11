#ifndef COLOR_H
#define COLOR_H

#include "BASE/BasicTypes.h"

namespace UTILS
{
    uint32 WhiteAlpha(float alpha);
    uint32 BlackAlpha(float alpha);
    uint32 ColorAlpha(uint32_t color, float alpha);
    uint32 ColorBlend(uint32_t color, uint32_t color2, float alpha);
    uint32 AlphaMul(uint32_t color, float alphaMul);
    uint32 RGBA(float r, float g, float b, float alpha);
    uint32 rgba_clamp(float r, float g, float b, float alpha);
    uint32 hsva(float h, float s, float v, float alpha);
}

#endif // COLOR_H
