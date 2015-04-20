#include "UIScreen.h"

#include <set>
#include <queue>

#include "UTILS/TIME/Time.h"
using UTILS::TIME::time_now_d;
#include "UI/KeyCodes.h"
using namespace UI;
#include "UI/ScreenManager.h"

namespace UI
{
    extern void UpdateViewHierarchy(const InputState &input_state, ViewGroup *root);
    extern void LayoutViewHierarchy(const UIContext &dc, ViewGroup *root);
    extern bool TouchEvent(const TouchInput &touch, ViewGroup *root);
    extern bool KeyEvent(const KeyInput &key, ViewGroup *root);
    extern bool AxisEvent(const AxisInput &axis, ViewGroup *root);

    UIScreen::UIScreen()
        : Screen(), root_(0), recreateViews_(true), hatDown_(0) {
    }

    UIScreen::~UIScreen() {
        delete root_;
    }

    void UIScreen::doRecreateViews() {
        if (recreateViews_) {
            delete root_;
            root_ = 0;
            createViews();
            if (root_ && root_->getDefaultFocusView()) {
                root_->getDefaultFocusView()->setFocus();
            }
            recreateViews_ = false;
        }
    }

    void UIScreen::update(InputState &input) {
        doRecreateViews();

        if (root_) {
            UpdateViewHierarchy(input, root_);
        }
    }

    void UIScreen::render() {
        doRecreateViews();

        if (root_) {
            LayoutViewHierarchy(*screenManager()->getUIContext(), root_);

            screenManager()->getUIContext()->begin();
            drawBackground(*screenManager()->getUIContext());
            root_->draw(*screenManager()->getUIContext());
            screenManager()->getUIContext()->end();
            screenManager()->getUIContext()->flush();
        }
    }

    bool UIScreen::touch(const TouchInput &touch) {
        if (root_) {
            TouchEvent(touch, root_);
            return true;
        }
        return false;
    }

    bool UIScreen::key(const KeyInput &key) {
        if (root_) {
            return KeyEvent(key, root_);
        }
        return false;
    }

    bool UIScreen::axis(const AxisInput &axis) {
        // Simple translation of hat to keys for Shield and other modern pads.
        // TODO: Use some variant of keymap?
        int flags = 0;
        if (axis.axisId == JOYSTICK_AXIS_HAT_X) {
            if (axis.value < -0.7f)
                flags |= PAD_BUTTON_LEFT;
            if (axis.value > 0.7f)
                flags |= PAD_BUTTON_RIGHT;
        }
        if (axis.axisId == JOYSTICK_AXIS_HAT_Y) {
            if (axis.value < -0.7f)
                flags |= PAD_BUTTON_UP;
            if (axis.value > 0.7f)
                flags |= PAD_BUTTON_DOWN;
        }

        // Yeah yeah, this should be table driven..
        int pressed = flags & ~hatDown_;
        int released = ~flags & hatDown_;
        if (pressed & PAD_BUTTON_LEFT) key(KeyInput(DEVICE_ID_KEYBOARD, NKCODE_DPAD_LEFT, KEY_DOWN));
        if (pressed & PAD_BUTTON_RIGHT) key(KeyInput(DEVICE_ID_KEYBOARD, NKCODE_DPAD_RIGHT, KEY_DOWN));
        if (pressed & PAD_BUTTON_UP) key(KeyInput(DEVICE_ID_KEYBOARD, NKCODE_DPAD_UP, KEY_DOWN));
        if (pressed & PAD_BUTTON_DOWN) key(KeyInput(DEVICE_ID_KEYBOARD, NKCODE_DPAD_DOWN, KEY_DOWN));
        if (released & PAD_BUTTON_LEFT) key(KeyInput(DEVICE_ID_KEYBOARD, NKCODE_DPAD_LEFT, KEY_UP));
        if (released & PAD_BUTTON_RIGHT) key(KeyInput(DEVICE_ID_KEYBOARD, NKCODE_DPAD_RIGHT, KEY_UP));
        if (released & PAD_BUTTON_UP) key(KeyInput(DEVICE_ID_KEYBOARD, NKCODE_DPAD_UP, KEY_UP));
        if (released & PAD_BUTTON_DOWN) key(KeyInput(DEVICE_ID_KEYBOARD, NKCODE_DPAD_DOWN, KEY_UP));
        hatDown_ = flags;
        if (root_) {
            AxisEvent(axis, root_);
            return true;
        }
        return (pressed & (PAD_BUTTON_LEFT | PAD_BUTTON_RIGHT | PAD_BUTTON_UP | PAD_BUTTON_DOWN)) != 0;
    }

    UI::EventReturn UIScreen::OnBack(UI::EventParams &) {
        screenManager()->finishDialog(this, DR_BACK);
        return UI::EVENT_DONE;
    }

    UI::EventReturn UIScreen::OnOK(UI::EventParams &) {
        screenManager()->finishDialog(this, DR_OK);
        return UI::EVENT_DONE;
    }

    UI::EventReturn UIScreen::OnCancel(UI::EventParams &) {
        screenManager()->finishDialog(this, DR_CANCEL);
        return UI::EVENT_DONE;
    }
}
