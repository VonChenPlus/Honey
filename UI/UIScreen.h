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

        virtual void update(UI::InputState &input) override;
        virtual void render() override;

        virtual bool touch(const UI::TouchInput &touch) override;
        virtual bool key(const UI::KeyInput &touch) override;
        virtual bool axis(const UI::AxisInput &touch) override;

        // Some useful default event handlers
        EventReturn OnOK(EventParams &e);
        EventReturn OnCancel(EventParams &e);
        EventReturn OnBack(EventParams &e);

    protected:
        virtual void createViews() = 0;
        virtual void drawBackground(UIContext &, int alpha = 1.0) { UNUSED(alpha); }

        virtual void recreateViews() { recreateViews_ = true; }

        UI::ViewGroup *root_;

    private:
        void doRecreateViews();

        bool recreateViews_;

        int hatDown_;
    };
}

#endif // UISCREEN_H
