#include "UIContext.h"
using MATH::Bounds;
using THIN3D::Thin3DContext;
using THIN3D::Thin3DShaderSet;
using THIN3D::Thin3DTexture;
using GFX::DrawBuffer;
using THIN3D::T3DBlendStatePreset;
using THIN3D::T3DComparison;
using GFX::TextDrawer;
using THIN3D::T3DRenderState;
using THIN3D::T3DCullMode;
using GFX::FLAG_DYNAMIC_ASCII;
#include "INPUT/InputState.h"
#include "UI/UI.h"
using UI::UIState;
#include "UI/View.h"

namespace UI
{
    // This is the drawbuffer used for UI. Remember to flush it at the end of the frame.
    // TODO: One should probably pass it in through UIInit.
    extern DrawBuffer ui_draw2d;
    extern DrawBuffer ui_draw2d_front;	// for things that need to be on top of the rest
    // This needs to be extern so that additional UI controls can be developed outside this file.
    extern UIState uistate;

    extern int dp_xres;
    extern int dp_yres;
    extern float g_dpi_scale;

    static void UIBegin(Thin3DShaderSet *shaderSet) {
        for (int i = 0; i < MAX_POINTERS; i++)
            uistate.hotitem[i] = 0;
        ui_draw2d.begin(shaderSet);
        ui_draw2d_front.begin(shaderSet);
    }

    static void UIEnd() {
        for (int i = 0; i < MAX_POINTERS; i++) {
            if (uistate.mousedown[i] == 0) {
                uistate.activeitem[i] = 0;
            }
            else {
                if (uistate.activeitem[i] == 0) {
                    uistate.activeitem[i] = -1;
                }
            }
        }
        ui_draw2d.end();
        ui_draw2d_front.end();

        if (uistate.ui_tick > 0)
            uistate.ui_tick--;
        ui_draw2d.flush();
        ui_draw2d_front.flush();
    }

    UIContext::UIContext()
        : uishader_(0)
        , uitexture_(0)
        , uidrawbuffer_(0)
        , uidrawbufferTop_(0) {
        fontScaleX_ = 1.0f;
        fontScaleY_ = 1.0f;
        fontStyle_ = new FontStyle();
        bounds_ = Bounds(0, 0, dp_xres, dp_yres);
    }

    UIContext::~UIContext() {
        delete fontStyle_;
        delete textDrawer_;
    }

    void UIContext::init(Thin3DContext *thin3d, Thin3DShaderSet *uishader, Thin3DShaderSet *uishadernotex, Thin3DTexture *uitexture, DrawBuffer *uidrawbuffer, DrawBuffer *uidrawbufferTop) {
        thin3d_ = thin3d;
        blend_ = thin3d_->getBlendStatePreset(T3DBlendStatePreset::BS_STANDARD_ALPHA);
        depth_ = thin3d_->createDepthStencilState(false, false, T3DComparison::LESS);

        uishader_ = uishader;
        uishadernotex_ = uishadernotex;
        uitexture_ = uitexture;
        uidrawbuffer_ = uidrawbuffer;
        uidrawbufferTop_ = uidrawbufferTop;
        textDrawer_ = new TextDrawer(thin3d);
    }

    void UIContext::begin() {
        thin3d_->setBlendState(blend_);
        thin3d_->setDepthStencilState(depth_);
        thin3d_->setRenderState(T3DRenderState::CULL_MODE, T3DCullMode::NO_CULL);
        thin3d_->setTexture(0, uitexture_);
        thin3d_->setScissorEnabled(false);
        UIBegin(uishader_);
    }

    void UIContext::beginNoTex() {
        thin3d_->setBlendState(blend_);
        thin3d_->setRenderState(T3DRenderState::CULL_MODE, T3DCullMode::NO_CULL);

        UIBegin(uishadernotex_);
    }

    void UIContext::reBindTexture() const {
        thin3d_->setTexture(0, uitexture_);
    }

    void UIContext::flush() {
        if (uidrawbuffer_) {
            uidrawbuffer_->end();
            uidrawbuffer_->flush();
        }
        if (uidrawbufferTop_) {
            uidrawbufferTop_->end();
            uidrawbufferTop_->flush();
        }
    }

    void UIContext::end() {
        UIEnd();
        flush();
    }

    // TODO: Support transformed bounds using stencil
    void UIContext::pushScissor(const Bounds &bounds) {
        flush();
        Bounds clipped = bounds;
        if (scissorStack_.size())
            clipped.clip(scissorStack_.back());
        scissorStack_.push_back(clipped);
        activateTopScissor();
    }

