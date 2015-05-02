#ifndef CLICKABLE_H
#define CLICKABLE_H

#include "UI/View.h"

namespace UI
{
    // All these light up their background when touched, or have focus.
    class Clickable : public View
    {
    public:
        Clickable(LayoutParams *layoutParams)
            : View(layoutParams), downCountDown_(0), dragging_(false), down_(false){}

        bool key(const KeyInput &input) override;
        void touch(const TouchInput &input) override;

        void focusChanged(int focusFlags) override;

        Event OnClick;

    protected:
        // Internal method that fires on a click. Default behaviour is to trigger
        // the event.
        // Use it for checking/unchecking checkboxes, etc.
        virtual void click();

        int downCountDown_;
        bool dragging_;
        bool down_;
    };
}

#endif // CLICKABLE_H
