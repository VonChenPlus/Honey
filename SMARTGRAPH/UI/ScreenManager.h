#ifndef SCREENMANAGER_H
#define SCREENMANAGER_H

#include <vector>
#include "SMARTGRAPH/UI/InputState.h"
#include "SMARTGRAPH/UI/UIContext.h"
#include "SMARTGRAPH/THIN3D/Thin3D.h"
#include "SMARTGRAPH/UI/Screen.h"

namespace UI
{
    enum LAYER_FLAG
    {
        LAYER_DEFAULT = 0,
        LAYER_SIDEMENU = 1,
        LAYER_TRANSPARENT = 1 << 1,
    };

    class ScreenManager
    {
    public:
        ScreenManager();
        virtual ~ScreenManager();

        void switchScreen(Screen *screen);
        void update(InputState &input);

        void setUIContext(UIContext *context) { uiContext_ = context; }
        UIContext *getUIContext() { return uiContext_; }

        void setThin3DContext(THIN3D::Thin3DContext *context) { thin3DContext_ = context; }
        THIN3D::Thin3DContext *getThin3DContext() { return thin3DContext_; }

        void render();
        void resized();
        void deviceLost();
        void shutdown();

        // Recreate all views
        void recreateAllViews();

        // Pops the dialog away.
        void finishDialog(Screen *dialog, DialogResult result = DR_OK);

        // Instant touch, separate from the update() mechanism.
        bool touch(const TouchInput &touch);
        bool key(const KeyInput &key);
        bool axis(const AxisInput &touch);

        // Push a dialog box in front. Currently 1-level only.
        void push(Screen *screen, LAYER_FLAG layerFlags = LAYER_DEFAULT);

        // Generic facility for gross hacks :P
        void sendMessage(const char *msg, const char *value);

        Screen *topScreen() const;
    private:
        void pop();
        void switchToNext();
        void processFinishDialog();

        Screen *nextScreen_;
        UIContext *uiContext_;
        THIN3D::Thin3DContext *thin3DContext_;

        const Screen *dialogFinished_;
        DialogResult dialogResult_;

        struct Layer
        {
            Screen *screen;
            LAYER_FLAG flags;  // From LAYER_ enum above
        };

        std::vector<Layer> stack_;
    };
}

#endif // SCREENMANAGER_H

