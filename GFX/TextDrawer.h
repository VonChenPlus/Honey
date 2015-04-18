#ifndef DRAWTEXT
#define DRAWTEXT

#include <map>

#include "BASE/BasicTypes.h"
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
        virtual ~TextDrawer();

        uint32 setFont(const char *fontName, int size, int flags);
        void setFontScale(float xscale, float yscale);

        void measureString(const char *str, float *w, float *h);

        void drawString(DrawBuffer &target, const char *str, float x, float y, uint32 color, int align = ALIGN_TOPLEFT);
        void drawStringRect(DrawBuffer &target, const char *str, const MATH::Bounds &bounds, uint32 color, int align);
        // Use for housekeeping like throwing out old strings.
        void oncePerFrame();

    protected:
        virtual TextDrawerFontContext *setFont(int size) = 0;  // Shortcut once you've set the font once.
        virtual void measureString(TextDrawerFontContext * font, const char *str, float *w, float *h) = 0;
        virtual void drawString(TextDrawerFontContext * font, const char *str, uint32 color, TextStringEntry **entry) = 0;
        THIN3D::Thin3DTexture *createTexture(uint16 *bitmapData, int width, int height);

        std::map<uint32, TextDrawerFontContext *> fontMap_;

    private:
        THIN3D::Thin3DContext *thin3d_;

        int frameCount_;
        float fontScaleX_;
        float fontScaleY_;

        TextDrawerContext *ctx_;

        uint32 fontHash_;
        // The key is the CityHash of the string xor the fontHash_.
        std::map<uint32, TextStringEntry *> cache_;
    };
}

#endif // DRAWTEXT