    void UIContext::popScissor() {
        flush();
        scissorStack_.pop_back();
        activateTopScissor();
    }

    Bounds UIContext::getScissorBounds() {
        if (!scissorStack_.empty())
            return scissorStack_.back();
        else
            return bounds_;
    }

    void UIContext::activateTopScissor() {
        if (scissorStack_.size()) {
            const Bounds &bounds = scissorStack_.back();
            float scale = 1.0f / g_dpi_scale;
            int x = scale * bounds.x;
            int y = scale * bounds.y;
            int w = scale * bounds.w;
            int h = scale * bounds.h;
            thin3d_->setScissorRect(x, y, w, h);
            thin3d_->setScissorEnabled(true);
        }
        else {
            thin3d_->setScissorEnabled(false);
        }
    }

    void UIContext::setFontScale(float scaleX, float scaleY) {
        fontScaleX_ = scaleX;
        fontScaleY_ = scaleY;
    }

    void UIContext::setFontStyle(const UI::FontStyle &fontStyle) {
        *fontStyle_ = fontStyle;
        if (textDrawer_) {
            textDrawer_->setFontScale(fontScaleX_, fontScaleY_);
            text()->setFont(fontStyle.fontName.c_str(), fontStyle.sizePts, fontStyle.flags);
        }
    }

    void UIContext::measureText(const UI::FontStyle &style, const char *str, float *x, float *y, int align) const {
        measureTextCount(style, str, (int)strlen(str), x, y, align);
    }

    void UIContext::measureTextCount(const UI::FontStyle &style, const char *str, int count, float *x, float *y, int align) const {
        if (!textDrawer_ || (align & FLAG_DYNAMIC_ASCII)) {
            float sizeFactor = (float)style.sizePts / 24.0f;
            draw()->setFontScale(fontScaleX_ * sizeFactor, fontScaleY_ * sizeFactor);
            draw()->measureTextCount(style.atlasFont, str, count, x, y);
        }
        else {
            textDrawer_->setFontScale(fontScaleX_, fontScaleY_);
            std::string subset(str, count);
            textDrawer_->measureString(subset.c_str(), x, y);
        }
    }

    void UIContext::drawText(const char *str, float x, float y, uint32 color, int align) {
        if (!textDrawer_ || (align & FLAG_DYNAMIC_ASCII)) {
            float sizeFactor = (float)fontStyle_->sizePts / 24.0f;
            draw()->setFontScale(fontScaleX_ * sizeFactor, fontScaleY_ * sizeFactor);
            draw()->drawText(fontStyle_->atlasFont, str, x, y, color, align);
        }
        else {
            textDrawer_->setFontScale(fontScaleX_, fontScaleY_);
            textDrawer_->drawString(*draw(), str, x, y, color, align);
            reBindTexture();
        }
    }

    void UIContext::drawTextRect(const char *str, const Bounds &bounds, uint32 color, int align) {
        if (!textDrawer_ || (align & FLAG_DYNAMIC_ASCII)) {
            float sizeFactor = (float)fontStyle_->sizePts / 24.0f;
            draw()->setFontScale(fontScaleX_ * sizeFactor, fontScaleY_ * sizeFactor);
            draw()->drawTextRect(fontStyle_->atlasFont, str, bounds.x, bounds.y, bounds.w, bounds.h, color, align);
        }
        else {
            textDrawer_->setFontScale(fontScaleX_, fontScaleY_);
            textDrawer_->drawStringRect(*draw(), str, bounds, color, align);
            reBindTexture();
        }
    }

    void UIContext::fillRect(const UI::Drawable &drawable, const Bounds &bounds) {
        // Only draw if alpha is non-zero.
        if ((drawable.color & 0xFF000000) == 0)
            return;

        switch (drawable.type) {
        case UI::DRAW_SOLID_COLOR:
            uidrawbuffer_->drawImageStretch(theme->whiteImage, bounds.x, bounds.y, bounds.x2(), bounds.y2(), drawable.color);
            break;
        case UI::DRAW_4GRID:
            uidrawbuffer_->drawImage4Grid(drawable.image, bounds.x, bounds.y, bounds.x2(), bounds.y2(), drawable.color);
            break;
        case UI::DRAW_STRETCH_IMAGE:
            uidrawbuffer_->drawImageStretch(drawable.image, bounds.x, bounds.y, bounds.x2(), bounds.y2(), drawable.color);
            break;
        case UI::DRAW_NOTHING:
            break;
        }
    }
}

