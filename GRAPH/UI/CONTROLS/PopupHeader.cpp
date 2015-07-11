#include "PopupHeader.h"
#include "GRAPH/GFX/Atlas.h"
using GFX::ALIGN_LEFT;
using GFX::ALIGN_VCENTER;

namespace UI
{
    void PopupHeader::draw(UIContext &dc) {
        dc.setFontStyle(dc.theme->uiFont);
        dc.drawText(text_.c_str(), bounds_.left + 12, bounds_.centerY(), dc.theme->popupTitle.fgColor, ALIGN_LEFT | ALIGN_VCENTER);
        dc.draw()->drawImageStretch(dc.theme->whiteImage, bounds_.left, bounds_.bottom()-2, bounds_.right(), bounds_.bottom(), dc.theme->popupTitle.fgColor);
    }
}
