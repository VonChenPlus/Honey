#ifndef TEXTURE_H
#define TEXTURE_H

#include <string>

#include "BASE/BasicTypes.h"
#include "GFX/GfxResourceHolder.h"

namespace GFX
{
    class Texture : public GfxResourceHolder
    {
    public:
        Texture();
        ~Texture();

        // Deduces format from the filename.
        // If loading fails, will load a 256x256 XOR texture.
        // If filename begins with "gen:", will defer to texture_gen.cpp/h.
        // When format is known, it's fine to use LoadZIM etc directly.
        // Those will NOT auto-fall back to xor texture however!
        bool load(const char *filename);
        void bind(int stage = -1);
        void destroy();

        // PNG from memory buffer
        bool loadPNG(const uint8 *data, Size size, bool genMips = true);
        bool loadZIM(const char *filename);
        bool loadPNG(const char *filename, bool genMips = true);
        bool loadJPEG(const char *filename, bool genMips = true);

        unsigned int handle() const
        {
            return id_;
        }

        virtual void glLost();
        std::string filename() const { return filename_; }

        static void unBind(int stage = -1);

        int width() const { return width_; }
        int height() const { return height_; }

    private:
        bool loadXOR();	// Loads a placeholder texture.

        std::string filename_;
        unsigned int id_;
        int width_, height_;
    };

    typedef int ImageID;

    struct AtlasChar
    {
        // texcoords
        float sx, sy, ex, ey;
        // offset from the origin
        float ox, oy;
        // distance to move the origin forward
        float wx;
        // size in pixels
        unsigned short pw, ph;
    };

    struct AtlasCharRange
    {
        int start;
        int end;
        int start_index;
    };

    struct AtlasFont
    {
        float padding;
        float height;
        float ascend;
        float distslope;
        const AtlasChar *charData;
        const AtlasCharRange *ranges;
        int numRanges;
        const char *name;

        // Returns 0 on no match.
        const AtlasChar *getChar(int utf32) const ;
    };

    struct AtlasImage
    {
        float u1, v1, u2, v2;
        int w, h;
        const char *name;
    };

    struct Atlas
    {
        const char *filename;
        const AtlasFont **fonts;
        int num_fonts;
        const AtlasImage *images;
        int num_images;

        // These are inefficient linear searches, try not to call every frame.
        const AtlasFont *getFontByName(const char *name) const;
        const AtlasImage *getImageByName(const char *name) const;
    };
}

#endif // TEXTURE_H
