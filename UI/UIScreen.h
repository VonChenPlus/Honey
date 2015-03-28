#ifndef UISCREEN
#define UISCREEN

#include "UI/Screen.h"

namespace UI
{
    class UIScreen : public Screen
    {
    public:
        UIScreen();
        ~UIScreen();

        virtual void update(InputState &input) override;
        virtual void render() override;

    protected:
        virtual void createViews() = 0;
        virtual void drawBackground(UIContext &dc) {}

        virtual void recreateViews() { recreateViews_ = true; }

        //UI::ViewGroup *root_;

    private:
        void doRecreateViews();

        bool recreateViews_;
    };
}

#endif // UISCREEN

