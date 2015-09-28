#ifndef SCREEN_H
#define SCREEN_H

#include "BASE/Honey.h"
#include "GRAPH/UI/InputState.h"

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
        Screen() : screenManager_(nullptr) { }
        virtual ~Screen() {
            screenManager_ = nullptr;
        }

        virtual void onFinish(DialogResult) {}
        virtual void update(InputState &) {}
        virtual void render() {}
        virtual void resized() {}
        virtual void dialogFinished(const Screen *, DialogResult) {}
        virtual bool touch(const TouchInput &) { return false;  }
        virtual bool key(const KeyInput &) { return false; }
        virtual bool axis(const AxisInput &) { return false; }

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

