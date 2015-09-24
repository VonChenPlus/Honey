#include <string.h>
#include "GRAPH/Fonts.h"
#include "GRAPH/Director.h"
#include "GRAPH/EventDispatcher.h"
#include "GRAPH/RENDERER/Texture2D.h"
#include "IO/FileUtils.h"
#include "UTILS/STRING/StringUtils.h"
#include "UTILS/STRING/UTFUtils.h"

namespace GRAPH
{
    Font::Font()
        : _usedGlyphs(GlyphCollection::DYNAMIC)
        , _customGlyphs(nullptr) {

    }

    void Font::setCurrentGlyphCollection(GlyphCollection glyphs, const char *customGlyphs) {
        if (_customGlyphs)
            delete [] _customGlyphs;

        if (customGlyphs) {
            size_t length = strlen(customGlyphs);
            _customGlyphs = new char [length + 2];
            memcpy(_customGlyphs, customGlyphs, length);

            _customGlyphs[length]   = 0;
            _customGlyphs[length+1] = 0;
        }

        _usedGlyphs = glyphs;
    }

    const char *Font::getCurrentGlyphCollection() const {
        if (_customGlyphs) {
            return _customGlyphs;
        }
        else {
            return 0;
        }
    }

    const int FontAtlas::CacheTextureWidth = 512;
    const int FontAtlas::CacheTextureHeight = 512;
    const char* FontAtlas::CMD_PURGE_FONTATLAS = "_PURGE_FONTATLAS";
    const char* FontAtlas::CMD_RESET_FONTATLAS = "_RESET_FONTATLAS";

    FontAtlas::FontAtlas(Font &theFont)
        : _font(&theFont)
        , _currentPageData(nullptr)
        , _rendererRecreatedListener(nullptr)
        , _antialiasEnabled(true)
        , _currLineHeight(0) {

        _font->retain();
        Font* fontTTf = _font;
        if (fontTTf) {
            _commonLineHeight = _font->getFontMaxHeight();
            auto texture = new (std::nothrow) Texture2D;
            _currentPage = 0;
            _currentPageOrigX = 0;
            _currentPageOrigY = 0;
            _letterPadding = 0;
            _currentPageDataSize = CacheTextureWidth * CacheTextureHeight;
            _currentPageData = new unsigned char[_currentPageDataSize];
            memset(_currentPageData, 0, _currentPageDataSize);

            auto  pixelFormat = IMAGE::PixelFormat::A8;
            texture->initWithData(_currentPageData, _currentPageDataSize,
                pixelFormat, CacheTextureWidth, CacheTextureHeight, MATH::Sizef(CacheTextureWidth,CacheTextureHeight) );

            addTexture(texture,0);
            texture->release();
        }
    }

    FontAtlas::~FontAtlas() {
        _font->release();
        relaseTextures();

        delete []_currentPageData;
    }

    void FontAtlas::relaseTextures() {
        for( auto &item: _atlasTextures) {
            item.second->release();
        }
        _atlasTextures.clear();
    }

    void FontAtlas::purgeTexturesAtlas() {
        Font* fontTTf = _font;
        if (fontTTf && _atlasTextures.size() > 1) {
            auto eventDispatcher = Director::getInstance().getEventDispatcher();
            eventDispatcher->dispatchCustomEvent(CMD_PURGE_FONTATLAS,this);
            eventDispatcher->dispatchCustomEvent(CMD_RESET_FONTATLAS,this);
        }
    }

    void FontAtlas::listenRendererRecreated(EventCustom *) {
        Font* fontTTf = _font;
        if (fontTTf) {
            auto eventDispatcher = Director::getInstance().getEventDispatcher();
            eventDispatcher->dispatchCustomEvent(CMD_PURGE_FONTATLAS,this);
            eventDispatcher->dispatchCustomEvent(CMD_RESET_FONTATLAS,this);
        }
    }

    void FontAtlas::addLetterDefinition(const FontLetterDefinition &letterDefinition) {
        _fontLetterDefinitions[letterDefinition.letteCharUTF16] = letterDefinition;
    }

    bool FontAtlas::getLetterDefinitionForChar(char16_t letteCharUTF16, FontLetterDefinition &outDefinition) {
        auto outIterator = _fontLetterDefinitions.find(letteCharUTF16);

        if (outIterator != _fontLetterDefinitions.end()) {
            outDefinition = (*outIterator).second;
            return true;
        }
        else {
            return false;
        }
    }

