#include "Button.h"
#include "GFX/Atlas.h"
using GFX::AtlasImage;
using GFX::ALIGN_CENTER;

namespace UI
{
    void Button::getContentDimensions(const UIContext &dc, float &w, float &h) const {
        if (imageID_ != -1) {
            const AtlasImage *img = &dc.draw()->getAtlas()->images[imageID_];
            w = img->w;
            h = img->h;
        }
        else {
            dc.measureText(dc.theme->uiFont, text_.c_str(), &w, &h);
        }
        // Add some internal padding to not look totally ugly
        w += 16;
        h += 8;
    }

    void Button::draw(UIContext &dc) {
        Style style = dc.theme->buttonStyle;

        if (hasFocus()) style = dc.theme->buttonFocusedStyle;
        if (down_) style = dc.theme->buttonDownStyle;
        if (!isEnabled()) style = dc.theme->buttonDisabledStyle;

        // dc.Draw()->DrawImage4Grid(style.image, bounds_.x, bounds_.y, bounds_.x2(), bounds_.y2(), style.bgColor);
        dc.fillRect(style.background, bounds_);
        float tw, th;
        dc.measureText(dc.theme->uiFont, text_.c_str(), &tw, &th);
        if (tw > bounds_.w || imageID_ != -1) {
            dc.pushScissor(bounds_);
        }
        dc.setFontStyle(dc.theme->uiFont);
        if (imageID_ != -1 && text_.empty()) {
            dc.draw()->drawImage(imageID_, bounds_.centerX(), bounds_.centerY(), 1.0f, 0xFFFFFFFF, ALIGN_CENTER);
        } else if (!text_.empty()) {
            dc.drawText(text_.c_str(), bounds_.centerX(), bounds_.centerY(), style.fgColor, ALIGN_CENTER);
            if (imageID_ != -1) {
                const AtlasImage &img = dc.draw()->getAtlas()->images[imageID_];
                dc.draw()->drawImage(imageID_, bounds_.centerX() - tw / 2 - 5 - img.w/2, bounds_.centerY(), 1.0f, 0xFFFFFFFF, ALIGN_CENTER);
            }
        }
        if (tw > bounds_.w || imageID_ != -1) {
            dc.popScissor();
        }
    }
}
