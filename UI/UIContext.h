#ifndef UICONTEXT_H
#define UICONTEXT_H

#include <vector>

#include "MATH/Bounds.h"
#include "THIN3D/Thin3D.h"
#include "GFX/DrawBuffer.h"
#include "GFX/DrawText.h"

namespace UI
{
    struct Drawable;
    struct Theme;
    struct FontStyle;

    class UIContext
    {
    public:
        UIContext();
        ~UIContext();

        void init(THIN3D::Thin3DContext *thin3d, THIN3D::Thin3DShaderSet *uiShaderTex, THIN3D::Thin3DShaderSet *uiShaderNoTex, THIN3D::Thin3DTexture *uitexture, GFX::DrawBuffer *uidrawbuffer, GFX::DrawBuffer *uidrawbufferTop);

        void begin();
        void beginNoTex();
        void flush();
        void end();

        void reBindTexture() const;

        // TODO: Support transformed bounds using stencil
        void pushScissor(const MATH::Bounds &bounds);
        void popScissor();
        MATH::Bounds getScissorBounds();

        void activateTopScissor();

        GFX::DrawBuffer *draw() const { return uidrawbuffer_; }
        GFX::DrawBuffer *drawTop() const { return uidrawbufferTop_; }
        const UI::Theme *theme;

        // Utility methods

        GFX::TextDrawer *text() const { return textDrawer_; }

        void setFontStyle(const UI::FontStyle &style);
        const UI::FontStyle &getFontStyle() { return *fontStyle_; }
        void setFontScale(float scaleX, float scaleY);
        void measureTextCount(const UI::FontStyle &style, const char *str, int count, float *x, float *y, int align = 0) const;
        void measureText(const UI::FontStyle &style, const char *str, float *x, float *y, int align = 0) const;
        void drawText(const char *str, float x, float y, uint32_t color, int align = 0);
        void drawTextRect(const char *str, const MATH::Bounds &bounds, uint32_t color, int align = 0);
        void fillRect(const UI::Drawable &drawable, const MATH::Bounds &bounds);

        // in dps, like dp_xres and dp_yres
        void setBounds(const MATH::Bounds &b) { bounds_ = b; }
        const MATH::Bounds &getBounds() const { return bounds_; }
        THIN3D::Thin3DContext *getThin3DContext() { return thin3d_; }

    private:
        THIN3D::Thin3DContext *thin3d_;
        MATH::Bounds bounds_;

        float fontScaleX_;
        float fontScaleY_;
        UI::FontStyle *fontStyle_;
        GFX::TextDrawer *textDrawer_;

        THIN3D::Thin3DContext *thin3D_;
        THIN3D::Thin3DDepthStencilState *depth_;
        THIN3D::Thin3DBlendState *blend_;
        THIN3D::Thin3DShaderSet *uishader_;
        THIN3D::Thin3DShaderSet *uishadernotex_;
        THIN3D::Thin3DTexture *uitexture_;

        GFX::DrawBuffer *uidrawbuffer_;
        GFX::DrawBuffer *uidrawbufferTop_;

        std::vector<MATH::Bounds> scissorStack_;
    };
}

#endif // UICONTEXT_H