    bool FontAtlas::prepareLetterDefinitions(const std::u16string& utf16String) {
        Font* fontTTf = _font;
        if(fontTTf == nullptr)
            return false;

        size_t length = utf16String.length();
        long bitmapWidth;
        long bitmapHeight;
        FontLetterDefinition tempDef;

        auto  pixelFormat = IMAGE::PixelFormat::A8;

        bool existNewLetter = false;
        float startY = _currentPageOrigY;

        for (size_t i = 0; i < length; ++i)
        {
            auto outIterator = _fontLetterDefinitions.find(utf16String[i]);

            if (outIterator == _fontLetterDefinitions.end())
            {
                existNewLetter = true;

                auto bitmap = fontTTf->getGlyphBitmap(utf16String[i],bitmapWidth,bitmapHeight);
                if (bitmap)
                {
                    tempDef.letteCharUTF16   = utf16String[i];
                    tempDef.width            = bitmapWidth + _letterPadding;
                    tempDef.height           = bitmapHeight + _letterPadding;

                    if (bitmapHeight > _currLineHeight)
                    {
                        _currLineHeight = bitmapHeight + 1;
                    }
                    if (_currentPageOrigX + tempDef.width > CacheTextureWidth)
                    {
                        _currentPageOrigY += _currLineHeight;
                        _currLineHeight = 0;
                        _currentPageOrigX = 0;
                        if(_currentPageOrigY + _commonLineHeight >= CacheTextureHeight)
                        {
                            unsigned char *data = nullptr;
                            if(pixelFormat == IMAGE::PixelFormat::AI88)
                            {
                                data = _currentPageData + CacheTextureWidth * (int)startY * 2;
                            }
                            else
                            {
                                data = _currentPageData + CacheTextureWidth * (int)startY;
                            }
                            _atlasTextures[_currentPage]->updateWithData(data, 0, startY,
                                CacheTextureWidth, CacheTextureHeight - startY);

                            startY = 0.0f;

                            _currentPageOrigY = 0;
                            memset(_currentPageData, 0, _currentPageDataSize);
                            _currentPage++;
                            auto tex = new (std::nothrow) Texture2D;
                            if (_antialiasEnabled)
                            {
                                tex->setAntiAliasTexParameters();
                            }
                            else
                            {
                                tex->setAliasTexParameters();
                            }
                            tex->initWithData(_currentPageData, _currentPageDataSize,
                                pixelFormat, CacheTextureWidth, CacheTextureHeight, MATH::Sizef(CacheTextureWidth,CacheTextureHeight) );
                            addTexture(tex,_currentPage);
                            tex->release();
                        }
                    }

                    renderCharAt(_currentPageData,_currentPageOrigX,_currentPageOrigY,bitmap,bitmapWidth,bitmapHeight);

                    tempDef.U                = _currentPageOrigX;
                    tempDef.V                = _currentPageOrigY;
                    tempDef.textureID        = _currentPage;
                    _currentPageOrigX        += tempDef.width + 1;
                    // take from pixels to points
                    tempDef.width  =    tempDef.width;
                    tempDef.height =    tempDef.height;
                    tempDef.U      =    tempDef.U;
                    tempDef.V      =    tempDef.V;
                }
                else {

                    tempDef.letteCharUTF16   = utf16String[i];
                    tempDef.width            = 0;
                    tempDef.height           = 0;
                    tempDef.U                = 0;
                    tempDef.V                = 0;
                    tempDef.textureID        = 0;
                    _currentPageOrigX += 1;
                }

                _fontLetterDefinitions[tempDef.letteCharUTF16] = tempDef;
            }
        }

        if(existNewLetter) {
            unsigned char *data = nullptr;
            if(pixelFormat == IMAGE::PixelFormat::AI88) {
                data = _currentPageData + CacheTextureWidth * (int)startY * 2;
            }
            else {
                data = _currentPageData + CacheTextureWidth * (int)startY;
            }
            _atlasTextures[_currentPage]->updateWithData(data, 0, startY,
                                                         CacheTextureWidth, _currentPageOrigY - startY + _commonLineHeight);
        }
        return true;
    }

    void FontAtlas::renderCharAt(unsigned char *dest,int posX, int posY, unsigned char* bitmap,long bitmapWidth,long bitmapHeight) {
        int iX = posX;
        int iY = posY;

        for (long y = 0; y < bitmapHeight; ++y) {
            long bitmap_y = y * bitmapWidth;
            for (int x = 0; x < bitmapWidth; ++x) {
                unsigned char cTemp = bitmap[bitmap_y + x];
                dest[(iX + ( iY * FontAtlas::CacheTextureWidth ) )] = cTemp;
                iX += 1;
            }

            iX  = posX;
            iY += 1;
        }
    }

    void FontAtlas::addTexture(Texture2D *texture, int slot) {
        texture->retain();
        _atlasTextures[slot] = texture;
    }

    Texture2D* FontAtlas::getTexture(int slot) {
        return _atlasTextures[slot];
    }

    float FontAtlas::getCommonLineHeight() const {
        return _commonLineHeight;
    }

    void  FontAtlas::setCommonLineHeight(float newHeight) {
        _commonLineHeight = newHeight;
    }

    const Font * FontAtlas::getFont() const {
        return _font;
    }

    void FontAtlas::setAliasTexParameters() {
        if (_antialiasEnabled) {
            _antialiasEnabled = false;
            for (const auto & tex : _atlasTextures) {
                tex.second->setAliasTexParameters();
            }
        }
    }

    void FontAtlas::setAntiAliasTexParameters() {
        if (! _antialiasEnabled) {
            _antialiasEnabled = true;
            for (const auto & tex : _atlasTextures) {
                tex.second->setAntiAliasTexParameters();
            }
        }
    }

    std::unordered_map<std::string, FontAtlas *> FontAtlasCache::_atlasMap;

    void FontAtlasCache::purgeCachedData() {
        for (auto & atlas:_atlasMap) {
            atlas.second->purgeTexturesAtlas();
        }
    }

    std::string FontAtlasCache::generateFontName(const std::string& fontFileName, int size, GlyphCollection theGlyphs, bool useDistanceField)
    {
        std::string tempName(fontFileName);

        switch (theGlyphs)
        {
            case GlyphCollection::DYNAMIC:
                tempName.append("_DYNAMIC_");
            break;
            case GlyphCollection::CUSTOM:
                tempName.append("_CUSTOM_");
                break;

            default:
                break;
        }
        if(useDistanceField)
            tempName.append("df");
        return  tempName.append(UTILS::STRING::StringFromInt(size));
    }

    bool FontAtlasCache::releaseFontAtlas(FontAtlas *atlas)
    {
        if (nullptr != atlas)
        {
            for( auto &item: _atlasMap )
            {
                if ( item.second == atlas )
                {
                    if (atlas->getReferenceCount() == 1)
                    {
                      _atlasMap.erase(item.first);
                    }

                    atlas->release();

                    return true;
                }
            }
        }

        return false;
    }
}
