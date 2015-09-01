#include "Color.h"

namespace GRAPH
{
    Color3B::Color3B()
        : red(0)
        , green(0)
        , blue(0) {

    }

    Color3B::Color3B(HBYTE _r, HBYTE _g, HBYTE _b)
        : red(_r)
        , green(_g)
        , blue(_b) {

    }

    Color3B::Color3B(const Color4B& color)
        : red(color.red)
        , green(color.green)
        , blue(color.blue) {

    }

    Color3B::Color3B(const Color4F& color)
        : red(color.red * 255.0f)
        , green(color.green * 255.0f)
        , blue(color.blue * 255.0f) {

    }

    bool Color3B::operator==(const Color3B& right) const {
        return (red == right.red && green == right.green && blue == right.blue);
    }

    bool Color3B::operator==(const Color4B& right) const {
        return (red == right.red && green == right.green && blue == right.blue && 255 == right.alpha);
    }

    bool Color3B::operator==(const Color4F& right) const {
        return (right.alpha == 1.0f && Color4F(*this) == right);
    }

    bool Color3B::operator!=(const Color3B& right) const {
        return !(*this == right);
    }

    bool Color3B::operator!=(const Color4B& right) const {
        return !(*this == right);
    }

    bool Color3B::operator!=(const Color4F& right) const {
        return !(*this == right);
    }

    Color4B::Color4B()
        : red(0)
        , green(0)
        , blue(0)
        , alpha(0) {

    }

    Color4B::Color4B(HBYTE _r, HBYTE _g, HBYTE _b, HBYTE _a)
        : red(_r)
        , green(_g)
        , blue(_b)
        , alpha(_a) {

    }

    Color4B::Color4B(uint32 color)
        : red(color & 0xFF)
        , green((color & 0xFF00) >> 8)
        , blue((color & 0xFF0000) >> 16)
        , alpha((color & 0xFF000000) >> 24) {

    }

    Color4B::Color4B(const Color3B& color)
        : red(color.red)
        , green(color.green)
        , blue(color.blue)
        , alpha(255) {

    }

    Color4B::Color4B(const Color4F& color)
        : red(color.red * 255)
        , green(color.green * 255)
        , blue(color.blue * 255)
        , alpha(color.alpha * 255) {

    }

    bool Color4B::operator==(const Color4B& right) const {
        return (red == right.red && green == right.green && blue == right.blue && alpha == right.alpha);
    }

    bool Color4B::operator==(const Color3B& right) const {
        return (red == right.red && green == right.green && blue == right.blue && alpha == 255);
    }

    bool Color4B::operator==(const Color4F& right) const {
        return (*this == Color4B(right));
    }

    bool Color4B::operator!=(const Color4B& right) const {
        return !(*this == right);
    }

    bool Color4B::operator!=(const Color3B& right) const {
        return !(*this == right);
    }

    bool Color4B::operator!=(const Color4F& right) const {
        return !(*this == right);
    }

    Color4B Color4B::DarkenColor(Color4B color) {
        uint32 _color = color;
        return (_color & 0xFF000000) | ((_color >> 1)&0x7F7F7F);
    }

    Color4B Color4B::WhiteColor(Color4B color) {
        uint32 _color = color;
        return ((_color & 0xFF000000) | ((_color >> 1)&0x7F7F7F)) + 0x7F7F7F;
    }

    Color4B Color4B::WhiteAlpha(float alpha) {
        if (alpha < 0.0f) alpha = 0.0f;
        if (alpha > 1.0f) alpha = 1.0f;
        return Color4B(255, 255, 255, alpha*255);
    }

    Color4B Color4B::BlackAlpha(float alpha) {
        if (alpha < 0.0f) alpha = 0.0f;
        if (alpha > 1.0f) alpha = 1.0f;
        return Color4B(0, 0, 0, alpha*255);
    }

    Color4B Color4B::ColorAlpha(Color4B color, float alpha) {
        if (alpha < 0.0f) alpha = 0.0f;
        if (alpha > 1.0f) alpha = 1.0f;
        color.alpha = alpha * 255;
        return color;
    }

    Color4F::Color4F()
        : red(0.0f)
        , green(0.0f)
        , blue(0.0f)
        , alpha(0.0f) {

    }

    Color4F::Color4F(float _r, float _g, float _b, float _a)
        : red(_r)
        , green(_g)
        , blue(_b)
        , alpha(_a) {

    }

    Color4F::Color4F(const Color3B& color)
        : red(color.red / 255.0f)
        , green(color.green / 255.0f)
        , blue(color.blue / 255.0f)
        , alpha(1.0f) {

    }

    Color4F::Color4F(const Color4B& color)
        : red(color.red / 255.0f)
        , green(color.green / 255.0f)
        , blue(color.blue / 255.0f)
        , alpha(color.alpha / 255.0f) {

    }

    bool Color4F::operator==(const Color4F& right) const {
        return (red == right.red && green == right.green && blue == right.blue && alpha == right.alpha);
    }

    bool Color4F::operator==(const Color3B& right) const {
        return (alpha == 1.0f && Color3B(*this) == right);
    }

    bool Color4F::operator==(const Color4B& right) const {
        return (*this == Color4F(right));
    }

    bool Color4F::operator!=(const Color4F& right) const {
        return !(*this == right);
    }

    bool Color4F::operator!=(const Color3B& right) const {
        return !(*this == right);
    }

    bool Color4F::operator!=(const Color4B& right) const {
        return !(*this == right);
    }

    const Color3B Color3B::WHITE  (255, 255, 255);
    const Color3B Color3B::YELLOW (255, 255,   0);
    const Color3B Color3B::GREEN  (  0, 255,   0);
    const Color3B Color3B::BLUE   (  0,   0, 255);
    const Color3B Color3B::RED    (255,   0,   0);
    const Color3B Color3B::MAGENTA(255,   0, 255);
    const Color3B Color3B::BLACK  (  0,   0,   0);
    const Color3B Color3B::ORANGE (255, 127,   0);
    const Color3B Color3B::GRAY   (166, 166, 166);

    const Color4B Color4B::WHITE  (255, 255, 255, 255);
    const Color4B Color4B::YELLOW (255, 255,   0, 255);
    const Color4B Color4B::GREEN  (  0, 255,   0, 255);
    const Color4B Color4B::BLUE   (  0,   0, 255, 255);
    const Color4B Color4B::RED    (255,   0,   0, 255);
    const Color4B Color4B::MAGENTA(255,   0, 255, 255);
    const Color4B Color4B::BLACK  (  0,   0,   0, 255);
    const Color4B Color4B::ORANGE (255, 127,   0, 255);
    const Color4B Color4B::GRAY   (166, 166, 166, 255);

    const Color4F Color4F::WHITE  (    1,     1,     1, 1);
    const Color4F Color4F::YELLOW (    1,     1,     0, 1);
    const Color4F Color4F::GREEN  (    0,     1,     0, 1);
    const Color4F Color4F::BLUE   (    0,     0,     1, 1);
    const Color4F Color4F::RED    (    1,     0,     0, 1);
    const Color4F Color4F::MAGENTA(    1,     0,     1, 1);
    const Color4F Color4F::BLACK  (    0,     0,     0, 1);
    const Color4F Color4F::ORANGE (    1,  0.5f,     0, 1);
    const Color4F Color4F::GRAY   (0.65f, 0.65f, 0.65f, 1);
}

