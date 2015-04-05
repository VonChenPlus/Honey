#ifndef COLOR_H
#define COLOR_H

#include "BASE/BasicTypes.h"

typedef unsigned int Color;

//have to use a define to ensure constant folding.. with an inline I don't get that, sucks
#define COLOR(i) (((i&0xFF) << 16) | (i & 0xFF00) | ((i & 0xFF0000) >> 16) | 0xFF000000)
inline Color DarkenColor(Color color) {
  return (color & 0xFF000000) | ((color >> 1)&0x7F7F7F);
}
inline Color WhitenColor(Color color) {
  return ((color & 0xFF000000) | ((color >> 1)&0x7F7F7F)) + 0x7F7F7F;
}

uint32 WhiteAlpha(float alpha);
uint32 BlackAlpha(float alpha);
uint32 ColorAlpha(uint32 color, float alpha);
uint32 ColorBlend(uint32 color, uint32 color2, float alpha);
uint32 AlphaMul(uint32 color, float alphaMul);
uint32 RGBA(float r, float g, float b, float alpha);
uint32 RGBAClamp(float r, float g, float b, float alpha);
uint32 HSVA(float h, float s, float v, float alpha);

#endif // COLOR_H
