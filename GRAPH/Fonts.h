#ifndef FONTS_H
#define FONTS_H

#include "BASE/HObject.h"
#include "GRAPH/Color.h"
#include "GRAPH/Types.h"
#include "GRAPH/EventListener.h"
#include "MATH/Size.h"
#include "MATH/Rectangle.h"

namespace GRAPH
{
    class Unity3DTexture;
    namespace UI
    {
        class Label;
    }

    struct FontLetterDefinition
    {
        unsigned short  letteCharUTF16;
        float U;
        float V;
        float width;
        float height;
        int textureID;
    };

    enum class GlyphCollection {
        DYNAMIC,
        CUSTOM
    };

    class FontAtlas;

    class Font : public HObject
    {
    public:
        virtual  FontAtlas *createFontAtlas() = 0;

        virtual int* getHorizontalKerningForTextUTF16(const std::u16string& text, int &outNumLetters) const = 0;
        virtual const char* getCurrentGlyphCollection() const;
        virtual int getFontMaxHeight() const { return 0; }

        virtual unsigned char* getGlyphBitmap(unsigned char theChar, long &outWidth, long &outHeight) = 0;

    protected:

        Font();
        virtual ~Font() {}
        void setCurrentGlyphCollection(GlyphCollection glyphs, const char *customGlyphs = 0);

        GlyphCollection     usedGlyphs_;
        char              * customGlyphs_;
    };

    class FontAtlas : public HObject
    {
    public:
        static const int CacheTextureWidth;
        static const int CacheTextureHeight;
        static const char* CMD_PURGE_FONTATLAS;
        static const char* CMD_RESET_FONTATLAS;

        FontAtlas(Font &theFont);
        virtual ~FontAtlas();

        void addLetterDefinition(const FontLetterDefinition &letterDefinition);
        bool getLetterDefinitionForChar(char16_t letteCharUTF16, FontLetterDefinition &outDefinition);

        bool prepareLetterDefinitions(const std::u16string& utf16String);

        inline const std::unordered_map<uint64, Unity3DTexture*>& getTextures() const{ return atlasTextures_;}
        void  addTexture(Unity3DTexture *texture, int slot);
        float getCommonLineHeight() const;
        void  setCommonLineHeight(float newHeight);

        Unity3DTexture* getTexture(int slot);
        const Font* getFont() const;

        void listenRendererRecreated(EventCustom *event);

        void purgeTexturesAtlas();

        void renderCharAt(unsigned char *dest,int posX, int posY, unsigned char* bitmap,long bitmapWidth,long bitmapHeight);

        void setAliasTexParameters();

    protected:
        void relaseTextures();

    private:
        std::unordered_map<uint64, Unity3DTexture*> atlasTextures_;
        std::unordered_map<unsigned short, FontLetterDefinition> letterDefinitions_;
        float commonLineHeight_;
        Font * font_;

        int currentPage_;
        unsigned char *currentPageData_;
        int currentPageDataSize_;
        float currentPageOrigX_;
        float currentPageOrigY_;
        float letterPadding_;

        EventListenerCustom* rendererRecreatedListener_;
        bool antialiasEnabled_;
        int currLineHeight_;

        friend class UI::Label;
    };

    class FontAtlasCache
    {
    public:
        static bool releaseFontAtlas(FontAtlas *atlas);
        static void purgeCachedData();

    private:
        static std::string generateFontName(const std::string& fontFileName, int size, GlyphCollection theGlyphs, bool useDistanceField);
        static std::unordered_map<std::string, FontAtlas *> atlasMap_;
    };
}

#endif // FONTS_H
