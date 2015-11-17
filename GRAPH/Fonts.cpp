#include <string.h>
#include "GRAPH/Fonts.h"
#include "GRAPH/Director.h"
#include "GRAPH/EventDispatcher.h"
#include "GRAPH/UNITY3D/TextureCache.h"
#include "IO/FileUtils.h"
#include "UTILS/STRING/StringUtils.h"
#include "UTILS/STRING/UTFUtils.h"

namespace GRAPH
{
    Font::Font()
        : usedGlyphs_(GlyphCollection::DYNAMIC)
        , customGlyphs_(nullptr) {

    }

    void Font::setCurrentGlyphCollection(GlyphCollection glyphs, const char *customGlyphs) {
        if (customGlyphs_)
            delete [] customGlyphs_;

        if (customGlyphs) {
            uint64 length = strlen(customGlyphs);
            customGlyphs_ = new char [length + 2];
            memcpy(customGlyphs_, customGlyphs, length);

            customGlyphs_[length]   = 0;
            customGlyphs_[length+1] = 0;
        }

        usedGlyphs_ = glyphs;
    }

    const char *Font::getCurrentGlyphCollection() const {
        if (customGlyphs_) {
            return customGlyphs_;
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
        : font_(&theFont)
        , currentPageData_(nullptr)
        , rendererRecreatedListener_(nullptr)
        , antialiasEnabled_(true)
        , currLineHeight_(0) {

        font_->retain();
        Font* fontTTf = font_;
        if (fontTTf) {
            commonLineHeight_ = font_->getFontMaxHeight();
            auto texture = Unity3DCreator::CreateTexture();
            currentPage_ = 0;
            currentPageOrigX_ = 0;
            currentPageOrigY_ = 0;
            letterPadding_ = 0;
            currentPageDataSize_ = CacheTextureWidth * CacheTextureHeight;
            currentPageData_ = new unsigned char[currentPageDataSize_];
            memset(currentPageData_, 0, currentPageDataSize_);

            auto  pixelFormat = IMAGE::ImageFormat::A8;
            texture->initWithData(currentPageData_, currentPageDataSize_,
                pixelFormat, CacheTextureWidth, CacheTextureHeight);

            addTexture(texture,0);
            texture->release();
        }
    }

    FontAtlas::~FontAtlas() {
        font_->release();
        relaseTextures();

        delete []currentPageData_;
    }

    void FontAtlas::relaseTextures() {
        for( auto &item: atlasTextures_) {
            item.second->release();
        }
        atlasTextures_.clear();
    }

    void FontAtlas::purgeTexturesAtlas() {
        Font* fontTTf = font_;
        if (fontTTf && atlasTextures_.size() > 1) {
            auto eventDispatcher = Director::getInstance().getEventDispatcher();
            eventDispatcher->dispatchCustomEvent(CMD_PURGE_FONTATLAS,this);
            eventDispatcher->dispatchCustomEvent(CMD_RESET_FONTATLAS,this);
        }
    }

    void FontAtlas::listenRendererRecreated(EventCustom *) {
        Font* fontTTf = font_;
        if (fontTTf) {
            auto eventDispatcher = Director::getInstance().getEventDispatcher();
            eventDispatcher->dispatchCustomEvent(CMD_PURGE_FONTATLAS,this);
            eventDispatcher->dispatchCustomEvent(CMD_RESET_FONTATLAS,this);
        }
    }

    void FontAtlas::addLetterDefinition(const FontLetterDefinition &letterDefinition) {
        letterDefinitions_[letterDefinition.letteCharUTF16] = letterDefinition;
    }

    bool FontAtlas::getLetterDefinitionForChar(char16_t letteCharUTF16, FontLetterDefinition &outDefinition) {
        auto outIterator = letterDefinitions_.find(letteCharUTF16);

        if (outIterator != letterDefinitions_.end()) {
            outDefinition = (*outIterator).second;
            return true;
        }
        else {
            return false;
        }
    }

    bool FontAtlas::prepareLetterDefinitions(const std::u16string& utf16String) {
        Font* fontTTf = font_;
        if(fontTTf == nullptr)
            return false;

        uint64 length = utf16String.length();
        long bitmapWidth;
        long bitmapHeight;
        FontLetterDefinition tempDef;

        auto  pixelFormat = IMAGE::ImageFormat::A8;

        bool existNewLetter = false;
        float startY = currentPageOrigY_;

        for (uint64 i = 0; i < length; ++i) {
            auto outIterator = letterDefinitions_.find(utf16String[i]);

            if (outIterator == letterDefinitions_.end()) {
                existNewLetter = true;

                auto bitmap = fontTTf->getGlyphBitmap(utf16String[i],bitmapWidth,bitmapHeight);
                if (bitmap) {
                    tempDef.letteCharUTF16   = utf16String[i];
                    tempDef.width            = bitmapWidth + letterPadding_;
                    tempDef.height           = bitmapHeight + letterPadding_;

                    if (bitmapHeight > currLineHeight_) {
                        currLineHeight_ = bitmapHeight + 1;
                    }
                    if (currentPageOrigX_ + tempDef.width > CacheTextureWidth) {
                        currentPageOrigY_ += currLineHeight_;
                        currLineHeight_ = 0;
                        currentPageOrigX_ = 0;
                        if(currentPageOrigY_ + commonLineHeight_ >= CacheTextureHeight) {
                            unsigned char *data = nullptr;
                            if(pixelFormat == IMAGE::ImageFormat::AI88) {
                                data = currentPageData_ + CacheTextureWidth * (int)startY * 2;
                            }
                            else {
                                data = currentPageData_ + CacheTextureWidth * (int)startY;
                            }
                            atlasTextures_[currentPage_]->updateWithData(data, 0, startY,
                                CacheTextureWidth, CacheTextureHeight - startY);

                            startY = 0.0f;

                            currentPageOrigY_ = 0;
                            memset(currentPageData_, 0, currentPageDataSize_);
                            currentPage_++;
                            auto tex = Unity3DCreator::CreateTexture();
                            tex->setAliasTexParameters();
                            tex->initWithData(currentPageData_, currentPageDataSize_,
                                pixelFormat, CacheTextureWidth, CacheTextureHeight);
                            addTexture(tex,currentPage_);
                            tex->release();
                        }
                    }

                    renderCharAt(currentPageData_,currentPageOrigX_,currentPageOrigY_,bitmap,bitmapWidth,bitmapHeight);

                    tempDef.U                = currentPageOrigX_;
                    tempDef.V                = currentPageOrigY_;
                    tempDef.textureID        = currentPage_;
                    currentPageOrigX_        += tempDef.width + 1;
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
                    currentPageOrigX_ += 1;
                }

                letterDefinitions_[tempDef.letteCharUTF16] = tempDef;
            }
        }

        if(existNewLetter) {
            unsigned char *data = nullptr;
            if(pixelFormat == IMAGE::ImageFormat::AI88) {
                data = currentPageData_ + CacheTextureWidth * (int)startY * 2;
            }
            else {
                data = currentPageData_ + CacheTextureWidth * (int)startY;
            }
            atlasTextures_[currentPage_]->updateWithData(data, 0, startY,
                                                         CacheTextureWidth, currentPageOrigY_ - startY + commonLineHeight_);
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

    void FontAtlas::addTexture(Unity3DTexture *texture, int slot) {
        texture->retain();
        atlasTextures_[slot] = texture;
    }

    Unity3DTexture* FontAtlas::getTexture(int slot) {
        return atlasTextures_[slot];
    }

    float FontAtlas::getCommonLineHeight() const {
        return commonLineHeight_;
    }

    void  FontAtlas::setCommonLineHeight(float newHeight) {
        commonLineHeight_ = newHeight;
    }

    const Font * FontAtlas::getFont() const {
        return font_;
    }

    void FontAtlas::setAliasTexParameters() {
        if (antialiasEnabled_) {
            antialiasEnabled_ = false;
            for (const auto & tex : atlasTextures_) {
                tex.second->setAliasTexParameters();
            }
        }
    }

    std::unordered_map<std::string, FontAtlas *> FontAtlasCache::atlasMap_;

    void FontAtlasCache::purgeCachedData() {
        for (auto & atlas:atlasMap_) {
            atlas.second->purgeTexturesAtlas();
        }
    }

    std::string FontAtlasCache::generateFontName(const std::string& fontFileName, int size, GlyphCollection theGlyphs, bool useDistanceField) {
        std::string tempName(fontFileName);

        switch (theGlyphs) {
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

    bool FontAtlasCache::releaseFontAtlas(FontAtlas *atlas) {
        if (nullptr != atlas) {
            for( auto &item: atlasMap_ ) {
                if ( item.second == atlas ) {
                    if (atlas->getReferenceCount() == 1) {
                        atlasMap_.erase(item.first);
                    }

                    atlas->release();

                    return true;
                }
            }
        }

        return false;
    }
}
