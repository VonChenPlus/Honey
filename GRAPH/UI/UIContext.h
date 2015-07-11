#ifndef UICONTEXT_H
#define UICONTEXT_H

#include <vector>

#include "MATH/Bounds.h"
#include "GRAPH/THIN3D/Thin3D.h"
#include "GRAPH/GFX/DrawBuffer.h"
#include "GRAPH/GFX/TextDrawer.h"

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

        void init(THIN3D::Thin3DContext *thin3d, THIN3D::Thin3DShaderSet *uiShaderTex,
                  THIN3D::Thin3DShaderSet *uiShaderNoTex, THIN3D::Thin3DTexture *uitexture,
                  GFX::DrawBuffer *uidrawbuffer, GFX::DrawBuffer *uidrawbufferTop,
                  GFX::TextDrawer *textDrawer);

        void begin();
        void beginNoTex();
        void flush();
        void end();

        void reBindTexture() const;

        // TODO: Support transformed bounds using stencil
        void pushScissor(const MATH::Boundsf &bounds);
        void popScissor();
        MATH::Boundsf getScissorBounds();

        void activateTopScissor();

        GFX::DrawBuffer *draw() const { return uidrawbuffer_; }
        GFX::DrawBuffer *drawTop() const { return uidrawbufferTop_; }
        const Theme *theme;

        // Utility methods

        GFX::TextDrawer *text() const { return textDrawer_; }

        void setFontStyle(const FontStyle &style);
        const FontStyle &getFontStyle() { return *fontStyle_; }
        void setFontScale(float scaleX, float scaleY);
        void measureTextCount(const FontStyle &style, const char *str, int count, float *x, float *y, int align = 0) const;
        void measureText(const FontStyle &style, const char *str, float *x, float *y, int align = 0) const;
        void drawText(const char *str, float x, float y, uint32 color, int align = 0);
        void drawTextRect(const char *str, const MATH::Boundsf &bounds, uint32 color, int align = 0);
        void fillRect(const Drawable &drawable, const MATH::Boundsf &bounds);

        // in dps, like dp_xres and dp_yres
        void setBounds(const MATH::Boundsf &b) { bounds_ = b; }
        const MATH::Boundsf &getBounds() const { return bounds_; }
        THIN3D::Thin3DContext *getThin3DContext() { return thin3d_; }

    private:
        void uiBegin(THIN3D::Thin3DShaderSet *shaderSet);
        void uiEnd();

        THIN3D::Thin3DContext *thin3d_;
        MATH::Boundsf bounds_;

        float fontScaleX_;
        float fontScaleY_;
        FontStyle *fontStyle_;
        GFX::TextDrawer *textDrawer_;

        THIN3D::Thin3DDepthStencilState *depth_;
        THIN3D::Thin3DBlendState *blend_;
        THIN3D::Thin3DShaderSet *uishader_;
        THIN3D::Thin3DShaderSet *uishadernotex_;
        THIN3D::Thin3DTexture *uitexture_;

        GFX::DrawBuffer *uidrawbuffer_;
        GFX::DrawBuffer *uidrawbufferTop_;

        std::vector<MATH::Boundsf> scissorStack_;
    };
}

#endif // UICONTEXT_H
