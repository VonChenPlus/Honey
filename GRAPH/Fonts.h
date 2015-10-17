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
    class GLTexture;

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

        GlyphCollection     _usedGlyphs;
        char              * _customGlyphs;
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

        inline const std::unordered_map<uint64, GLTexture*>& getTextures() const{ return _atlasTextures;}
        void  addTexture(GLTexture *texture, int slot);
        float getCommonLineHeight() const;
        void  setCommonLineHeight(float newHeight);

        GLTexture* getTexture(int slot);
        const Font* getFont() const;

        void listenRendererRecreated(EventCustom *event);

        void purgeTexturesAtlas();

        void renderCharAt(unsigned char *dest,int posX, int posY, unsigned char* bitmap,long bitmapWidth,long bitmapHeight);

        void setAntiAliasTexParameters();

        void setAliasTexParameters();

    protected:
        void relaseTextures();

    private:
        std::unordered_map<uint64, GLTexture*> _atlasTextures;
        std::unordered_map<unsigned short, FontLetterDefinition> _letterDefinitions;
        float _commonLineHeight;
        Font * _font;

        // Dynamic GlyphCollection related stuff
        int _currentPage;
        unsigned char *_currentPageData;
        int _currentPageDataSize;
        float _currentPageOrigX;
        float _currentPageOrigY;
        float _letterPadding;

        EventListenerCustom* _rendererRecreatedListener;
        bool _antialiasEnabled;
        int _currLineHeight;

        friend class Label;
    };

    class FontAtlasCache
    {
    public:
        static bool releaseFontAtlas(FontAtlas *atlas);
        static void purgeCachedData();

    private:
        static std::string generateFontName(const std::string& fontFileName, int size, GlyphCollection theGlyphs, bool useDistanceField);
        static std::unordered_map<std::string, FontAtlas *> _atlasMap;
    };
}

#endif // FONTS_H
