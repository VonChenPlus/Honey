#ifndef INERTVIEW_H
#define INERTVIEW_H

#include "UI/View.h"

namespace UI
{
    class PopupHeader : public Item
    {
    public:
        PopupHeader(const std::string &text, LayoutParams *layoutParams = 0)
            : Item(layoutParams), text_(text) {
                layoutParams_->width = FILL_PARENT;
                layoutParams_->height = 64;
        }
        void draw(UIContext &dc) override;
    private:
        std::string text_;
    };
}

#endif // INERTVIEW_H
