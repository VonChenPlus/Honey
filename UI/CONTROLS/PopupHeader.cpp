#include "PopupHeader.h"
#include "GFX/Atlas.h"
using GFX::ALIGN_LEFT;
using GFX::ALIGN_VCENTER;

namespace UI
{
    void PopupHeader::draw(UIContext &dc) {
        dc.setFontStyle(dc.theme->uiFont);
        dc.drawText(text_.c_str(), bounds_.x + 12, bounds_.centerY(), dc.theme->popupTitle.fgColor, ALIGN_LEFT | ALIGN_VCENTER);
        dc.draw()->drawImageStretch(dc.theme->whiteImage, bounds_.x, bounds_.y2()-2, bounds_.x2(), bounds_.y2(), dc.theme->popupTitle.fgColor);
    }
}
