#ifndef BUTTON_H
#define BUTTON_H

#include "GRAPH/UI/CONTROLS/Clickable.h"
#include "GRAPH/GFX/Texture2D.h"

namespace UI
{
    class Button : public Clickable
    {
    public:
        Button(const std::string &text, LayoutParams *layoutParams = 0)
            : Clickable(layoutParams), text_(text), imageID_(-1) {}
        Button(GFX::ImageID imageID, LayoutParams *layoutParams = 0)
            : Clickable(layoutParams), imageID_(imageID) {}
        Button(const std::string &text, GFX::ImageID imageID, LayoutParams *layoutParams = 0)
            : Clickable(layoutParams), text_(text), imageID_(imageID) {}

        void draw(UIContext &dc) override;
        void getContentDimensions(const UIContext &dc, float &w, float &h) const override;
        const std::string &getText() const { return text_; }

    private:
        Style style_;
        std::string text_;
        GFX::ImageID imageID_;
    };
}

#endif // BUTTON_H
