#ifndef UI_H
#define UI_H

#include "INPUT/InputState.h"

namespace UI
{
    // "Mouse" out of habit, applies just as well to touch events.
    // TODO: Change to "pointer"
    // This struct is zeroed on init, so should be valid at that state.
    // Never inherit from this.
    struct UIState
    {
        int mousex[MAX_POINTERS];
        int mousey[MAX_POINTERS];
        bool mousedown[MAX_POINTERS];
        bool mousepressed[MAX_POINTERS];
        short mouseframesdown[MAX_POINTERS];

        int mouseStartX[MAX_POINTERS];
        int mouseStartY[MAX_POINTERS];

        int hotitem[MAX_POINTERS];
        int activeitem[MAX_POINTERS];

        // keyboard focus, not currently used
        int kbdwidget;
        int lastwidget;

        int ui_tick;

        // deprecated: tempfloat
        float tempfloat;
    };
}

#endif // UI_H

