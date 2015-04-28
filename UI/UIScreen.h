#ifndef UISCREEN_H
#define UISCREEN_H

#include "UI/Screen.h"
#include "UI/ViewGroup.h"

namespace UI
{
    class UIScreen : public Screen
    {
    public:
        UIScreen();
        ~UIScreen();

        virtual void update(InputState &input) override;
        virtual void render() override;

        virtual bool touch(const TouchInput &touch) override;
        virtual bool key(const KeyInput &touch) override;
        virtual bool axis(const AxisInput &touch) override;

        // Some useful default event handlers
        EventReturn OnOK(EventParams &e);
        EventReturn OnCancel(EventParams &e);
        EventReturn OnBack(EventParams &e);

    protected:
        virtual void createViews() = 0;
        virtual void drawBackground(UIContext &, int alpha = 1.0) = 0;

        virtual void recreateViews() { recreateViews_ = true; }

        ViewGroup *root_;

    private:
        void doRecreateViews();

        bool recreateViews_;

        int hatDown_;
    };

    class UIScreenWithBackground : public UIScreen {
    public:
        UIScreenWithBackground() : UIScreen() {}
    protected:
        virtual void drawBackground(UIContext &, int alpha = 1.0) override;
    };

    class UIDialogScreen : public UIScreen
    {
    public:
        UIDialogScreen() : UIScreen(), finished_(false) {}
        virtual bool key(const KeyInput &key) override;

    private:
        bool finished_;
    };

    class UIDialogScreenWithBackground : public UIDialogScreen, public UIScreenWithBackground {
    public:
        UIDialogScreenWithBackground() : UIDialogScreen() {}
    protected:
        virtual void drawBackground(UIContext &dc, int alpha = 1.0) override;
    };
}

#endif // UISCREEN_H
