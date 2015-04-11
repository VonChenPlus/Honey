#ifndef TEXTURE_H
#define TEXTURE_H

#include <string>

#include "BASE/BasicTypes.h"
#include "GFX/GfxResourceHolder.h"
#include "GFX/Atlas.h"

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
        void load(const char *filename);
        void bind(int stage = -1);
        void destroy();

        // PNG from memory buffer
        void loadPNG(const uint8 *data, Size size, bool genMips = true);
        void loadZIM(const char *filename);
        void loadPNG(const char *filename, bool genMips = true);
        void loadJPEG(const char *filename, bool genMips = true);
        void loadXOR();	// Loads a placeholder texture.

        unsigned int handle() const {
            return id_;
        }

        virtual void glLost();
        std::string filename() const { return filename_; }

        static void unBind(int stage = -1);

        int width() const { return width_; }
        int height() const { return height_; }

    private:
        std::string filename_;
        unsigned int id_;
        int width_, height_;
    };

    typedef int ImageID;
}

#endif // TEXTURE_H
