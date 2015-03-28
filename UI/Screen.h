#ifndef SCREEN
#define SCREEN

#include "BASE/BasicTypes.h"
#include "INPUT/InputState.h"
using Input::TouchInput;
using Input::KeyInput;
using Input::AxisInput;

namespace UI
{
    class ScreenManager;

    class Screen
    {
    public:
        Screen() : screenManager_(NULLPTR) { }
        virtual ~Screen()
        {
            screenManager_ = NULLPTR;
        }

        virtual void update() {}
        virtual void render() {}
        virtual void resized() {}

        virtual bool touch(const TouchInput &touch) { UNUSED(touch) return false;  }
        virtual bool key(const KeyInput &key) { UNUSED(key) return false; }
        virtual bool axis(const AxisInput &touch) { UNUSED(touch) return false; }

        virtual void sendMessage(const char *msg, const char *value) { UNUSED(msg) UNUSED(value)}

        ScreenManager *screenManager() { return screenManager_; }
        void setScreenManager(ScreenManager *sm) { screenManager_ = sm; }

        virtual bool isTransparent() const { return false; }
        virtual bool isTopLevel() const { return false; }

        DISALLOW_COPY_AND_ASSIGN(Screen)
    private:
        ScreenManager *screenManager_;
    };
}

#endif // SCREEN

