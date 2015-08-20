#include <string.h>
#include "GRAPH/BASE/Fonts.h"
#include "GRAPH/RENDERER/Texture2D.h"
#include "IMAGE/SmartImage.h"
#include "GRAPH/BASE/Director.h"
#include "GRAPH/BASE/EventDispatcher.h"
#include "GRAPH/BASE/Macros.h"
#include "GRAPH/BASE/Label.h"
#include "UTILS/STRING/StringUtils.h"
#include "UTILS/HASH/uthash.h"
#include "BASE/HData.h"
#include "IO/FileUtils.h"
#include "UTILS/STRING/UTFUtils.h"
#include "EXTERNALS/edtaa3func/edtaa3func.h"
#include "GRAPH/RENDERER/TextureCache.h"

namespace GRAPH
{
    const char * Font::_glyphASCII = "\"!#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~¡¢£¤¥¦§¨©ª«¬­®¯°±²³´µ¶·¸¹º»¼½¾¿ÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏÐÑÒÓÔÕÖ×ØÙÚÛÜÝÞßàáâãäåæçèéêëìíîïðñòóôõö÷øùúûüýþ ";

    const char * Font::_glyphNEHE =  "!\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~ ";

    Font::Font() :
    _usedGlyphs(GlyphCollection::ASCII)
    , _customGlyphs(nullptr)
    {
    }

    const char * Font::getGlyphCollection(GlyphCollection glyphs) const
    {
        switch (glyphs)
        {
            case GlyphCollection::NEHE:
                return _glyphNEHE;
                break;

            case GlyphCollection::ASCII:
                return _glyphASCII;
                break;

            default:
                return 0;
                break;
        }
    }

    void Font::setCurrentGlyphCollection(GlyphCollection glyphs, const char *customGlyphs)
    {
        if (_customGlyphs)
            delete [] _customGlyphs;

        switch (glyphs)
        {
            case GlyphCollection::NEHE:
                _customGlyphs = 0;
                break;

            case GlyphCollection::ASCII:
                _customGlyphs = 0;
                break;

            default:
                if (customGlyphs)
                {
                    size_t length = strlen(customGlyphs);
                    _customGlyphs = new char [length + 2];
                    memcpy(_customGlyphs, customGlyphs, length);

                    _customGlyphs[length]   = 0;
                    _customGlyphs[length+1] = 0;
                }

                break;
        }
        _usedGlyphs = glyphs;
    }

    const char * Font::getCurrentGlyphCollection() const
    {
        if (_customGlyphs)
        {
            return _customGlyphs;
        }
        else
        {
            return getGlyphCollection(_usedGlyphs);
        }
    }

    const int FontAtlas::CacheTextureWidth = 512;
    const int FontAtlas::CacheTextureHeight = 512;
    const char* FontAtlas::CMD_PURGE_FONTATLAS = "__cc_PURGE_FONTATLAS";
    const char* FontAtlas::CMD_RESET_FONTATLAS = "__cc_RESET_FONTATLAS";

    FontAtlas::FontAtlas(Font &theFont)
    : _font(&theFont)
    , _currentPageData(nullptr)
    , _fontAscender(0)
    , _rendererRecreatedListener(nullptr)
    , _antialiasEnabled(true)
    , _currLineHeight(0)
    {
        _font->retain();

        FontFreeType* fontTTf = dynamic_cast<FontFreeType*>(_font);
        if (fontTTf)
        {
            _commonLineHeight = _font->getFontMaxHeight();
            _fontAscender = fontTTf->getFontAscender();
            auto texture = new (std::nothrow) Texture2D;
            _currentPage = 0;
            _currentPageOrigX = 0;
            _currentPageOrigY = 0;
            _letterPadding = 0;

            if(fontTTf->isDistanceFieldEnabled())
            {
                _letterPadding += 2 * FontFreeType::DistanceMapSpread;
            }
            _currentPageDataSize = CacheTextureWidth * CacheTextureHeight;
            auto outlineSize = fontTTf->getOutlineSize();
            if(outlineSize > 0)
            {
                _commonLineHeight += 2 * outlineSize;
                _currentPageDataSize *= 2;
            }

            _currentPageData = new unsigned char[_currentPageDataSize];
            memset(_currentPageData, 0, _currentPageDataSize);

            auto  pixelFormat = outlineSize > 0 ? IMAGE::PixelFormat::AI88 : IMAGE::PixelFormat::A8;
            texture->initWithData(_currentPageData, _currentPageDataSize,
                pixelFormat, CacheTextureWidth, CacheTextureHeight, MATH::Sizef(CacheTextureWidth,CacheTextureHeight) );

            addTexture(texture,0);
            texture->release();
        }
    }

    FontAtlas::~FontAtlas()
    {
        _font->release();
        relaseTextures();

        delete []_currentPageData;
    }

    void FontAtlas::relaseTextures()
    {
        for( auto &item: _atlasTextures)
        {
            item.second->release();
        }
        _atlasTextures.clear();
    }

    void FontAtlas::purgeTexturesAtlas()
    {
        FontFreeType* fontTTf = dynamic_cast<FontFreeType*>(_font);
        if (fontTTf && _atlasTextures.size() > 1)
        {
            auto eventDispatcher = Director::getInstance()->getEventDispatcher();
            eventDispatcher->dispatchCustomEvent(CMD_PURGE_FONTATLAS,this);
            eventDispatcher->dispatchCustomEvent(CMD_RESET_FONTATLAS,this);
        }
    }

    void FontAtlas::listenRendererRecreated(EventCustom *event)
    {
        FontFreeType* fontTTf = dynamic_cast<FontFreeType*>(_font);
        if (fontTTf)
        {
            auto eventDispatcher = Director::getInstance()->getEventDispatcher();
            eventDispatcher->dispatchCustomEvent(CMD_PURGE_FONTATLAS,this);
            eventDispatcher->dispatchCustomEvent(CMD_RESET_FONTATLAS,this);
        }
    }

    void FontAtlas::addLetterDefinition(const FontLetterDefinition &letterDefinition)
    {
        _fontLetterDefinitions[letterDefinition.letteCharUTF16] = letterDefinition;
    }

    bool FontAtlas::getLetterDefinitionForChar(char16_t letteCharUTF16, FontLetterDefinition &outDefinition)
    {
        auto outIterator = _fontLetterDefinitions.find(letteCharUTF16);

        if (outIterator != _fontLetterDefinitions.end())
        {
            outDefinition = (*outIterator).second;
            return true;
        }
        else
        {
            outDefinition.validDefinition = false;
            return false;
        }
    }

