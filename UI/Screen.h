#ifndef SCREEN
#define SCREEN

#include "BASE/BasicTypes.h"

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
        virtual void sendMessage(const char *msg, const char *value) { UNUSED(msg) UNUSED(value)}

        ScreenManager *screenManager() { return screenManager_; }
        void setScreenManager(ScreenManager *sm) { screenManager_ = sm; }

        virtual bool isTransparent() const { return false; }
        virtual bool isTopLevel() const { return false; }

    private:
        ScreenManager *screenManager_;
        DISALLOW_COPY_AND_ASSIGN(Screen)
    };
}

#endif // SCREEN

