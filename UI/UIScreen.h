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

        virtual void update(_INPUT::InputState &input) override;
        virtual void render() override;

        virtual bool touch(const _INPUT::TouchInput &touch) override;
        virtual bool key(const _INPUT::KeyInput &touch) override;
        virtual bool axis(const _INPUT::AxisInput &touch) override;

        // Some useful default event handlers
        EventReturn OnOK(EventParams &e);
        EventReturn OnCancel(EventParams &e);
        EventReturn OnBack(EventParams &e);

    protected:
        virtual void createViews() = 0;
        virtual void drawBackground(UIContext &) {}

        virtual void recreateViews() { recreateViews_ = true; }

        UI::ViewGroup *root_;

    private:
        void doRecreateViews();

        bool recreateViews_;

        int hatDown_;
    };
}

#endif // UISCREEN_H
