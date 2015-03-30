#ifndef UICONTEXT_H
#define UICONTEXT_H

#include <vector>

#include "MATH/Bounds.h"

namespace UI
{
    class UIContext
    {
    public:
        UIContext();
        ~UIContext();

        void init(Thin3DContext *thin3d, Thin3DShaderSet *uiShaderTex, Thin3DShaderSet *uiShaderNoTex, Thin3DTexture *uitexture, DrawBuffer *uidrawbuffer, DrawBuffer *uidrawbufferTop);

        void begin();
        void beginNoTex();
        void flush();
        void end();

        void rebindTexture() const;

        // TODO: Support transformed bounds using stencil
        void pushScissor(const Bounds &bounds);
        void popScissor();
        MATH::Bounds getScissorBounds();

        void activateTopScissor();

        DrawBuffer *draw() const { return uidrawbuffer_; }
        DrawBuffer *drawTop() const { return uidrawbufferTop_; }
        const UI::Theme *theme;

        // Utility methods

        TextDrawer *text() const { return textDrawer_; }

        void setFontStyle(const UI::FontStyle &style);
        const UI::FontStyle &getFontStyle() { return *fontStyle_; }
        void setFontScale(float scaleX, float scaleY);
        void measureTextCount(const UI::FontStyle &style, const char *str, int count, float *x, float *y, int align = 0) const;
        void measureText(const UI::FontStyle &style, const char *str, float *x, float *y, int align = 0) const;
        void drawText(const char *str, float x, float y, uint32_t color, int align = 0);
        void drawTextRect(const char *str, const Bounds &bounds, uint32_t color, int align = 0);
        void fillRect(const UI::Drawable &drawable, const Bounds &bounds);

        // in dps, like dp_xres and dp_yres
        void setBounds(const Bounds &b) { bounds_ = b; }
        const Bounds &getBounds() const { return bounds_; }
        Thin3DContext *getThin3DContext() { return thin3d_; }

    private:
        Thin3DContext *thin3d_;
        Bounds bounds_;

        float fontScaleX_;
        float fontScaleY_;
        UI::FontStyle *fontStyle_;
        TextDrawer *textDrawer_;

        Thin3DContext *thin3D_;
        Thin3DDepthStencilState *depth_;
        Thin3DBlendState *blend_;
        Thin3DShaderSet *uishader_;
        Thin3DShaderSet *uishadernotex_;
        Thin3DTexture *uitexture_;

        DrawBuffer *uidrawbuffer_;
        DrawBuffer *uidrawbufferTop_;

        std::vector<Bounds> scissorStack_;
    };
}

#endif // UICONTEXT_H
