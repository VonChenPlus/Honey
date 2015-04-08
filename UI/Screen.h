#ifndef SCREEN_H
#define SCREEN_H

#include "BASE/BasicTypes.h"
#include "INPUT/InputState.h"

namespace UI
{
    class ScreenManager;

    enum DialogResult
    {
        DR_OK,
        DR_CANCEL,
        DR_YES,
        DR_NO,
        DR_BACK,
    };

    class Screen
    {
    public:
        Screen() : screenManager_(NULLPTR) { }
        virtual ~Screen() {
            screenManager_ = NULLPTR;
        }

        virtual void onFinish(DialogResult) {}
        virtual void update(_INPUT::InputState &) {}
        virtual void render() {}
        virtual void resized() {}
        virtual void dialogFinished(const Screen *, DialogResult) {}
        virtual bool touch(const _INPUT::TouchInput &) { return false;  }
        virtual bool key(const _INPUT::KeyInput &) { return false; }
        virtual bool axis(const _INPUT::AxisInput &) { return false; }

        virtual void sendMessage(const char *msg, const char *value) { UNUSED(msg); UNUSED(value); }

        virtual void recreateViews() {}

        ScreenManager *screenManager() { return screenManager_; }
        void setScreenManager(ScreenManager *sm) { screenManager_ = sm; }

        // This one is icky to use because you can't know what's in it until you know
        // what screen it is.
        virtual void *dialogData() { return 0; }

        virtual std::string tag() const { return std::string(""); }

        virtual bool isTransparent() const { return false; }
        virtual bool isTopLevel() const { return false; }

        DISALLOW_COPY_AND_ASSIGN(Screen)
    private:
        ScreenManager *screenManager_;
    };
}

#endif // SCREEN_H

