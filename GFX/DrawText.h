#ifndef DRAWTEXT
#define DRAWTEXT

#include <map>

#include "THIN3D/Thin3D.h"
#include "GFX/DrawBuffer.h"
#include "MATH/Bounds.h"

namespace GFX
{
    struct TextStringEntry
    {
        THIN3D::Thin3DTexture *texture;
        int width;
        int height;
        int bmWidth;
        int bmHeight;
        int lastUsedFrame;
    };

    // Not yet functional
    enum
    {
        FONTSTYLE_BOLD = 1,
        FONTSTYLE_ITALIC = 2,
    };

    // Internal struct but all details in .cpp file (pimpl to avoid pulling in excessive headers here)
    struct TextDrawerContext;
    struct TextDrawerFontContext;

    class TextDrawer
    {
    public:
        TextDrawer(THIN3D::Thin3DContext *thin3d);
        ~TextDrawer();

        uint32_t setFont(const char *fontName, int size, int flags);
        void setFont(uint32_t fontHandle);  // Shortcut once you've set the font once.

        void setFontScale(float xscale, float yscale);
        void measureString(const char *str, float *w, float *h);
        void drawString(DrawBuffer &target, const char *str, float x, float y, uint32_t color, int align = ALIGN_TOPLEFT);
        void drawStringRect(DrawBuffer &target, const char *str, const MATH::Bounds &bounds, uint32_t color, int align);
        // Use for housekeeping like throwing out old strings.
        void oncePerFrame();

    private:
        THIN3D::Thin3DContext *thin3d_;

        int frameCount_;
        float fontScaleX_;
        float fontScaleY_;

        TextDrawerContext *ctx_;
        std::map<uint32_t, TextDrawerFontContext *> fontMap_;

        uint32_t fontHash_;
        // The key is the CityHash of the string xor the fontHash_.
        std::map<uint32_t, TextStringEntry *> cache_;
    };
}

#endif // DRAWTEXT

