#ifndef FONTS_H
#define FONTS_H

#include "BASE/HObject.h"
#include "MATH/Size.h"
#include "GRAPH/BASE/Color.h"
#include "GRAPH/BASE/Types.h"
#include "GRAPH/BASE/EventListener.h"
#include "MATH/Rectangle.h"

namespace GRAPH
{
    class Texture2D;

    struct FontShadow
    {
    public:

        // shadow is not enabled by default
        FontShadow()
            : _shadowEnabled(false)
            , _shadowBlur(0)
            , _shadowOpacity(0)
        {}

        bool   _shadowEnabled;
        MATH::Sizef _shadowOffset;
        float  _shadowBlur;
        float  _shadowOpacity;
    };

    struct FontStroke
    {
    public:
        FontStroke()
            : _strokeEnabled(false)
            , _strokeColor(Color3B::BLACK)
            , _strokeAlpha(255)
            , _strokeSize(0)
        {}

        bool      _strokeEnabled;
        Color3B   _strokeColor;
        GLubyte   _strokeAlpha;
        float     _strokeSize;
    };

    struct FontDefinition
    {
    public:
        FontDefinition()
            : _fontSize(0)
            , _alignment(TextHAlignment::CENTER)
            , _vertAlignment(TextVAlignment::TOP)
            , _dimensions(MATH::SizefZERO)
            , _fontFillColor(Color3B::WHITE)
            , _fontAlpha(255)
        {}

        std::string           _fontName;
        int                   _fontSize;
        TextHAlignment        _alignment;
        TextVAlignment _vertAlignment;
        MATH::Sizef         _dimensions;
        Color3B               _fontFillColor;
        GLubyte               _fontAlpha;
        FontShadow            _shadow;
        FontStroke            _stroke;
    };

    struct FontLetterDefinition
    {
        unsigned short  letteCharUTF16;
        float U;
        float V;
        float width;
        float height;
        float offsetX;
        float offsetY;
        int textureID;
        bool validDefinition;
        int xAdvance;

        int clipBottom;
    };

    enum class GlyphCollection {
        DYNAMIC,
        NEHE,
        ASCII,
        CUSTOM
    };

    typedef struct _ttfConfig
    {
        std::string fontFilePath;
        int fontSize;

        GlyphCollection glyphs;
        const char *customGlyphs;

        bool distanceFieldEnabled;
        int outlineSize;

        _ttfConfig(const char* filePath = "",int size = 12, const GlyphCollection& glyphCollection = GlyphCollection::DYNAMIC,
            const char *customGlyphCollection = nullptr,bool useDistanceField = false,int outline = 0)
            :fontFilePath(filePath)
            ,fontSize(size)
            ,glyphs(glyphCollection)
            ,customGlyphs(customGlyphCollection)
            ,distanceFieldEnabled(useDistanceField)
            ,outlineSize(outline)
        {
            if(outline > 0)
            {
                distanceFieldEnabled = false;
            }
        }
    }TTFConfig;

    class FontAtlas;

    class Font : public HObject
    {
    public:
        virtual  FontAtlas *createFontAtlas() = 0;

        virtual int* getHorizontalKerningForTextUTF16(const std::u16string& text, int &outNumLetters) const = 0;
        virtual const char* getCurrentGlyphCollection() const;


        virtual int getFontMaxHeight() const { return 0; }

    protected:

        Font();
        virtual ~Font() {}
        void setCurrentGlyphCollection(GlyphCollection glyphs, const char *customGlyphs = 0);
        const char * getGlyphCollection(GlyphCollection glyphs) const;


        GlyphCollection     _usedGlyphs;
        char              * _customGlyphs;
        static const char * _glyphASCII;
        static const char * _glyphNEHE;

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

        inline const std::unordered_map<ssize_t, Texture2D*>& getTextures() const{ return _atlasTextures;}
        void  addTexture(Texture2D *texture, int slot);
        float getCommonLineHeight() const;
        void  setCommonLineHeight(float newHeight);

        Texture2D* getTexture(int slot);
        const Font* getFont() const;

        void listenRendererRecreated(EventCustom *event);

        void purgeTexturesAtlas();

         void setAntiAliasTexParameters();

         void setAliasTexParameters();

    protected:
        void relaseTextures();
        std::unordered_map<ssize_t, Texture2D*> _atlasTextures;
        std::unordered_map<unsigned short, FontLetterDefinition> _fontLetterDefinitions;
        float _commonLineHeight;
        Font * _font;

        // Dynamic GlyphCollection related stuff
        int _currentPage;
        unsigned char *_currentPageData;
        int _currentPageDataSize;
        float _currentPageOrigX;
        float _currentPageOrigY;
        float _letterPadding;

        int _fontAscender;
        EventListenerCustom* _rendererRecreatedListener;
        bool _antialiasEnabled;
        int _currLineHeight;
    };

    class FontAtlasCache
    {
    public:
        static FontAtlas * getFontAtlasTTF(const TTFConfig & config);
        static FontAtlas * getFontAtlasFNT(const std::string& fontFileName, const MATH::Vector2f& imageOffset = MATH::Vec2fZERO);

        static FontAtlas * getFontAtlasCharMap(const std::string& charMapFile, int itemWidth, int itemHeight, int startCharMap);
        static FontAtlas * getFontAtlasCharMap(Texture2D* texture, int itemWidth, int itemHeight, int startCharMap);
        static FontAtlas * getFontAtlasCharMap(const std::string& plistFile);

        static bool releaseFontAtlas(FontAtlas *atlas);

        static void purgeCachedData();

    private:
        static std::string generateFontName(const std::string& fontFileName, int size, GlyphCollection theGlyphs, bool useDistanceField);
        static std::unordered_map<std::string, FontAtlas *> _atlasMap;
    };

    class BMFontConfiguration;

    class FontFNT : public Font
    {
    public:
        static FontFNT * create(const std::string& fntFilePath, const MATH::Vector2f& imageOffset = MATH::Vec2fZERO);

        static void purgeCachedData();
        virtual int* getHorizontalKerningForTextUTF16(const std::u16string& text, int &outNumLetters) const override;
        virtual FontAtlas *createFontAtlas() override;

    protected:

        FontFNT(BMFontConfiguration *theContfig, const MATH::Vector2f& imageOffset = MATH::Vec2fZERO);
        virtual ~FontFNT();

    private:
        int  getHorizontalKerningForChars(unsigned short firstChar, unsigned short secondChar) const;

        BMFontConfiguration * _configuration;
        MATH::Vector2f      _imageOffset;

    };

    class FontCharMap : public Font
    {
    public:
        static FontCharMap * create(const std::string& charMapFile, int itemWidth, int itemHeight, int startCharMap);
        static FontCharMap * create(Texture2D* texture, int itemWidth, int itemHeight, int startCharMap);
        static FontCharMap * create(const std::string& plistFile);

        virtual int* getHorizontalKerningForTextUTF16(const std::u16string& text, int &outNumLetters) const override;
        virtual FontAtlas *createFontAtlas() override;

    protected:
        FontCharMap(Texture2D* texture,int itemWidth, int itemHeight, int startCharMap)
            :_texture(texture)
            ,_mapStartChar(startCharMap)
            ,_itemWidth(itemWidth)
            ,_itemHeight(itemHeight)
        {}

        virtual ~FontCharMap();

    private:
        Texture2D* _texture;
        int _mapStartChar;
        int _itemWidth;
        int _itemHeight;

    };
}

#endif // FONTS_H
