#include "UIScreen.h"

#include <set>
#include <queue>

#include "UTILS/TIME/Time.h"
using UTILS::TIME::time_now_d;
using UTILS::TIME::time_now;
#include "UI/KeyCodes.h"
#include "UI/ScreenManager.h"
#include "UTILS/RANDOM/GMRandom.h"
using UTILS::RANDOM::GMRandom;
#include "UTILS/COLOR/Color.h"
using UTILS::COLOR::ColorAlpha;

namespace UI
{
    extern void UpdateViewHierarchy(const InputState &input_state, ViewGroup *root);
    extern void LayoutViewHierarchy(const UIContext &dc, ViewGroup *root);
    extern bool TouchEvent(const TouchInput &touch, ViewGroup *root);
    extern bool KeyEvent(const KeyInput &key, ViewGroup *root);
    extern bool AxisEvent(const AxisInput &axis, ViewGroup *root);
    extern bool IsEscapeKeyCode(int keyCode);

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

    EventReturn UIScreen::OnBack(EventParams &) {
        screenManager()->finishDialog(this, DR_BACK);
        return EVENT_DONE;
    }

    EventReturn UIScreen::OnOK(EventParams &) {
        screenManager()->finishDialog(this, DR_OK);
        return EVENT_DONE;
    }

    EventReturn UIScreen::OnCancel(EventParams &) {
        screenManager()->finishDialog(this, DR_CANCEL);
        return EVENT_DONE;
    }

    static const int symbols[4] = {
        I_H,
        I_I,
        I_V,
        I_E
    };

    static const uint32_t colors[4] = {
        0xC0FFFFFF,
        0xC0FFFFFF,
        0xC0FFFFFF,
        0xC0FFFFFF,
    };

    void UIScreenWithBackground::drawBackground(UIContext &dc, int alpha) {
        static float xbase[100] = {0};
        static float ybase[100] = {0};
        float xres = dc.getBounds().w;
        float yres = dc.getBounds().h;
        static int last_xres = 0;
        static int last_yres = 0;

        if (xbase[0] == 0.0f || last_xres != xres || last_yres != yres) {
            GMRandom rng;
            for (int i = 0; i < 100; i++) {
                xbase[i] = rng.randFloat() * xres;
                ybase[i] = rng.randFloat() * yres;
            }
            last_xres = xres;
            last_yres = yres;
        }

        int img = I_BG;
        dc.draw()->drawImageStretch(img, dc.getBounds());
        float t = time_now();
        for (int i = 0; i < 100; i++) {
            float x = xbase[i] + dc.getBounds().x;
            float y = ybase[i] + dc.getBounds().y + 40 * cosf(i * 7.2f + t * 1.3f);
            float angle = sinf(i + t);
            int n = i & 3;
            dc.draw()->drawImageRotated(symbols[n], x, y, 1.0f, angle, ColorAlpha(colors[n], alpha * 0.1f));
        }

        dc.flush();
    }

    bool UIDialogScreen::key(const KeyInput &key) {
        bool retval = UIScreen::key(key);
        if (!retval && (key.flags & KEY_DOWN) && IsEscapeKeyCode(key.keyCode)) {
            if (finished_) {
                throw _NException_Normal("Screen already finished");
            } else {
                finished_ = true;
                screenManager()->finishDialog(this, DR_BACK);
            }
            return true;
        }
        return retval;
    }

    void UIDialogScreenWithBackground::drawBackground(UIContext &dc, int alpha) {
        UIScreenWithBackground::drawBackground(dc, alpha);
    }
}
