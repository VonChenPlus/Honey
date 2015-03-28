#ifndef SCREENMANAGER
#define SCREENMANAGER

#include <vector>

namespace UI
{
    class Screen;

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
        void update();

        void render();
        void resized();
        void shutdown();

        // Push a dialog box in front. Currently 1-level only.
        void push(Screen *screen, LAYER_FLAG layerFlags = LAYER_DEFAULT);

        // Generic facility for gross hacks :P
        void sendMessage(const char *msg, const char *value);

    private:
        void pop();
        void switchToNext();

        Screen *nextScreen_;

        struct Layer
        {
            Screen *screen;
            LAYER_FLAG flags;  // From LAYER_ enum above
        };

        std::vector<Layer> stack_;
    };
}

#endif // SCREENMANAGER