    bool FontAtlas::prepareLetterDefinitions(const std::u16string& utf16String)
    {
        FontFreeType* fontTTf = dynamic_cast<FontFreeType*>(_font);
        if(fontTTf == nullptr)
            return false;

        size_t length = utf16String.length();

        float offsetAdjust = _letterPadding / 2;
        long bitmapWidth;
        long bitmapHeight;
        MATH::Rectf tempRect;
        FontLetterDefinition tempDef;

        auto scaleFactor = CC_CONTENT_SCALE_FACTOR();
        auto  pixelFormat = fontTTf->getOutlineSize() > 0 ? IMAGE::PixelFormat::AI88 : IMAGE::PixelFormat::A8;

        bool existNewLetter = false;
        int bottomHeight = _commonLineHeight - _fontAscender;

        float startY = _currentPageOrigY;

        for (size_t i = 0; i < length; ++i)
        {
            auto outIterator = _fontLetterDefinitions.find(utf16String[i]);

            if (outIterator == _fontLetterDefinitions.end())
            {
                existNewLetter = true;

                auto bitmap = fontTTf->getGlyphBitmap(utf16String[i],bitmapWidth,bitmapHeight,tempRect,tempDef.xAdvance);
                if (bitmap)
                {
                    tempDef.validDefinition = true;
                    tempDef.letteCharUTF16   = utf16String[i];
                    tempDef.width            = tempRect.size.width + _letterPadding;
                    tempDef.height           = tempRect.size.height + _letterPadding;
                    tempDef.offsetX          = tempRect.origin.x + offsetAdjust;
                    tempDef.offsetY          = _fontAscender + tempRect.origin.y - offsetAdjust;
                    tempDef.clipBottom     = bottomHeight - (tempDef.height + tempRect.origin.y + offsetAdjust);

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
                    fontTTf->renderCharAt(_currentPageData,_currentPageOrigX,_currentPageOrigY,bitmap,bitmapWidth,bitmapHeight);

                    tempDef.U                = _currentPageOrigX;
                    tempDef.V                = _currentPageOrigY;
                    tempDef.textureID        = _currentPage;
                    _currentPageOrigX        += tempDef.width + 1;
                    // take from pixels to points
                    tempDef.width  =    tempDef.width  / scaleFactor;
                    tempDef.height =    tempDef.height / scaleFactor;
                    tempDef.U      =    tempDef.U      / scaleFactor;
                    tempDef.V      =    tempDef.V      / scaleFactor;
                }
                else{
                    if(tempDef.xAdvance)
                        tempDef.validDefinition = true;
                    else
                        tempDef.validDefinition = false;

                    tempDef.letteCharUTF16   = utf16String[i];
                    tempDef.width            = 0;
                    tempDef.height           = 0;
                    tempDef.U                = 0;
                    tempDef.V                = 0;
                    tempDef.offsetX          = 0;
                    tempDef.offsetY          = 0;
                    tempDef.textureID        = 0;
                    tempDef.clipBottom = 0;
                    _currentPageOrigX += 1;
                }

                _fontLetterDefinitions[tempDef.letteCharUTF16] = tempDef;
            }
        }

        if(existNewLetter)
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
                                                         CacheTextureWidth, _currentPageOrigY - startY + _commonLineHeight);
        }
        return true;
    }

    void FontAtlas::addTexture(Texture2D *texture, int slot)
    {
        texture->retain();
        _atlasTextures[slot] = texture;
    }

    Texture2D* FontAtlas::getTexture(int slot)
    {
        return _atlasTextures[slot];
    }

    float FontAtlas::getCommonLineHeight() const
    {
        return _commonLineHeight;
    }

    void  FontAtlas::setCommonLineHeight(float newHeight)
    {
        _commonLineHeight = newHeight;
    }

    const Font * FontAtlas::getFont() const
    {
        return _font;
    }

    void FontAtlas::setAliasTexParameters()
    {
        if (_antialiasEnabled)
        {
            _antialiasEnabled = false;
            for (const auto & tex : _atlasTextures)
            {
                tex.second->setAliasTexParameters();
            }
        }
    }

    void FontAtlas::setAntiAliasTexParameters()
    {
        if (! _antialiasEnabled)
        {
            _antialiasEnabled = true;
            for (const auto & tex : _atlasTextures)
            {
                tex.second->setAntiAliasTexParameters();
            }
        }
    }

    std::unordered_map<std::string, FontAtlas *> FontAtlasCache::_atlasMap;

    void FontAtlasCache::purgeCachedData()
    {
        for (auto & atlas:_atlasMap)
        {
            atlas.second->purgeTexturesAtlas();
        }
    }

    FontAtlas * FontAtlasCache::getFontAtlasTTF(const TTFConfig & config)
    {
        bool useDistanceField = config.distanceFieldEnabled;
        if(config.outlineSize > 0)
        {
            useDistanceField = false;
        }
        int fontSize = config.fontSize;
        auto contentScaleFactor = CC_CONTENT_SCALE_FACTOR();

        if (useDistanceField)
        {
            fontSize = Label::DistanceFieldFontSize / contentScaleFactor;
        }

        auto atlasName = generateFontName(config.fontFilePath, fontSize, GlyphCollection::DYNAMIC, useDistanceField);
        atlasName.append("_outline_");
        atlasName.append(UTILS::STRING::StringFromInt(config.outlineSize));

        auto it = _atlasMap.find(atlasName);

        if ( it == _atlasMap.end() )
        {
            auto font = FontFreeType::create(config.fontFilePath, fontSize, config.glyphs,
                config.customGlyphs, useDistanceField, config.outlineSize);
            if (font)
            {
                auto tempAtlas = font->createFontAtlas();
                if (tempAtlas)
                {
                    _atlasMap[atlasName] = tempAtlas;
                    return _atlasMap[atlasName];
                }
            }
        }
        else
        {
            _atlasMap[atlasName]->retain();
            return _atlasMap[atlasName];
        }

        return nullptr;
    }

    FontAtlas * FontAtlasCache::getFontAtlasFNT(const std::string& fontFileName, const MATH::Vector2f& imageOffset /* = Vec2::ZERO */)
    {
        std::string atlasName = generateFontName(fontFileName, 0, GlyphCollection::CUSTOM,false);
        auto it = _atlasMap.find(atlasName);

        if ( it == _atlasMap.end() )
        {
            auto font = FontFNT::create(fontFileName,imageOffset);

            if(font)
            {
                auto tempAtlas = font->createFontAtlas();
                if (tempAtlas)
                {
                    _atlasMap[atlasName] = tempAtlas;
                    return _atlasMap[atlasName];
                }
            }
        }
        else
        {
            _atlasMap[atlasName]->retain();
            return _atlasMap[atlasName];
        }

        return nullptr;
    }

    FontAtlas * FontAtlasCache::getFontAtlasCharMap(const std::string& plistFile)
    {
        std::string atlasName = generateFontName(plistFile, 0, GlyphCollection::CUSTOM,false);
        auto it = _atlasMap.find(atlasName);

        if ( it == _atlasMap.end() )
        {
            auto font = FontCharMap::create(plistFile);

            if(font)
            {
                auto tempAtlas = font->createFontAtlas();
                if (tempAtlas)
                {
                    _atlasMap[atlasName] = tempAtlas;
                    return _atlasMap[atlasName];
                }
            }
        }
        else
        {
            _atlasMap[atlasName]->retain();
            return _atlasMap[atlasName];
        }

        return nullptr;
    }

    FontAtlas * FontAtlasCache::getFontAtlasCharMap(Texture2D* texture, int itemWidth, int itemHeight, int startCharMap)
    {
        char tmp[30];
        sprintf(tmp,"name:%u_%d_%d_%d",texture->getName(),itemWidth,itemHeight,startCharMap);
        std::string atlasName = generateFontName(tmp, 0, GlyphCollection::CUSTOM,false);

        auto it = _atlasMap.find(atlasName);
        if ( it == _atlasMap.end() )
        {
            auto font = FontCharMap::create(texture,itemWidth,itemHeight,startCharMap);

            if(font)
            {
                auto tempAtlas = font->createFontAtlas();
                if (tempAtlas)
                {
                    _atlasMap[atlasName] = tempAtlas;
                    return _atlasMap[atlasName];
                }
            }
        }
        else
        {
            _atlasMap[atlasName]->retain();
            return _atlasMap[atlasName];
        }

        return nullptr;
    }

    FontAtlas * FontAtlasCache::getFontAtlasCharMap(const std::string& charMapFile, int itemWidth, int itemHeight, int startCharMap)
    {
        char tmp[255];
        snprintf(tmp,250,"name:%s_%d_%d_%d",charMapFile.c_str(),itemWidth,itemHeight,startCharMap);

        std::string atlasName = generateFontName(tmp, 0, GlyphCollection::CUSTOM,false);

        auto it = _atlasMap.find(atlasName);
        if ( it == _atlasMap.end() )
        {
            auto font = FontCharMap::create(charMapFile,itemWidth,itemHeight,startCharMap);

            if(font)
            {
                auto tempAtlas = font->createFontAtlas();
                if (tempAtlas)
                {
                    _atlasMap[atlasName] = tempAtlas;
                    return _atlasMap[atlasName];
                }
            }
        }
        else
        {
            _atlasMap[atlasName]->retain();
            return _atlasMap[atlasName];
        }

        return nullptr;
    }

    std::string FontAtlasCache::generateFontName(const std::string& fontFileName, int size, GlyphCollection theGlyphs, bool useDistanceField)
    {
        std::string tempName(fontFileName);

        switch (theGlyphs)
        {
            case GlyphCollection::DYNAMIC:
                tempName.append("_DYNAMIC_");
            break;

            case GlyphCollection::NEHE:
                tempName.append("_NEHE_");
                break;

            case GlyphCollection::ASCII:
                tempName.append("_ASCII_");
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

    FT_Library FontFreeType::_FTlibrary;
    bool       FontFreeType::_FTInitialized = false;
    const int  FontFreeType::DistanceMapSpread = 3;

    typedef struct _DataRef
    {
        HData data;
        unsigned int referenceCount;
    }DataRef;

    static std::unordered_map<std::string, DataRef> s_cacheFontData;

    FontFreeType * FontFreeType::create(const std::string &fontName, int fontSize, GlyphCollection glyphs, const char *customGlyphs,bool distanceFieldEnabled /* = false */,int outline /* = 0 */)
    {
        FontFreeType *tempFont =  new FontFreeType(distanceFieldEnabled,outline);

        if (!tempFont)
            return nullptr;

        tempFont->setCurrentGlyphCollection(glyphs, customGlyphs);

        if (!tempFont->createFontObject(fontName, fontSize))
        {
            delete tempFont;
            return nullptr;
        }
        return tempFont;
    }

    bool FontFreeType::initFreeType()
    {
        if (_FTInitialized == false)
        {
            // begin freetype
            if (FT_Init_FreeType( &_FTlibrary ))
                return false;

            _FTInitialized = true;
        }

        return  _FTInitialized;
    }

    void FontFreeType::shutdownFreeType()
    {
        if (_FTInitialized == true)
        {
            FT_Done_FreeType(_FTlibrary);
            _FTInitialized = false;
        }
    }

    FT_Library FontFreeType::getFTLibrary()
    {
        initFreeType();
        return _FTlibrary;
    }

    FontFreeType::FontFreeType(bool distanceFieldEnabled /* = false */,int outline /* = 0 */)
    : _fontRef(nullptr)
    , _stroker(nullptr)
    , _distanceFieldEnabled(distanceFieldEnabled)
    , _outlineSize(0.0f)
    , _lineHeight(0)
    , _fontAtlas(nullptr)
    {
        if (outline > 0)
        {
            _outlineSize = outline * CC_CONTENT_SCALE_FACTOR();
            FT_Stroker_New(FontFreeType::getFTLibrary(), &_stroker);
            FT_Stroker_Set(_stroker,
                (int)(_outlineSize * 64),
                FT_STROKER_LINECAP_ROUND,
                FT_STROKER_LINEJOIN_ROUND,
                0);
        }
    }

    bool FontFreeType::createFontObject(const std::string &fontName, int fontSize)
    {
        FT_Face face;
        // save font name locally
        _fontName = fontName;

        auto it = s_cacheFontData.find(fontName);
        if (it != s_cacheFontData.end())
        {
            (*it).second.referenceCount += 1;
        }
        else
        {
            s_cacheFontData[fontName].referenceCount = 1;
            s_cacheFontData[fontName].data = IO::FileUtils::getInstance().getDataFromFile(fontName);

            if (s_cacheFontData[fontName].data.isNull())
            {
                return false;
            }
        }

        if (FT_New_Memory_Face(getFTLibrary(), s_cacheFontData[fontName].data.getBytes(), s_cacheFontData[fontName].data.getSize(), 0, &face ))
            return false;

        if (FT_Select_Charmap(face, FT_ENCODING_UNICODE))
        {
            int foundIndex = -1;
            for (int charmapIndex = 0; charmapIndex < face->num_charmaps; charmapIndex++)
            {
                if (face->charmaps[charmapIndex]->encoding != FT_ENCODING_NONE)
                {
                    foundIndex = charmapIndex;
                    break;
                }
            }

            if (foundIndex == -1)
            {
                return false;
            }

            if (FT_Select_Charmap(face, face->charmaps[foundIndex]->encoding))
            {
                return false;
            }
        }

        // set the requested font size
        int dpi = 72;
        int fontSizePoints = (int)(64.f * fontSize * CC_CONTENT_SCALE_FACTOR());
        if (FT_Set_Char_Size(face, fontSizePoints, fontSizePoints, dpi, dpi))
            return false;

        // store the face globally
        _fontRef = face;
        _lineHeight = static_cast<int>(_fontRef->size->metrics.height >> 6);

        // done and good
        return true;
    }

    FontFreeType::~FontFreeType()
    {
        if (_stroker)
        {
            FT_Stroker_Done(_stroker);
        }
        if (_fontRef)
        {
            FT_Done_Face(_fontRef);
        }

        s_cacheFontData[_fontName].referenceCount -= 1;
        if (s_cacheFontData[_fontName].referenceCount == 0)
        {
            s_cacheFontData.erase(_fontName);
        }
    }

    FontAtlas * FontFreeType::createFontAtlas()
    {
        if (_fontAtlas == nullptr)
        {
            _fontAtlas = new (std::nothrow) FontAtlas(*this);
            if (_fontAtlas && _usedGlyphs != GlyphCollection::DYNAMIC)
            {
                std::u16string utf16;
                UTILS::STRING::UTF8ToUTF16(getCurrentGlyphCollection(), utf16);
                _fontAtlas->prepareLetterDefinitions(utf16);
            }
            this->release();
        }

        return _fontAtlas;
    }

    int * FontFreeType::getHorizontalKerningForTextUTF16(const std::u16string& text, int &outNumLetters) const
    {
        if (!_fontRef)
            return nullptr;

        outNumLetters = static_cast<int>(text.length());

        if (!outNumLetters)
            return nullptr;

        int *sizes = new int[outNumLetters];
        if (!sizes)
            return nullptr;
        memset(sizes,0,outNumLetters * sizeof(int));

        bool hasKerning = FT_HAS_KERNING( _fontRef ) != 0;
        if (hasKerning)
        {
            for (int c = 1; c < outNumLetters; ++c)
            {
                sizes[c] = getHorizontalKerningForChars(text[c-1], text[c]);
            }
        }

        return sizes;
    }

    int  FontFreeType::getHorizontalKerningForChars(unsigned short firstChar, unsigned short secondChar) const
    {
        // get the ID to the char we need
        int glyphIndex1 = FT_Get_Char_Index(_fontRef, firstChar);

        if (!glyphIndex1)
            return 0;

        // get the ID to the char we need
        int glyphIndex2 = FT_Get_Char_Index(_fontRef, secondChar);

        if (!glyphIndex2)
            return 0;

        FT_Vector kerning;

        if (FT_Get_Kerning( _fontRef, glyphIndex1, glyphIndex2,  FT_KERNING_DEFAULT,  &kerning))
            return 0;

        return (static_cast<int>(kerning.x >> 6));
    }

    int FontFreeType::getFontAscender() const
    {
        return (static_cast<int>(_fontRef->size->metrics.ascender >> 6));
    }

    unsigned char* FontFreeType::getGlyphBitmap(unsigned short theChar, long &outWidth, long &outHeight, MATH::Rectf &outRect,int &xAdvance)
    {
        bool invalidChar = true;
        unsigned char * ret = nullptr;

        do
        {
            if (!_fontRef)
                break;

            auto glyphIndex = FT_Get_Char_Index(_fontRef, theChar);
            if(!glyphIndex)
                break;

            if (_distanceFieldEnabled)
            {
                if (FT_Load_Glyph(_fontRef,glyphIndex,FT_LOAD_RENDER | FT_LOAD_NO_HINTING | FT_LOAD_NO_AUTOHINT))
                    break;
            }
            else
            {
                if (FT_Load_Glyph(_fontRef,glyphIndex,FT_LOAD_RENDER | FT_LOAD_NO_AUTOHINT))
                    break;
            }

            auto& metrics = _fontRef->glyph->metrics;
            outRect.origin.x = metrics.horiBearingX >> 6;
            outRect.origin.y = -(metrics.horiBearingY >> 6);
            outRect.size.width = (metrics.width >> 6);
            outRect.size.height = (metrics.height >> 6);

            xAdvance = (static_cast<int>(_fontRef->glyph->metrics.horiAdvance >> 6));

            outWidth  = _fontRef->glyph->bitmap.width;
            outHeight = _fontRef->glyph->bitmap.rows;
            ret = _fontRef->glyph->bitmap.buffer;

            if (_outlineSize > 0)
            {
                auto copyBitmap = new unsigned char[outWidth * outHeight];
                memcpy(copyBitmap,ret,outWidth * outHeight * sizeof(unsigned char));

                FT_BBox bbox;
                auto outlineBitmap = getGlyphBitmapWithOutline(theChar,bbox);
                if(outlineBitmap == nullptr)
                {
                    ret = nullptr;
                    delete [] copyBitmap;
                    break;
                }

                long glyphMinX = outRect.origin.x;
                long glyphMaxX = outRect.origin.x + outWidth;
                long glyphMinY = -outHeight - outRect.origin.y;
                long glyphMaxY = -outRect.origin.y;

                auto outlineMinX = bbox.xMin >> 6;
                auto outlineMaxX = bbox.xMax >> 6;
                auto outlineMinY = bbox.yMin >> 6;
                auto outlineMaxY = bbox.yMax >> 6;
                auto outlineWidth = outlineMaxX - outlineMinX;
                auto outlineHeight = outlineMaxY - outlineMinY;

                auto blendImageMinX = MATH::MATH_MIN(outlineMinX, glyphMinX);
                auto blendImageMaxY = MATH::MATH_MAX(outlineMaxY, glyphMaxY);
                auto blendWidth = MATH::MATH_MAX(outlineMaxX, glyphMaxX) - blendImageMinX;
                auto blendHeight = blendImageMaxY - MATH::MATH_MIN(outlineMinY, glyphMinY);

                outRect.origin.x = blendImageMinX;
                outRect.origin.y = -blendImageMaxY + _outlineSize;

                long index, index2;
                auto blendImage = new unsigned char[blendWidth * blendHeight * 2];
                memset(blendImage, 0, blendWidth * blendHeight * 2);

                auto px = outlineMinX - blendImageMinX;
                auto py = blendImageMaxY - outlineMaxY;
                for (int x = 0; x < outlineWidth; ++x)
                {
                    for (int y = 0; y < outlineHeight; ++y)
                    {
                        index = px + x + ((py + y) * blendWidth);
                        index2 = x + (y * outlineWidth);
                        blendImage[2 * index] = outlineBitmap[index2];
                    }
                }

                px = glyphMinX - blendImageMinX;
                py = blendImageMaxY - glyphMaxY;
                for (int x = 0; x < outWidth; ++x)
                {
                    for (int y = 0; y < outHeight; ++y)
                    {
                        index = px + x + ((y + py) * blendWidth);
                        index2 = x + (y * outWidth);
                        blendImage[2 * index + 1] = copyBitmap[index2];
                    }
                }

                outRect.size.width  =  blendWidth;
                outRect.size.height =  blendHeight;
                outWidth  = blendWidth;
                outHeight = blendHeight;

                delete [] outlineBitmap;
                delete [] copyBitmap;
                ret = blendImage;
            }

            invalidChar = false;
        } while (0);

        if (invalidChar)
        {
            outRect.size.width  = 0;
            outRect.size.height = 0;
            xAdvance = 0;

            return nullptr;
        }
        else
        {
           return ret;
        }
    }

    unsigned char * FontFreeType::getGlyphBitmapWithOutline(unsigned short theChar, FT_BBox &bbox)
    {
        unsigned char* ret = nullptr;

        FT_UInt gindex = FT_Get_Char_Index(_fontRef, theChar);
        if (FT_Load_Glyph(_fontRef, gindex, FT_LOAD_NO_BITMAP) == 0)
        {
            if (_fontRef->glyph->format == FT_GLYPH_FORMAT_OUTLINE)
            {
                FT_Glyph glyph;
                if (FT_Get_Glyph(_fontRef->glyph, &glyph) == 0)
                {
                    FT_Glyph_StrokeBorder(&glyph, _stroker, 0, 1);
                    if (glyph->format == FT_GLYPH_FORMAT_OUTLINE)
                    {
                        FT_Outline *outline = &reinterpret_cast<FT_OutlineGlyph>(glyph)->outline;
                        FT_Glyph_Get_CBox(glyph,FT_GLYPH_BBOX_GRIDFIT,&bbox);
                        long width = (bbox.xMax - bbox.xMin)>>6;
                        long rows = (bbox.yMax - bbox.yMin)>>6;

                        FT_Bitmap bmp;
                        bmp.buffer = new unsigned char[width * rows];
                        memset(bmp.buffer, 0, width * rows);
                        bmp.width = (int)width;
                        bmp.rows = (int)rows;
                        bmp.pitch = (int)width;
                        bmp.pixel_mode = FT_PIXEL_MODE_GRAY;
                        bmp.num_grays = 256;

                        FT_Raster_Params params;
                        memset(&params, 0, sizeof (params));
                        params.source = outline;
                        params.target = &bmp;
                        params.flags = FT_RASTER_FLAG_AA;
                        FT_Outline_Translate(outline,-bbox.xMin,-bbox.yMin);
                        FT_Outline_Render(_FTlibrary, outline, &params);

                        ret = bmp.buffer;
                    }
                    FT_Done_Glyph(glyph);
                }
            }
        }

        return ret;
    }

    unsigned char * makeDistanceMap( unsigned char *img, long width, long height)
    {
        long pixelAmount = (width + 2 * FontFreeType::DistanceMapSpread) * (height + 2 * FontFreeType::DistanceMapSpread);

        short * xdist = (short *)  malloc( pixelAmount * sizeof(short) );
        short * ydist = (short *)  malloc( pixelAmount * sizeof(short) );
        double * gx   = (double *) calloc( pixelAmount, sizeof(double) );
        double * gy      = (double *) calloc( pixelAmount, sizeof(double) );
        double * data    = (double *) calloc( pixelAmount, sizeof(double) );
        double * outside = (double *) calloc( pixelAmount, sizeof(double) );
        double * inside  = (double *) calloc( pixelAmount, sizeof(double) );
        long i,j;

        // Convert img into double (data) rescale image levels between 0 and 1
        long outWidth = width + 2 * FontFreeType::DistanceMapSpread;
        for (i = 0; i < width; ++i)
        {
            for (j = 0; j < height; ++j)
            {
                data[j * outWidth + FontFreeType::DistanceMapSpread + i] = img[j * width + i] / 255.0;
            }
        }

        width += 2 * FontFreeType::DistanceMapSpread;
        height += 2 * FontFreeType::DistanceMapSpread;

        // Transform background (outside contour, in areas of 0's)
        computegradient( data, (int)width, (int)height, gx, gy);
        edtaa3(data, gx, gy, (int)width, (int)height, xdist, ydist, outside);
        for( i=0; i< pixelAmount; i++)
            if( outside[i] < 0.0 )
                outside[i] = 0.0;

        // Transform foreground (inside contour, in areas of 1's)
        for( i=0; i< pixelAmount; i++)
            data[i] = 1 - data[i];
        computegradient( data, (int)width, (int)height, gx, gy);
        edtaa3(data, gx, gy, (int)width, (int)height, xdist, ydist, inside);
        for( i=0; i< pixelAmount; i++)
            if( inside[i] < 0.0 )
                inside[i] = 0.0;

        // The bipolar distance field is now outside-inside
        double dist;
        /* Single channel 8-bit output (bad precision and range, but simple) */
        unsigned char *out = (unsigned char *) malloc( pixelAmount * sizeof(unsigned char) );
        for( i=0; i < pixelAmount; i++)
        {
            dist = outside[i] - inside[i];
            dist = 128.0 - dist*16;
            if( dist < 0 ) dist = 0;
            if( dist > 255 ) dist = 255;
            out[i] = (unsigned char) dist;
        }
        /* Dual channel 16-bit output (more complicated, but good precision and range) */
        /*unsigned char *out = (unsigned char *) malloc( pixelAmount * 3 * sizeof(unsigned char) );
        for( i=0; i< pixelAmount; i++)
        {
            dist = outside[i] - inside[i];
            dist = 128.0 - dist*16;
            if( dist < 0.0 ) dist = 0.0;
            if( dist >= 256.0 ) dist = 255.999;
            // R channel is a copy of the original grayscale image
            out[3*i] = img[i];
            // G channel is fraction
            out[3*i + 1] = (unsigned char) ( 256 - (dist - floor(dist)* 256.0 ));
            // B channel is truncated integer part
            out[3*i + 2] = (unsigned char)dist;
        }*/

        free( xdist );
        free( ydist );
        free( gx );
        free( gy );
        free( data );
        free( outside );
        free( inside );

        return out;
    }

    void FontFreeType::renderCharAt(unsigned char *dest,int posX, int posY, unsigned char* bitmap,long bitmapWidth,long bitmapHeight)
    {
        int iX = posX;
        int iY = posY;

        if (_distanceFieldEnabled)
        {
            auto distanceMap = makeDistanceMap(bitmap,bitmapWidth,bitmapHeight);

            bitmapWidth += 2 * DistanceMapSpread;
            bitmapHeight += 2 * DistanceMapSpread;

            for (long y = 0; y < bitmapHeight; ++y)
            {
                long bitmap_y = y * bitmapWidth;

                for (long x = 0; x < bitmapWidth; ++x)
                {
                    /* Dual channel 16-bit output (more complicated, but good precision and range) */
                    /*int index = (iX + ( iY * destSize )) * 3;
                    int index2 = (bitmap_y + x)*3;
                    dest[index] = out[index2];
                    dest[index + 1] = out[index2 + 1];
                    dest[index + 2] = out[index2 + 2];*/

                    //Single channel 8-bit output
                    dest[iX + ( iY * FontAtlas::CacheTextureWidth )] = distanceMap[bitmap_y + x];

                    iX += 1;
                }

                iX  = posX;
                iY += 1;
            }
            free(distanceMap);
        }
        else if(_outlineSize > 0)
        {
            unsigned char tempChar;
            for (long y = 0; y < bitmapHeight; ++y)
            {
                long bitmap_y = y * bitmapWidth;

                for (int x = 0; x < bitmapWidth; ++x)
                {
                    tempChar = bitmap[(bitmap_y + x) * 2];
                    dest[(iX + ( iY * FontAtlas::CacheTextureWidth ) ) * 2] = tempChar;
                    tempChar = bitmap[(bitmap_y + x) * 2 + 1];
                    dest[(iX + ( iY * FontAtlas::CacheTextureWidth ) ) * 2 + 1] = tempChar;

                    iX += 1;
                }

                iX  = posX;
                iY += 1;
            }
            delete [] bitmap;
        }
        else
        {
            for (long y = 0; y < bitmapHeight; ++y)
            {
                long bitmap_y = y * bitmapWidth;

                for (int x = 0; x < bitmapWidth; ++x)
                {
                    unsigned char cTemp = bitmap[bitmap_y + x];

                    // the final pixel
                    dest[(iX + ( iY * FontAtlas::CacheTextureWidth ) )] = cTemp;

                    iX += 1;
                }

                iX  = posX;
                iY += 1;
            }
        }
    }

    struct _FontDefHashElement;

    /**
    @struct BMFontDef
    BMFont definition
    */
    typedef struct _BMFontDef {
        //! ID of the character
        unsigned int charID;
        //! origin and size of the font
        MATH::Rectf rect;
        //! The X amount the image should be offset when drawing the image (in pixels)
        short xOffset;
        //! The Y amount the image should be offset when drawing the image (in pixels)
        short yOffset;
        //! The amount to move the current position after drawing the character (in pixels)
        short xAdvance;
    } BMFontDef;

    /** @struct BMFontPadding
    BMFont padding
    @since v0.8.2
    */
    typedef struct _BMFontPadding {
        /// padding left
        int    left;
        /// padding top
        int top;
        /// padding right
        int right;
        /// padding bottom
        int bottom;
    } BMFontPadding;

    typedef struct _FontDefHashElement
    {
        unsigned int    key;        // key. Font Unicode value
        BMFontDef       fontDef;    // font definition
        UT_hash_handle  hh;
    } tFontDefHashElement;

    // Equal function for targetSet.
    typedef struct _KerningHashElement
    {
        int				key;        // key for the hash. 16-bit for 1st element, 16-bit for 2nd element
        int				amount;
        UT_hash_handle	hh;
    } tKerningHashElement;

    class BMFontConfiguration : public HObject
    {
    public://@public
        tFontDefHashElement *_fontDefDictionary;
        int _commonHeight;
        BMFontPadding    _padding;
        std::string _atlasName;
        tKerningHashElement *_kerningDictionary;
        std::set<unsigned int> *_characterSet;

    public:
        BMFontConfiguration();
        virtual ~BMFontConfiguration();

        /** allocates a BMFontConfiguration with a FNT file */
        static BMFontConfiguration * create(const std::string& FNTfile);

        /** initializes a BitmapFontConfiguration with a FNT file */
        bool initWithFNTfile(const std::string& FNTfile);

        inline const std::string& getAtlasName(){ return _atlasName; }
        inline void setAtlasName(const std::string& atlasName) { _atlasName = atlasName; }

        std::set<unsigned int>* getCharacterSet() const;
    private:
        std::set<unsigned int>* parseConfigFile(const std::string& controlFile);
        std::set<unsigned int>* parseBinaryConfigFile(unsigned char* pData, unsigned long size, const std::string& controlFile);
        void parseCharacterDefinition(const char* line, BMFontDef *characterDefinition);
        void parseInfoArguments(const char* line);
        void parseCommonArguments(const char* line);
        void parseImageFileName(const char* line, const std::string& fntFile);
        void parseKerningEntry(const char* line);
        void purgeKerningDictionary();
        void purgeFontDefDictionary();
    };

    //
    //FNTConfig Cache - free functions
    //
    static HObjectMap<std::string, BMFontConfiguration*>* s_configurations = nullptr;

    BMFontConfiguration* FNTConfigLoadFile(const std::string& fntFile)
    {
        BMFontConfiguration* ret = nullptr;

        if( s_configurations == nullptr )
        {
            s_configurations = new (std::nothrow) HObjectMap<std::string, BMFontConfiguration*>();
        }

        ret = s_configurations->at(fntFile);
        if( ret == nullptr )
        {
            ret = BMFontConfiguration::create(fntFile.c_str());
            if (ret)
            {
                s_configurations->insert(fntFile, ret);
            }
        }

        return ret;
    }

    //
    //BitmapFontConfiguration
    //

    BMFontConfiguration * BMFontConfiguration::create(const std::string& FNTfile)
    {
        BMFontConfiguration * ret = new (std::nothrow) BMFontConfiguration();
        if (ret->initWithFNTfile(FNTfile))
        {
            ret->autorelease();
            return ret;
        }
        SAFE_DELETE(ret);
        return nullptr;
    }

    bool BMFontConfiguration::initWithFNTfile(const std::string& FNTfile)
    {
        _kerningDictionary = nullptr;
        _fontDefDictionary = nullptr;

        _characterSet = this->parseConfigFile(FNTfile);

        if (! _characterSet)
        {
            return false;
        }

        return true;
    }

    std::set<unsigned int>* BMFontConfiguration::getCharacterSet() const
    {
        return _characterSet;
    }

    BMFontConfiguration::BMFontConfiguration()
    : _fontDefDictionary(nullptr)
    , _commonHeight(0)
    , _kerningDictionary(nullptr)
    , _characterSet(nullptr)
    {

    }

    BMFontConfiguration::~BMFontConfiguration()
    {
        this->purgeFontDefDictionary();
        this->purgeKerningDictionary();
        _atlasName.clear();
        SAFE_DELETE(_characterSet);
    }

    void BMFontConfiguration::purgeKerningDictionary()
    {
        tKerningHashElement *current;
        while(_kerningDictionary)
        {
            current = _kerningDictionary;
            HASH_DEL(_kerningDictionary,current);
            free(current);
        }
    }

    void BMFontConfiguration::purgeFontDefDictionary()
    {
        tFontDefHashElement *current, *tmp;

        HASH_ITER(hh, _fontDefDictionary, current, tmp) {
            HASH_DEL(_fontDefDictionary, current);
            free(current);
        }
    }

    std::set<unsigned int>* BMFontConfiguration::parseConfigFile(const std::string& controlFile)
    {
        HData data = IO::FileUtils::getInstance().getDataFromFile(controlFile);
        if (memcmp("BMF", data.getBytes(), 3) == 0) {
            std::set<unsigned int>* ret = parseBinaryConfigFile(data.getBytes(), data.getSize(), controlFile);
            return ret;
        }

        auto contents = (const char*)data.getBytes();
        if (contents[0] == 0)
        {
            return nullptr;
        }

        std::set<unsigned int> *validCharsString = new std::set<unsigned int>();

        auto contentsLen = data.getSize();
        char line[512];

        auto next = strchr(contents, '\n');
        auto base = contents;
        int lineLength = 0;
        int parseCount = 0;
        while (next)
        {
            lineLength = ((int)(next - base));
            memcpy(line, contents + parseCount, lineLength);
            line[lineLength] = 0;

            parseCount += lineLength + 1;
            if (parseCount < contentsLen)
            {
                base = next + 1;
                next = strchr(base, '\n');
            }
            else
            {
                next = nullptr;
            }

            if (memcmp(line, "info face", 9) == 0)
            {
                // FIXME: info parsing is incomplete
                // Not needed for the Hiero editors, but needed for the AngelCode editor
                //            [self parseInfoArguments:line];
                this->parseInfoArguments(line);
            }
            // Check to see if the start of the line is something we are interested in
            else if (memcmp(line, "common lineHeight", 17) == 0)
            {
                this->parseCommonArguments(line);
            }
            else if (memcmp(line, "page id", 7) == 0)
            {
                this->parseImageFileName(line, controlFile);
            }
            else if (memcmp(line, "chars c", 7) == 0)
            {
                // Ignore this line
            }
            else if (memcmp(line, "char", 4) == 0)
            {
                // Parse the current line and create a new CharDef
                tFontDefHashElement* element = (tFontDefHashElement*)malloc( sizeof(*element) );
                this->parseCharacterDefinition(line, &element->fontDef);

                element->key = element->fontDef.charID;
                HASH_ADD_INT(_fontDefDictionary, key, element);

                validCharsString->insert(element->fontDef.charID);
            }
    //        else if(line.substr(0,strlen("kernings count")) == "kernings count")
    //        {
    //            this->parseKerningCapacity(line);
    //        }
            else if (memcmp(line, "kerning first", 13) == 0)
            {
                this->parseKerningEntry(line);
            }
        }

        return validCharsString;
    }

    std::set<unsigned int>* BMFontConfiguration::parseBinaryConfigFile(unsigned char* pData, unsigned long size, const std::string& controlFile)
    {
        /* based on http://www.angelcode.com/products/bmfont/doc/file_format.html file format */

        std::set<unsigned int> *validCharsString = new std::set<unsigned int>();

        unsigned long remains = size;

        pData += 4; remains -= 4;

        while (remains > 0)
        {
            unsigned char blockId = pData[0]; pData += 1; remains -= 1;
            uint32_t blockSize = 0; memcpy(&blockSize, pData, 4);

            pData += 4; remains -= 4;

            if (blockId == 1)
            {
                /*
                 fontSize       2   int      0
                 bitField       1   bits     2  bit 0: smooth, bit 1: unicode, bit 2: italic, bit 3: bold, bit 4: fixedHeigth, bits 5-7: reserved
                 charSet        1   uint     3
                 stretchH       2   uint     4
                 aa             1   uint     6
                 paddingUp      1   uint     7
                 paddingRight   1   uint     8
                 paddingDown    1   uint     9
                 paddingLeft    1   uint     10
                 spacingHoriz   1   uint     11
                 spacingVert    1   uint     12
                 outline        1   uint     13 added with version 2
                 fontName       n+1 string   14 null terminated string with length n
                 */

                _padding.top = (unsigned char)pData[7];
                _padding.right = (unsigned char)pData[8];
                _padding.bottom = (unsigned char)pData[9];
                _padding.left = (unsigned char)pData[10];
            }
            else if (blockId == 2)
            {
                /*
                 lineHeight 2   uint    0
                 base       2   uint    2
                 scaleW     2   uint    4
                 scaleH     2   uint    6
                 pages      2   uint    8
                 bitField   1   bits    10  bits 0-6: reserved, bit 7: packed
                 alphaChnl  1   uint    11
                 redChnl    1   uint    12
                 greenChnl  1   uint    13
                 blueChnl   1   uint    14
                 */

                uint16_t lineHeight = 0; memcpy(&lineHeight, pData, 2);
                _commonHeight = lineHeight;

                uint16_t scaleW = 0; memcpy(&scaleW, pData + 4, 2);
                uint16_t scaleH = 0; memcpy(&scaleH, pData + 6, 2);

                uint16_t pages = 0; memcpy(&pages, pData + 8, 2);
            }
            else if (blockId == 3)
            {
                /*
                 pageNames 	p*(n+1) 	strings 	0 	p null terminated strings, each with length n
                 */

                const char *value = (const char *)pData;
                _atlasName = IO::FileUtils::getInstance().fullPathFromRelativeFile(value, controlFile);
            }
            else if (blockId == 4)
            {
                /*
                 id         4   uint    0+c*20  These fields are repeated until all characters have been described
                 x          2   uint    4+c*20
                 y          2   uint    6+c*20
                 width      2   uint    8+c*20
                 height     2   uint    10+c*20
                 xoffset    2   int     12+c*20
                 yoffset    2   int     14+c*20
                 xadvance   2   int     16+c*20
                 page       1   uint    18+c*20
                 chnl       1   uint    19+c*20
                 */

                unsigned long count = blockSize / 20;

                for (unsigned long i = 0; i < count; i++)
                {
                    tFontDefHashElement* element = (tFontDefHashElement*)malloc( sizeof(*element) );

                    uint32_t charId = 0; memcpy(&charId, pData + (i * 20), 4);
                    element->fontDef.charID = charId;

                    uint16_t charX = 0; memcpy(&charX, pData + (i * 20) + 4, 2);
                    element->fontDef.rect.origin.x = charX;

                    uint16_t charY = 0; memcpy(&charY, pData + (i * 20) + 6, 2);
                    element->fontDef.rect.origin.y = charY;

                    uint16_t charWidth = 0; memcpy(&charWidth, pData + (i * 20) + 8, 2);
                    element->fontDef.rect.size.width = charWidth;

                    uint16_t charHeight = 0; memcpy(&charHeight, pData + (i * 20) + 10, 2);
                    element->fontDef.rect.size.height = charHeight;

                    int16_t xoffset = 0; memcpy(&xoffset, pData + (i * 20) + 12, 2);
                    element->fontDef.xOffset = xoffset;

                    int16_t yoffset = 0; memcpy(&yoffset, pData + (i * 20) + 14, 2);
                    element->fontDef.yOffset = yoffset;

                    int16_t xadvance = 0; memcpy(&xadvance, pData + (i * 20) + 16, 2);
                    element->fontDef.xAdvance = xadvance;

                    element->key = element->fontDef.charID;
                    HASH_ADD_INT(_fontDefDictionary, key, element);

                    validCharsString->insert(element->fontDef.charID);
                }
            }
            else if (blockId == 5) {
                /*
                 first  4   uint    0+c*10 	These fields are repeated until all kerning pairs have been described
                 second 4   uint    4+c*10
                 amount 2   int     8+c*10
                 */

                unsigned long count = blockSize / 20;

                for (unsigned long i = 0; i < count; i++)
                {
                    uint32_t first = 0; memcpy(&first, pData + (i * 10), 4);
                    uint32_t second = 0; memcpy(&second, pData + (i * 10) + 4, 4);
                    int16_t amount = 0; memcpy(&amount, pData + (i * 10) + 8, 2);

                    tKerningHashElement *element = (tKerningHashElement *)calloc( sizeof( *element ), 1 );
                    element->amount = amount;
                    element->key = (first<<16) | (second&0xffff);
                    HASH_ADD_INT(_kerningDictionary,key, element);
                }
            }

            pData += blockSize; remains -= blockSize;
        }

        return validCharsString;
    }

    void BMFontConfiguration::parseImageFileName(const char* line, const std::string& fntFile)
    {
        //////////////////////////////////////////////////////////////////////////
        // line to parse:
        // page id=0 file="bitmapFontTest.png"
        //////////////////////////////////////////////////////////////////////////

        // page ID. Sanity check
        int pageId;
        sscanf(line, "page id=%d", &pageId);
        // file
        char fileName[255];
        sscanf(strchr(line,'"') + 1, "%[^\"]", fileName);
        _atlasName = IO::FileUtils::getInstance().fullPathFromRelativeFile(fileName, fntFile);
    }

    void BMFontConfiguration::parseInfoArguments(const char* line)
    {
        //////////////////////////////////////////////////////////////////////////
        // possible lines to parse:
        // info face="Script" size=32 bold=0 italic=0 charset="" unicode=1 stretchH=100 smooth=1 aa=1 padding=1,4,3,2 spacing=0,0 outline=0
        // info face="Cracked" size=36 bold=0 italic=0 charset="" unicode=0 stretchH=100 smooth=1 aa=1 padding=0,0,0,0 spacing=1,1
        //////////////////////////////////////////////////////////////////////////

        // padding
        sscanf(strstr(line,"padding=") + 8, "%d,%d,%d,%d", &_padding.top, &_padding.right, &_padding.bottom, &_padding.left);
        //CCLOG("cocos2d: padding: %d,%d,%d,%d", _padding.left, _padding.top, _padding.right, _padding.bottom);
    }

    void BMFontConfiguration::parseCommonArguments(const char* line)
    {
        //////////////////////////////////////////////////////////////////////////
        // line to parse:
        // common lineHeight=104 base=26 scaleW=1024 scaleH=512 pages=1 packed=0
        //////////////////////////////////////////////////////////////////////////

        // Height
        auto tmp = strstr(line, "lineHeight=") + 11;
        sscanf(tmp, "%d", &_commonHeight);
        // scaleW. sanity check
        int value;
        tmp = strstr(tmp, "scaleW=") + 7;
        sscanf(tmp, "%d", &value);

        // scaleH. sanity check
        tmp = strstr(tmp, "scaleH=") + 7;
        sscanf(tmp, "%d", &value);

        // pages. sanity check
        tmp = strstr(tmp, "pages=") + 6;
        sscanf(tmp, "%d", &value);

        // packed (ignore) What does this mean ??
    }

    void BMFontConfiguration::parseCharacterDefinition(const char* line, BMFontDef *characterDefinition)
    {
        //////////////////////////////////////////////////////////////////////////
        // line to parse:
        // char id=32   x=0     y=0     width=0     height=0     xoffset=0     yoffset=44    xadvance=14     page=0  chnl=0
        //////////////////////////////////////////////////////////////////////////

        // Character ID
        auto tmp = strstr(line, "id=") + 3;
        sscanf(tmp, "%u", &characterDefinition->charID);

        // Character x
        tmp = strstr(tmp, "x=") + 2;
        sscanf(tmp, "%f", &characterDefinition->rect.origin.x);
        // Character y
        tmp = strstr(tmp, "y=") + 2;
        sscanf(tmp, "%f", &characterDefinition->rect.origin.y);
        // Character width
        tmp = strstr(tmp, "width=") + 6;
        sscanf(tmp, "%f", &characterDefinition->rect.size.width);
        // Character height
        tmp = strstr(tmp, "height=") + 7;
        sscanf(tmp, "%f", &characterDefinition->rect.size.height);
        // Character xoffset
        tmp = strstr(tmp, "xoffset=") + 8;
        sscanf(tmp, "%hd", &characterDefinition->xOffset);
        // Character yoffset
        tmp = strstr(tmp, "yoffset=") + 8;
        sscanf(tmp, "%hd", &characterDefinition->yOffset);
        // Character xadvance
        tmp = strstr(tmp, "xadvance=") + 9;
        sscanf(tmp, "%hd", &characterDefinition->xAdvance);
    }

    void BMFontConfiguration::parseKerningEntry(const char* line)
    {
        //////////////////////////////////////////////////////////////////////////
        // line to parse:
        // kerning first=121  second=44  amount=-7
        //////////////////////////////////////////////////////////////////////////

        int first, second, amount;
        auto tmp = strstr(line, "first=") + 6;
        sscanf(tmp, "%d", &first);

        tmp = strstr(tmp, "second=") + 7;
        sscanf(tmp, "%d", &second);

        tmp = strstr(tmp, "amount=") + 7;
        sscanf(tmp, "%d", &amount);

        tKerningHashElement *element = (tKerningHashElement *)calloc( sizeof( *element ), 1 );
        element->amount = amount;
        element->key = (first<<16) | (second&0xffff);
        HASH_ADD_INT(_kerningDictionary,key, element);
    }

    FontFNT * FontFNT::create(const std::string& fntFilePath, const MATH::Vector2f& imageOffset)
    {
        BMFontConfiguration *newConf = FNTConfigLoadFile(fntFilePath);
        if (!newConf)
            return nullptr;

        // add the texture
        Texture2D *tempTexture = Director::getInstance()->getTextureCache()->addImage(newConf->getAtlasName());
        if (!tempTexture)
        {
            return nullptr;
        }

        FontFNT *tempFont =  new FontFNT(newConf,imageOffset);

        if (!tempFont)
        {
            return nullptr;
        }
        tempFont->autorelease();
        return tempFont;
    }

    FontFNT::FontFNT(BMFontConfiguration *theContfig, const MATH::Vector2f& imageOffset)
    :_configuration(theContfig)
    ,_imageOffset(CC_POINT_PIXELS_TO_POINTS(imageOffset))
    {
        _configuration->retain();
    }

    FontFNT::~FontFNT()
    {
        _configuration->release();
    }

    void FontFNT::purgeCachedData()
    {
        if (s_configurations)
        {
            s_configurations->clear();
            SAFE_DELETE(s_configurations);
        }
    }

    int * FontFNT::getHorizontalKerningForTextUTF16(const std::u16string& text, int &outNumLetters) const
    {
        outNumLetters = static_cast<int>(text.length());

        if (!outNumLetters)
            return 0;

        int *sizes = new int[outNumLetters];
        if (!sizes)
            return 0;

        for (int c = 0; c < outNumLetters; ++c)
        {
            if (c < (outNumLetters-1))
                sizes[c] = getHorizontalKerningForChars(text[c], text[c+1]);
            else
                sizes[c] = 0;
        }

        return sizes;
    }

    int  FontFNT::getHorizontalKerningForChars(unsigned short firstChar, unsigned short secondChar) const
    {
        int ret = 0;
        unsigned int key = (firstChar << 16) | (secondChar & 0xffff);

        if (_configuration->_kerningDictionary)
        {
            tKerningHashElement *element = nullptr;
            HASH_FIND_INT(_configuration->_kerningDictionary, &key, element);

            if (element)
                ret = element->amount;
        }

        return ret;
    }

    FontAtlas * FontFNT::createFontAtlas()
    {
        FontAtlas *tempAtlas = new (std::nothrow) FontAtlas(*this);
        if (!tempAtlas)
            return nullptr;

        // check that everything is fine with the BMFontCofniguration
        if (!_configuration->_fontDefDictionary)
            return nullptr;

        size_t numGlyphs = _configuration->_characterSet->size();
        if (!numGlyphs)
            return nullptr;

        if (_configuration->_commonHeight == 0)
            return nullptr;

        // commone height
        tempAtlas->setCommonLineHeight(_configuration->_commonHeight);


        BMFontDef fontDef;
        tFontDefHashElement *currentElement, *tmp;

        // Purge uniform hash
        HASH_ITER(hh, _configuration->_fontDefDictionary, currentElement, tmp)
        {

            FontLetterDefinition tempDefinition;

            fontDef = currentElement->fontDef;
            MATH::Rectf tempRect;

            tempRect = fontDef.rect;
            tempRect = CC_RECT_PIXELS_TO_POINTS(tempRect);

            tempDefinition.letteCharUTF16 = fontDef.charID;

            tempDefinition.offsetX  = fontDef.xOffset;
            tempDefinition.offsetY  = fontDef.yOffset;

            tempDefinition.U        = tempRect.origin.x + _imageOffset.x;
            tempDefinition.V        = tempRect.origin.y + _imageOffset.y;

            tempDefinition.width    = tempRect.size.width;
            tempDefinition.height   = tempRect.size.height;

            //carloX: only one texture supported FOR NOW
            tempDefinition.textureID = 0;

            tempDefinition.validDefinition = true;
            tempDefinition.xAdvance = fontDef.xAdvance;
            // add the new definition
            tempAtlas->addLetterDefinition(tempDefinition);
        }

        // add the texture (only one texture for now)

        Texture2D *tempTexture = Director::getInstance()->getTextureCache()->addImage(_configuration->getAtlasName());
        if (!tempTexture)
            return 0;

        // add the texture
        tempAtlas->addTexture(tempTexture, 0);

        // done
        return tempAtlas;
    }

    FontCharMap * FontCharMap::create(const std::string& plistFile)
    {
        std::string pathStr = IO::FileUtils::getInstance().fullPathForFilename(plistFile);
        std::string relPathStr = pathStr.substr(0, pathStr.find_last_of("/"))+"/";

        ValueMap dict = IO::FileUtils::getInstance().getValueMapFromFile(pathStr.c_str());

        std::string textureFilename = relPathStr + dict["textureFilename"].asString();

        unsigned int width = dict["itemWidth"].asInt();
        unsigned int height = dict["itemHeight"].asInt();
        unsigned int startChar = dict["firstChar"].asInt();

        Texture2D *tempTexture = Director::getInstance()->getTextureCache()->addImage(textureFilename);
        if (!tempTexture)
        {
            return nullptr;
        }

        FontCharMap *tempFont =  new FontCharMap(tempTexture,width,height,startChar);

        if (!tempFont)
        {
            return nullptr;
        }
        tempFont->autorelease();
        return tempFont;
    }

    FontCharMap* FontCharMap::create(const std::string& charMapFile, int itemWidth, int itemHeight, int startCharMap)
    {
        Texture2D *tempTexture = Director::getInstance()->getTextureCache()->addImage(charMapFile);

        if (!tempTexture)
        {
            return nullptr;
        }

        FontCharMap *tempFont =  new FontCharMap(tempTexture,itemWidth,itemHeight,startCharMap);

        if (!tempFont)
        {
            return nullptr;
        }
        tempFont->autorelease();
        return tempFont;
    }

    FontCharMap* FontCharMap::create(Texture2D* texture, int itemWidth, int itemHeight, int startCharMap)
    {
        FontCharMap *tempFont =  new FontCharMap(texture,itemWidth,itemHeight,startCharMap);

        if (!tempFont)
        {
            return nullptr;
        }
        tempFont->autorelease();
        return tempFont;
    }

    FontCharMap::~FontCharMap()
    {

    }

    int * FontCharMap::getHorizontalKerningForTextUTF16(const std::u16string& text, int &outNumLetters) const
    {
        outNumLetters = static_cast<int>(text.length());

        if (outNumLetters <= 0)
            return nullptr;

        auto kernings = new int[outNumLetters];
        if (!kernings)
            return nullptr;

        memset(kernings, 0, outNumLetters * sizeof(int));

        return kernings;
    }

    FontAtlas * FontCharMap::createFontAtlas()
    {
        FontAtlas *tempAtlas = new (std::nothrow) FontAtlas(*this);
        if (!tempAtlas)
            return nullptr;

        MATH::Sizef s = _texture->getContentSizeInPixels();
        int itemsPerColumn = (int)(s.height / _itemHeight);
        int itemsPerRow = (int)(s.width / _itemWidth);

        tempAtlas->setCommonLineHeight(_itemHeight);

        auto contentScaleFactor = CC_CONTENT_SCALE_FACTOR();

        FontLetterDefinition tempDefinition;
        tempDefinition.textureID = 0;
        tempDefinition.offsetX  = 0.0f;
        tempDefinition.offsetY  = 0.0f;
        tempDefinition.validDefinition = true;
        tempDefinition.width = _itemWidth / contentScaleFactor;
        tempDefinition.height = _itemHeight / contentScaleFactor;
        tempDefinition.xAdvance = _itemWidth;

        int charId = _mapStartChar;
        for (int row = 0; row < itemsPerColumn; ++row)
        {
            for (int col = 0; col < itemsPerRow; ++col)
            {
                tempDefinition.letteCharUTF16 = charId;

                tempDefinition.U = _itemWidth * col / contentScaleFactor;
                tempDefinition.V = _itemHeight * row / contentScaleFactor;

                tempAtlas->addLetterDefinition(tempDefinition);
                charId++;
            }
        }

        tempAtlas->addTexture(_texture,0);

        return tempAtlas;
    }
}
