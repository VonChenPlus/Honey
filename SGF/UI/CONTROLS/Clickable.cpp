#include "Clickable.h"

namespace UI
{
    extern bool IsFocusMovementEnabled();
    extern void SetFocusedView(View *view, bool force = false);
    extern bool IsAcceptKeyCode(int keyCode);
    extern bool IsEscapeKeyCode(int keyCode);

    void Clickable::click() {
        EventParams e;
        e.v = this;
        OnClick.trigger(e);
    };

    void Clickable::focusChanged(int focusFlags) {
        if (focusFlags & FF_LOSTFOCUS) {
            down_ = false;
            dragging_ = false;
        }
    }

    void Clickable::touch(const TouchInput &input) {
        if (!isEnabled()) {
            dragging_ = false;
            down_ = false;
            return;
        }

        if (input.flags & TOUCH_DOWN) {
            if (bounds_.contains(input.x, input.y)) {
                if (IsFocusMovementEnabled())
                    SetFocusedView(this);
                dragging_ = true;
                down_ = true;
            }
            else {
                down_ = false;
                dragging_ = false;
            }
        }
        else if (input.flags & TOUCH_MOVE) {
            if (dragging_)
                down_ = bounds_.contains(input.x, input.y);
        }
        if (input.flags & TOUCH_UP) {
            if ((input.flags & TOUCH_CANCEL) == 0 && dragging_ && bounds_.contains(input.x, input.y)) {
                click();
            }
            down_ = false;
            downCountDown_ = 0;
            dragging_ = false;
        }
    }

    bool Clickable::key(const KeyInput &key) {
        if (!hasFocus() && key.deviceId != DEVICE_ID_MOUSE) {
            down_ = false;
            return false;
        }
        // TODO: Replace most of Update with this.
        bool ret = false;
        if (key.flags & KEY_DOWN) {
            if (IsAcceptKeyCode(key.keyCode)) {
                down_ = true;
                ret = true;
            }
        }
        if (key.flags & KEY_UP) {
            if (IsAcceptKeyCode(key.keyCode)) {
                if (down_) {
                    click();
                    down_ = false;
                    ret = true;
                }
            } else if (IsEscapeKeyCode(key.keyCode)) {
                down_ = false;
            }
        }
        return ret;
    }

    ClickableItem::ClickableItem(LayoutParams *layoutParams) : Clickable(layoutParams) {
        if (!layoutParams) {
            if (layoutParams_->width == WRAP_CONTENT)
                layoutParams_->width = FILL_PARENT;
            layoutParams_->height = 64.0f;
        }
    }

    void ClickableItem::getContentDimensions(const UIContext &, float &w, float &h) const {
        w = 0.0f;
        h = 0.0f;
    }

    void ClickableItem::draw(UIContext &dc) {
        Style style = dc.theme->itemStyle;

        if (hasFocus()) {
            style = dc.theme->itemFocusedStyle;
        }
        if (down_) {
            style = dc.theme->itemDownStyle;
        }

        dc.fillRect(style.background, bounds_);
    }
}
