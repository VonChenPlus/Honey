#ifndef THEME_H
#define THEME_H

#include "BASE/BasicTypes.h"

namespace UI
{
enum DrawableType
{
    DRAW_NOTHING,
    DRAW_SOLID_COLOR,
    DRAW_4GRID,
    DRAW_STRETCH_IMAGE,
};

struct Drawable
{
    Drawable() : type(DRAW_NOTHING), image(-1), color(0xFFFFFFFF) {}
    explicit Drawable(uint32 col) : type(DRAW_SOLID_COLOR), image(-1), color(col) {}
    Drawable(DrawableType t, int img, uint32 col = 0xFFFFFFFF) : type(t), image(img), color(col) {}

    DrawableType type;
    uint32 image;
    uint32 color;
};

struct Style
{
    Style() : fgColor(0xFFFFFFFF), background(0xFF303030), image(-1) {}

    uint32 fgColor;
    Drawable background;
    int image;  // where applicable.
};

struct FontStyle
{
    FontStyle() : atlasFont(0), sizePts(0), flags(0) {}
    FontStyle(const char *name, int size) : atlasFont(0), fontName(name), sizePts(size), flags(0) {}
    FontStyle(int atlasFnt, const char *name, int size) : atlasFont(atlasFnt), fontName(name), sizePts(size), flags(0) {}

    int atlasFont;
    // For native fonts:
    std::string fontName;
    int sizePts;
    int flags;
};

// To use with an UI atlas.
struct Theme
{
    FontStyle uiFont;
    FontStyle uiFontSmall;
    FontStyle uiFontSmaller;
    int checkOn;
    int checkOff;
    int sliderKnob;
    int whiteImage;
    int dropShadow4Grid;

    Style buttonStyle;
    Style buttonFocusedStyle;
    Style buttonDownStyle;
    Style buttonDisabledStyle;
    Style buttonHighlightedStyle;

    Style itemStyle;
    Style itemDownStyle;
    Style itemFocusedStyle;
    Style itemDisabledStyle;
    Style itemHighlightedStyle;

    Style headerStyle;

    Style popupTitle;
};
}

#endif // THEME_H

