#include "LogoScreen.h"
using _INPUT::InputState;
using _INPUT::KeyInput;
using _INPUT::DEVICE_ID_MOUSE;
#include "UI/UIContext.h"
#include "UI/ScreenManager.h"
#include "MATH/Bounds.h"
using MATH::Bounds;
#include "GFX/DrawBuffer.h"
using GFX::DrawBuffer;
using GFX::ALIGN_CENTER;
#include "BASE/Color.h"

namespace GLOBAL
{
    extern DrawBuffer &drawBuffer2D();
}

namespace UI
{
    void LogoScreen::next() {
    }

    void LogoScreen::update(InputState &input_state) {
        UIScreen::update(input_state);
        frames_++;
        if (frames_ > 180 || input_state.pointer_down[0]) {
            next();
        }
    }

    void LogoScreen::sendMessage(const char *message, const char *value) {
    }

    bool LogoScreen::key(const KeyInput &key) {
        if (key.deviceId != DEVICE_ID_MOUSE) {
            next();
            return true;
        }
        return false;
    }

    void LogoScreen::render() {
        UIScreen::render();
        UIContext &dc = *screenManager()->getUIContext();

        const Bounds &bounds = dc.getBounds();

        dc.begin();
        float t = (float)frames_ / 60.0f;

        float alpha = t;
        if (t > 1.0f)
            alpha = 1.0f;
        float alphaText = alpha;
        if (t > 2.0f)
            alphaText = 3.0f - t;

        drawBackground(dc, alpha);

        char temp[256];
        // Manually formatting utf-8 is fun.  \xXX doesn't work everywhere.
        snprintf(temp, sizeof(temp), "Created by Feng Chen%c%crd", 0xC3, 0xA5);
        dc.draw()->setFontScale(1.0f, 1.0f);
        dc.setFontStyle(dc.theme->uiFont);
        dc.drawText(temp, bounds.centerX(), bounds.centerY() + 40, ColorAlpha(0xFFFFFFFF, alphaText), ALIGN_CENTER);

        dc.end();
        dc.flush();
    }

    void LogoScreen::drawBackground(UIContext &, int alpha) {

    }
}