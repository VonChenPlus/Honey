#ifndef POPSCREEN_H
#define POPSCREEN_H

#include "UIScreen.h"
#include "CONTROLS/Button.h"

namespace UI
{
    class PopupScreen : public UIDialogScreen
    {
    public:
        PopupScreen(std::string title, std::string button1 = "", std::string button2 = "");

        virtual void createPopupContents(UI::ViewGroup *parent) = 0;
        virtual void createViews() override;
        virtual bool isTransparent() const override { return true; }
        virtual bool touch(const TouchInput &touch) override;
        virtual bool key(const KeyInput &key) override;

    protected:
        virtual bool fillVertical() const { return false; }
        virtual bool showButtons() const { return true; }
        virtual bool onCompleted(DialogResult) { return false;  }

    private:
        UI::EventReturn OnOK(UI::EventParams &e);
        UI::EventReturn OnCancel(UI::EventParams &e);

        UI::ViewGroup *box_;
        UI::Button *defaultButton_;
        std::string title_;
        std::string button1_;
        std::string button2_;
    };
}

#endif // POPSCREEN_H
