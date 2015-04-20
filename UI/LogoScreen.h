#ifndef LOGOSCREEN_H
#define LOGOSCREEN_H

#include "UI/UIScreen.h"
#include "UI/InputState.h"

namespace UI
{
    class LogoScreen : public UIScreen {
    public:
        LogoScreen()
            : frames_(0), switched_(false) {}
        bool key(const KeyInput &key) override;
        virtual void update(InputState &input) override;
        virtual void render() override;
        virtual void sendMessage(const char *message, const char *value) override;
        virtual void createViews() override {}
        virtual void drawBackground(UIContext &, int alpha = 1.0) override;

    private:
        void next();
        int frames_;
        bool switched_;
    };
}

#endif // LOGOSCREEN_H
