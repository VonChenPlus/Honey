#include "GRAPH/BASE/Label.h"
#include "GRAPH/BASE/Sprite.h"
#include "GRAPH/BASE/Macros.h"
#include "GRAPH/BASE/EventDispatcher.h"
#include "IO/FileUtils.h"
#include "GRAPH/RENDERER/GLProgram.h"
#include "UTILS/STRING/UTFUtils.h"
#include "GRAPH/BASE/Director.h"

namespace GRAPH
{
    class LabelTextFormatter
    {
    public:

        static bool multilineText(Label *theLabel);
        static bool alignText(Label *theLabel);
        static bool createStringSprites(Label *theLabel);
    };

    bool LabelTextFormatter::multilineText(Label *theLabel)
    {
        auto limit = theLabel->_limitShowCount;
        auto strWhole = theLabel->_currentUTF16String;

        std::vector<char16_t> multiline_string;
        multiline_string.reserve( limit );
        std::vector<char16_t> last_word;
        last_word.reserve( 25 );

        bool lineIsEmpty = true;
        bool calculateLineStart = false;
        float startOfLine = 0.f;

        int skip = 0;
        int tIndex = 0;
        float lineWidth = theLabel->_maxLineWidth;
        bool breakLineWithoutSpace = theLabel->_lineBreakWithoutSpaces;
        Label::LetterInfo* info = nullptr;

        for (int j = 0; j+skip < limit; j++)
        {
            info = & theLabel->_lettersInfo.at(j+skip);

            unsigned int justSkipped = 0;

            while (info->def.validDefinition == false)
            {
                justSkipped++;
                tIndex = j+skip+justSkipped;
                if (strWhole[tIndex-1] == '\n')
                {
                    UTILS::STRING::TrimUTF16Vector(last_word);

                    last_word.push_back('\n');
                    multiline_string.insert(multiline_string.end(), last_word.begin(), last_word.end());
                    last_word.clear();

                    calculateLineStart = false;
                    startOfLine = 0.f;
                    lineIsEmpty = true;
                }
                if(tIndex < limit)
                {
                    info = & theLabel->_lettersInfo.at( tIndex );
                }
                else
                    break;
            }
            skip += justSkipped;
            tIndex = j + skip;

            if (tIndex >= limit)
                break;

            if (calculateLineStart)
            {
                startOfLine = info->position.x - info->def.offsetX - theLabel->_horizontalKernings[tIndex];
                calculateLineStart = false;
                lineIsEmpty = true;
            }

            auto character = strWhole[tIndex];
            if (breakLineWithoutSpace)
            {
                float posRight = info->position.x + info->contentSize.width;
                if (posRight - startOfLine > lineWidth)
                {
                    //issue #8492:endless loop if not using system font, and constrained length is less than one character width
                    if (last_word.empty())
                        last_word.push_back(character);
                    else
                        --j;

                    last_word.push_back('\n');
                    multiline_string.insert(multiline_string.end(), last_word.begin(), last_word.end());
                    last_word.clear();

                    startOfLine += lineWidth;
                }
                else
                {
                    last_word.push_back(character);
                }
            }
            //Break line with space.
            else
            {
                std::vector<char16_t> nonCJKword;
                int wordIndex = tIndex;
                for (; wordIndex < limit; ++wordIndex)
                {
                    auto ch = strWhole[wordIndex];
                    if (ch == '\n' || UTILS::STRING::IsUnicodeSpace(ch) || UTILS::STRING::IsCJKUnicode(ch))
                    {
                        break;
                    }

                    nonCJKword.push_back(ch);
                }

                if (!nonCJKword.empty())
                {
                    auto wordLenth = nonCJKword.size();
                    auto lastCharacterInfo = &theLabel->_lettersInfo.at(tIndex + wordLenth - 1);

                    float posRight = lastCharacterInfo->position.x + lastCharacterInfo->contentSize.width;
                    if (posRight - startOfLine > lineWidth)
                    {
                        if (last_word.empty())
                        {
                            nonCJKword.push_back('\n');
                            multiline_string.insert(multiline_string.end(), nonCJKword.begin(), nonCJKword.end());

                            calculateLineStart = true;
                        }
                        else
                        {
                            last_word.push_back('\n');
                            multiline_string.insert(multiline_string.end(), last_word.begin(), last_word.end());
                            last_word.clear();

                            startOfLine = info->position.x - info->def.offsetX - theLabel->_horizontalKernings[tIndex];
                            if (posRight - startOfLine > lineWidth)
                            {
                                nonCJKword.push_back('\n');
                                multiline_string.insert(multiline_string.end(), nonCJKword.begin(), nonCJKword.end());
                                calculateLineStart = true;
                            }
                            else
                            {
                                multiline_string.insert(multiline_string.end(), nonCJKword.begin(), nonCJKword.end());
                                lineIsEmpty = false;
                                calculateLineStart = false;
                            }
                        }
                    }
                    else
                    {
                        multiline_string.insert(multiline_string.end(), last_word.begin(), last_word.end());
                        last_word.clear();

                        multiline_string.insert(multiline_string.end(), nonCJKword.begin(), nonCJKword.end());
                        lineIsEmpty = false;
                    }

                    j += wordLenth - 1;
                    continue;
                }

                float posRight = info->position.x + info->contentSize.width;
                if (posRight - startOfLine > lineWidth)
                {
                    //issue #8492:endless loop if not using system font, and constrained length is less than one character width
                    if (lineIsEmpty && last_word.empty())
                        last_word.push_back(character);
                    else
                        --j;

                    last_word.push_back('\n');
                    multiline_string.insert(multiline_string.end(), last_word.begin(), last_word.end());
                    last_word.clear();

                    calculateLineStart = true;
                }
                else
                {
                    last_word.push_back(character);
                }
            }
        }

        multiline_string.insert(multiline_string.end(), last_word.begin(), last_word.end());
        std::u16string strNew(multiline_string.begin(), multiline_string.end());

        theLabel->_currentUTF16String = strNew;
        theLabel->computeStringNumLines();
        theLabel->computeHorizontalKernings(theLabel->_currentUTF16String);

        return true;
    }

    bool LabelTextFormatter::alignText(Label *theLabel)
    {
        int i = 0;

        int lineNumber = 0;
        int strLen = theLabel->_limitShowCount;
        std::vector<char16_t> lastLine;
        auto strWhole = theLabel->_currentUTF16String;

        if (theLabel->_labelWidth > theLabel->_contentSize.width)
        {
            theLabel->setContentSize(MATH::Sizef(theLabel->_labelWidth,theLabel->_contentSize.height));
        }

        for (int ctr = 0; ctr <= strLen; ++ctr)
        {
            char16_t currentChar = strWhole[ctr];

            if (currentChar == '\n' || currentChar == 0)
            {
                auto lineLength = lastLine.size();

                // if last line is empty we must just increase lineNumber and work with next line
                if (lineLength == 0)
                {
                    lineNumber++;
                    continue;
                }
                int index = static_cast<int>(i + lineLength - 1 + lineNumber);
                if (index < 0) continue;

                auto info = & theLabel->_lettersInfo.at( index );
                if(info->def.validDefinition == false)
                    continue;

                float shift = 0;
                switch (theLabel->_hAlignment)
                {
                    case TextHAlignment::CENTER:
                        {
                            float lineWidth = info->position.x + info->contentSize.width;
                            shift = theLabel->_contentSize.width/2.0f - lineWidth/2.0f;
                            break;
                        }
                    case TextHAlignment::RIGHT:
                        {
                            float lineWidth = info->position.x + info->contentSize.width;
                            shift = theLabel->_contentSize.width - lineWidth;
                            break;
                        }
                    default:
                        break;
                }

                if (shift != 0)
                {
                    for (unsigned j = 0; j < lineLength; ++j)
                    {
                        index = i + j + lineNumber;
                        if (index < 0) continue;

                        info = & theLabel->_lettersInfo.at( index );
                        if(info)
                        {
                            info->position.x += shift;
                        }
                    }
                }

                i += lineLength;
                ++lineNumber;

                lastLine.clear();
                continue;
            }

            lastLine.push_back(currentChar);
        }

        return true;
    }

    bool LabelTextFormatter::createStringSprites(Label *theLabel)
    {
        theLabel->_limitShowCount = 0;
        // check for string
        int stringLen = theLabel->getStringLength();
        if (stringLen <= 0)
            return false;

        auto totalHeight = theLabel->_commonLineHeight * theLabel->_currNumLines;
        auto longestLine = 0.0f;
        auto nextFontPositionX = 0.0f;
        auto nextFontPositionY = totalHeight;
        auto contentScaleFactor = CC_CONTENT_SCALE_FACTOR();

        if (theLabel->_labelHeight > 0)
        {
            auto labelHeightPixel = theLabel->_labelHeight * contentScaleFactor;
            if (totalHeight > labelHeightPixel)
            {
                int numLines = labelHeightPixel / theLabel->_commonLineHeight;
                totalHeight = numLines * theLabel->_commonLineHeight;
            }
            switch (theLabel->_vAlignment)
            {
            case TextVAlignment::TOP:
                nextFontPositionY = labelHeightPixel;
                break;
            case TextVAlignment::CENTER:
                nextFontPositionY = (labelHeightPixel + totalHeight) / 2.0f;
                break;
            case TextVAlignment::BOTTOM:
                nextFontPositionY = totalHeight;
                break;
            default:
                break;
            }
        }

        int charXOffset = 0;
        int charYOffset = 0;
        int charAdvance = 0;

        auto strWhole = theLabel->_currentUTF16String;
        auto fontAtlas = theLabel->_fontAtlas;
        FontLetterDefinition tempDefinition;
        MATH::Vector2f letterPosition;
        const auto& kernings = theLabel->_horizontalKernings;

        float clipTop = 0;
        float clipBottom = 0;
        int lineIndex = 0;
        bool lineStart = true;
        bool clipBlank = false;
        if (theLabel->_currentLabelType == Label::LabelType::TTF && theLabel->_clipEnabled)
        {
            clipBlank = true;
        }

        for (int i = 0; i < stringLen; i++)
        {
            char16_t c    = strWhole[i];
            if (fontAtlas->getLetterDefinitionForChar(c, tempDefinition))
            {
                charXOffset         = tempDefinition.offsetX;
                charYOffset         = tempDefinition.offsetY;
                charAdvance         = tempDefinition.xAdvance;
            }
            else
            {
                charXOffset         = -1;
                charYOffset         = -1;
                charAdvance         = -1;
            }

            if (c == '\n')
            {
                lineIndex++;
                nextFontPositionX  = 0;
                nextFontPositionY -= theLabel->_commonLineHeight;

                theLabel->recordPlaceholderInfo(i);
                if (nextFontPositionY < theLabel->_commonLineHeight)
                    break;

                lineStart = true;
                continue;
            }
            else if (clipBlank && tempDefinition.height > 0.0f)
            {
                if (lineStart)
                {
                    if (lineIndex == 0)
                    {
                        clipTop = charYOffset;
                    }
                    lineStart = false;
                    clipBottom = tempDefinition.clipBottom;
                }
                else if(tempDefinition.clipBottom < clipBottom)
                {
                    clipBottom = tempDefinition.clipBottom;
                }

                if (lineIndex == 0 && charYOffset < clipTop)
                {
                    clipTop = charYOffset;
                }
            }

            letterPosition.x = (nextFontPositionX + charXOffset) / contentScaleFactor;
            letterPosition.y = (nextFontPositionY - charYOffset) / contentScaleFactor;

            if( theLabel->recordLetterInfo(letterPosition, tempDefinition, i) == false)
            {
                continue;
            }

            nextFontPositionX += charAdvance + theLabel->_additionalKerning;
            if (i < stringLen - 1)
            {
                nextFontPositionX += kernings[i + 1];
            }

            auto letterRight = letterPosition.x + tempDefinition.width;
            if (longestLine < letterRight)
            {
                longestLine = letterRight;
            }
        }

        MATH::Sizef tmpSize(longestLine * contentScaleFactor, totalHeight);
        if (theLabel->_labelHeight > 0)
        {
            tmpSize.height = theLabel->_labelHeight * contentScaleFactor;
        }

        if (clipBlank)
        {
            int clipTotal = (clipTop + clipBottom) / contentScaleFactor;
            tmpSize.height -= clipTotal * contentScaleFactor;
            clipBottom /= contentScaleFactor;

            for (int i = 0; i < theLabel->_limitShowCount; i++)
            {
                theLabel->_lettersInfo[i].position.y -= clipBottom;
            }
        }

        theLabel->setContentSize(CC_SIZE_PIXELS_TO_POINTS(tmpSize));

        return true;
    }

    const int Label::DistanceFieldFontSize = 50;

    /**
     * LabelLetter used to update the quad in texture atlas without SpriteBatchNode.
     */
    class LabelLetter : public Sprite
    {
    public:
        LabelLetter()
        {
            _textureAtlas = nullptr;
        }

        static LabelLetter* createWithTexture(Texture2D *texture, const MATH::Rectf& rect, bool rotated = false)
        {
            auto letter = new (std::nothrow) LabelLetter();
            if (letter && letter->initWithTexture(texture, rect, rotated))
            {
                letter->setVisible(false);
                letter->autorelease();
                return letter;
            }
            SAFE_DELETE(letter);
            return nullptr;
        }

        virtual void updateTransform() override
        {
            if (isDirty())
            {
                _transformToBatch = getNodeToParentTransform();
                MATH::Sizef &size = _rect.size;

                float x1 = _offsetPosition.x;
                float y1 = _offsetPosition.y;
                float x2 = x1 + size.width;
                float y2 = y1 + size.height;
                float x = _transformToBatch.m[12];
                float y = _transformToBatch.m[13];

                float cr = _transformToBatch.m[0];
                float sr = _transformToBatch.m[1];
                float cr2 = _transformToBatch.m[5];
                float sr2 = -_transformToBatch.m[4];
                float ax = x1 * cr - y1 * sr2 + x;
                float ay = x1 * sr + y1 * cr2 + y;

                float bx = x2 * cr - y1 * sr2 + x;
                float by = x2 * sr + y1 * cr2 + y;
                float cx = x2 * cr - y2 * sr2 + x;
                float cy = x2 * sr + y2 * cr2 + y;
                float dx = x1 * cr - y2 * sr2 + x;
                float dy = x1 * sr + y2 * cr2 + y;

                _quad.bl.vertices.set(ax, ay, _positionZ);
                _quad.br.vertices.set(bx, by, _positionZ);
                _quad.tl.vertices.set(dx, dy, _positionZ);
                _quad.tr.vertices.set(cx, cy, _positionZ);

                if (_textureAtlas)
                {
                    _textureAtlas->updateQuad(&_quad, _atlasIndex);
                }

                _recursiveDirty = false;
                setDirty(false);
            }

            Node::updateTransform();
        }

        virtual void updateColor()
        {
            if (_textureAtlas == nullptr)
            {
                return;
            }

            Color4B color4(_displayedColor.red, _displayedColor.green, _displayedColor.blue, _displayedOpacity);
            // special opacity for premultiplied textures
            if (_opacityModifyRGB)
            {
                color4.red *= _displayedOpacity / 255.0f;
                color4.green *= _displayedOpacity / 255.0f;
                color4.blue *= _displayedOpacity / 255.0f;
            }
            _quad.bl.colors = color4;
            _quad.br.colors = color4;
            _quad.tl.colors = color4;
            _quad.tr.colors = color4;

            _textureAtlas->updateQuad(&_quad, _atlasIndex);
        }
    };

    Label* Label::create()
    {
        auto ret = new (std::nothrow) Label();

        if (ret)
        {
            ret->autorelease();
        }

        return ret;
    }

    Label* Label::createWithSystemFont(const std::string& text, const std::string& font, float fontSize, const MATH::Sizef& dimensions /* = MATH::SizefZERO */, TextHAlignment hAlignment /* = TextHAlignment::LEFT */, TextVAlignment vAlignment /* = TextVAlignment::TOP */)
    {
        auto ret = new (std::nothrow) Label(nullptr,hAlignment,vAlignment);

        if (ret)
        {
            ret->setSystemFontName(font);
            ret->setSystemFontSize(fontSize);
            ret->setDimensions(dimensions.width, dimensions.height);
            ret->setString(text);

            ret->autorelease();

            return ret;
        }

        delete ret;
        return nullptr;
    }

    Label* Label::createWithTTF(const std::string& text, const std::string& fontFile, float fontSize, const MATH::Sizef& dimensions /* = MATH::SizefZERO */, TextHAlignment hAlignment /* = TextHAlignment::LEFT */, TextVAlignment vAlignment /* = TextVAlignment::TOP */)
    {
        auto ret = new (std::nothrow) Label(nullptr,hAlignment,vAlignment);

        if (ret && IO::FileUtils::getInstance().isFileExist(fontFile))
        {
            TTFConfig ttfConfig(fontFile.c_str(),fontSize,GlyphCollection::DYNAMIC);
            if (ret->setTTFConfig(ttfConfig))
            {
                ret->setDimensions(dimensions.width,dimensions.height);
                ret->setString(text);

                ret->autorelease();

                return ret;
            }
        }

        delete ret;
        return nullptr;
    }

    Label* Label::createWithTTF(const TTFConfig& ttfConfig, const std::string& text, TextHAlignment alignment /* = TextHAlignment::CENTER */, int maxLineWidth /* = 0 */)
    {
        auto ret = new (std::nothrow) Label(nullptr,alignment);

        if (ret && IO::FileUtils::getInstance().isFileExist(ttfConfig.fontFilePath) && ret->setTTFConfig(ttfConfig))
        {
            ret->setMaxLineWidth(maxLineWidth);
            ret->setString(text);
            ret->autorelease();

            return ret;
        }

        delete ret;
        return nullptr;
    }

    Label* Label::createWithBMFont(const std::string& bmfontFilePath, const std::string& text,const TextHAlignment& alignment /* = TextHAlignment::LEFT */, int maxLineWidth /* = 0 */, const MATH::Vector2f& imageOffset /* = MATH::Vec2fZERO */)
    {
        auto ret = new (std::nothrow) Label(nullptr,alignment);

        if (ret && ret->setBMFontFilePath(bmfontFilePath,imageOffset))
        {
            ret->setMaxLineWidth(maxLineWidth);
            ret->setString(text);
            ret->autorelease();

            return ret;
        }

        delete ret;
        return nullptr;
    }

    Label* Label::createWithCharMap(const std::string& plistFile)
    {
        auto ret = new (std::nothrow) Label();

        if (ret && ret->setCharMap(plistFile))
        {
            ret->autorelease();
            return ret;
        }

        delete ret;
        return nullptr;
    }

    Label* Label::createWithCharMap(Texture2D* texture, int itemWidth, int itemHeight, int startCharMap)
    {
        auto ret = new (std::nothrow) Label();

        if (ret && ret->setCharMap(texture,itemWidth,itemHeight,startCharMap))
        {
            ret->autorelease();
            return ret;
        }

        delete ret;
        return nullptr;
    }

    Label* Label::createWithCharMap(const std::string& charMapFile, int itemWidth, int itemHeight, int startCharMap)
    {
        auto ret = new (std::nothrow) Label();

        if (ret && ret->setCharMap(charMapFile,itemWidth,itemHeight,startCharMap))
        {
            ret->autorelease();
            return ret;
        }

        delete ret;
        return nullptr;
    }

    bool Label::setCharMap(const std::string& plistFile)
    {
        auto newAtlas = FontAtlasCache::getFontAtlasCharMap(plistFile);

        if (!newAtlas)
        {
            reset();
            return false;
        }

        _currentLabelType = LabelType::CHARMAP;
        setFontAtlas(newAtlas);

        return true;
    }

    bool Label::setCharMap(Texture2D* texture, int itemWidth, int itemHeight, int startCharMap)
    {
        auto newAtlas = FontAtlasCache::getFontAtlasCharMap(texture,itemWidth,itemHeight,startCharMap);

        if (!newAtlas)
        {
            reset();
            return false;
        }

        _currentLabelType = LabelType::CHARMAP;
        setFontAtlas(newAtlas);

        return true;
    }

    bool Label::setCharMap(const std::string& charMapFile, int itemWidth, int itemHeight, int startCharMap)
    {
        auto newAtlas = FontAtlasCache::getFontAtlasCharMap(charMapFile,itemWidth,itemHeight,startCharMap);

        if (!newAtlas)
        {
            reset();
            return false;
        }

        _currentLabelType = LabelType::CHARMAP;
        setFontAtlas(newAtlas);

        return true;
    }

    Label::Label(FontAtlas *atlas /* = nullptr */, TextHAlignment hAlignment /* = TextHAlignment::LEFT */,
                 TextVAlignment vAlignment /* = TextVAlignment::TOP */,bool useDistanceField /* = false */,bool useA8Shader /* = false */)
    : _isOpacityModifyRGB(false)
    , _contentDirty(false)
    , _fontAtlas(atlas)
    , _textSprite(nullptr)
    , _compatibleMode(false)
    , _reusedLetter(nullptr)
    , _additionalKerning(0.0f)
    , _commonLineHeight(0.0f)
    , _lineBreakWithoutSpaces(false)
    , _horizontalKernings(nullptr)
    , _maxLineWidth(0.0f)
    , _labelDimensions(MATH::SizefZERO)
    , _labelWidth(0.0f)
    , _labelHeight(0.0f)
    , _hAlignment(hAlignment)
    , _vAlignment(vAlignment)
    , _currNumLines(-1)
    , _fontScale(1.0f)
    , _useDistanceField(useDistanceField)
    , _useA8Shader(useA8Shader)
    , _effectColorF(Color4F::BLACK)
    , _uniformEffectColor(0)
    , _shadowDirty(false)
    , _blendFunc(BlendFunc::ALPHA_PREMULTIPLIED)
    , _insideBounds(true)
    {
        setAnchorPoint(MATH::Vec2fMIDDLE);
        reset();

        _purgeTextureListener = EventListenerCustom::create(FontAtlas::CMD_PURGE_FONTATLAS, [this](EventCustom* event){
            if (_fontAtlas && _currentLabelType == LabelType::TTF && event->getUserData() == _fontAtlas)
            {
                for (auto it : _letters)
                {
                    it.second->setTexture(nullptr);
                }
                _batchNodes.clear();

                if (_fontAtlas)
                {
                    FontAtlasCache::releaseFontAtlas(_fontAtlas);
                }
            }
        });
        _eventDispatcher->addEventListenerWithFixedPriority(_purgeTextureListener, 1);

        _resetTextureListener = EventListenerCustom::create(FontAtlas::CMD_RESET_FONTATLAS, [this](EventCustom* event){
            if (_fontAtlas && _currentLabelType == LabelType::TTF && event->getUserData() == _fontAtlas)
            {
                _fontAtlas = nullptr;
                this->setTTFConfig(_fontConfig);
                for (auto it : _letters)
                {
                    getLetter(it.first);
                }
            }
        });
        _eventDispatcher->addEventListenerWithFixedPriority(_resetTextureListener, 2);
    }

    Label::~Label()
    {
        delete [] _horizontalKernings;

        if (_fontAtlas)
        {
            FontAtlasCache::releaseFontAtlas(_fontAtlas);
        }
        _eventDispatcher->removeEventListener(_purgeTextureListener);
        _eventDispatcher->removeEventListener(_resetTextureListener);

        SAFE_RELEASE_NULL(_reusedLetter);
        SAFE_RELEASE_NULL(_textSprite);
        SAFE_RELEASE_NULL(_shadowNode);
    }

    void Label::reset()
    {
        TTFConfig temp;
        _fontConfig = temp;

        _systemFontDirty = false;
        _systemFont = "Helvetica";
        _systemFontSize = 12;

        _batchNodes.clear();

        if (_fontAtlas)
        {
            FontAtlasCache::releaseFontAtlas(_fontAtlas);
            _fontAtlas = nullptr;
        }

        _currentLabelType = LabelType::STRING_TEXTURE;
        _currLabelEffect = LabelEffect::NORMAL;
        _shadowBlurRadius = 0;

        Node::removeAllChildrenWithCleanup(true);
        _textSprite = nullptr;
        _shadowNode = nullptr;

        SAFE_RELEASE_NULL(_reusedLetter);

        _textColor = Color4B::WHITE;
        _textColorF = Color4F::WHITE;
        setColor(Color3B::WHITE);

        _shadowEnabled = false;
        _clipEnabled = false;
        _blendFuncDirty = false;
    }

    void Label::updateShaderProgram()
    {
        switch (_currLabelEffect)
        {
        case LabelEffect::NORMAL:
            if (_useDistanceField)
                setGLProgramState(GLProgramState::getOrCreateWithGLProgramName(GLProgram::SHADER_NAME_LABEL_DISTANCEFIELD_NORMAL));
            else if (_useA8Shader)
                setGLProgramState(GLProgramState::getOrCreateWithGLProgramName(GLProgram::SHADER_NAME_LABEL_NORMAL));
            else
                setGLProgramState(GLProgramState::getOrCreateWithGLProgramName(GLProgram::SHADER_NAME_POSITION_TEXTURE_COLOR));

            break;
        case LabelEffect::OUTLINE:
            setGLProgramState(GLProgramState::getOrCreateWithGLProgramName(GLProgram::SHADER_NAME_LABEL_OUTLINE));
            _uniformEffectColor = glGetUniformLocation(getGLProgram()->getProgram(), "u_effectColor");
            break;
        case LabelEffect::GLOW:
            if (_useDistanceField)
            {
                setGLProgramState(GLProgramState::getOrCreateWithGLProgramName(GLProgram::SHADER_NAME_LABEL_DISTANCEFIELD_GLOW));
                _uniformEffectColor = glGetUniformLocation(getGLProgram()->getProgram(), "u_effectColor");
            }
            break;
        default:
            return;
        }

        _uniformTextColor = glGetUniformLocation(getGLProgram()->getProgram(), "u_textColor");
    }

    void Label::setFontAtlas(FontAtlas* atlas,bool distanceFieldEnabled /* = false */, bool useA8Shader /* = false */)
    {
        if (atlas == _fontAtlas)
        {
            FontAtlasCache::releaseFontAtlas(atlas);
            return;
        }

        if (_fontAtlas)
        {
            _batchNodes.clear();
            FontAtlasCache::releaseFontAtlas(_fontAtlas);
            _fontAtlas = nullptr;
        }

        _fontAtlas = atlas;
        if (_reusedLetter == nullptr)
        {
            _reusedLetter = Sprite::create();
            _reusedLetter->setOpacityModifyRGB(_isOpacityModifyRGB);
            _reusedLetter->retain();
            _reusedLetter->setAnchorPoint(MATH::Vec2fTOPLEFT);
        }

        if (_fontAtlas)
        {
            _commonLineHeight = _fontAtlas->getCommonLineHeight();
            _contentDirty = true;
        }
        _useDistanceField = distanceFieldEnabled;
        _useA8Shader = useA8Shader;

        if (_currentLabelType != LabelType::TTF)
        {
            _currLabelEffect = LabelEffect::NORMAL;
            updateShaderProgram();
        }
    }

    bool Label::setTTFConfig(const TTFConfig& ttfConfig)
    {
        FontAtlas *newAtlas = FontAtlasCache::getFontAtlasTTF(ttfConfig);

        if (!newAtlas)
        {
            reset();
            return false;
        }
        _systemFontDirty = false;

        _currentLabelType = LabelType::TTF;
        setFontAtlas(newAtlas,ttfConfig.distanceFieldEnabled,true);

        _fontConfig = ttfConfig;
        if (_fontConfig.outlineSize > 0)
        {
            _fontConfig.distanceFieldEnabled = false;
            _useDistanceField = false;
            _useA8Shader = false;
            _currLabelEffect = LabelEffect::OUTLINE;
            updateShaderProgram();
        }
        else
        {
            _currLabelEffect = LabelEffect::NORMAL;
            updateShaderProgram();
            if(ttfConfig.distanceFieldEnabled)
            {
                this->setFontScale(1.0f * ttfConfig.fontSize / DistanceFieldFontSize);
            }
        }

        return true;
    }

    bool Label::setBMFontFilePath(const std::string& bmfontFilePath, const MATH::Vector2f& imageOffset /* = MATH::Vec2fZERO */)
    {
        FontAtlas *newAtlas = FontAtlasCache::getFontAtlasFNT(bmfontFilePath,imageOffset);

        if (!newAtlas)
        {
            reset();
            return false;
        }
        _bmFontPath = bmfontFilePath;
        _currentLabelType = LabelType::BMFONT;
        setFontAtlas(newAtlas);

        return true;
    }

    void Label::setString(const std::string& text)
    {
        if (text.compare(_originalUTF8String))
        {
            _originalUTF8String = text;
            _contentDirty = true;

            std::u16string utf16String;
            UTILS::STRING::UTF8ToUTF16(_originalUTF8String, utf16String);
            _currentUTF16String  = utf16String;
        }
    }

    void Label::setAlignment(TextHAlignment hAlignment,TextVAlignment vAlignment)
    {
        if (hAlignment != _hAlignment || vAlignment != _vAlignment)
        {
            _hAlignment = hAlignment;
            _vAlignment = vAlignment;

            _contentDirty = true;
        }
    }

    void Label::setMaxLineWidth(float maxLineWidth)
    {
        if (_labelWidth == 0 && _maxLineWidth != maxLineWidth)
        {
            _maxLineWidth = maxLineWidth;
            _contentDirty = true;
        }
    }

    void Label::setDimensions(float width, float height)
    {
        if (height != _labelHeight || width != _labelWidth)
        {
            _labelWidth = width;
            _labelHeight = height;
            _labelDimensions.width = width;
            _labelDimensions.height = height;

            _maxLineWidth = width;
            _contentDirty = true;
        }
    }

    void Label::setLineBreakWithoutSpace(bool breakWithoutSpace)
    {
        if (breakWithoutSpace != _lineBreakWithoutSpaces)
        {
            _lineBreakWithoutSpaces = breakWithoutSpace;
            _contentDirty = true;
        }
    }

    void Label::setScale(float scale)
    {
        if (_useDistanceField)
        {
            scale *= _fontScale;
        }
        Node::setScale(scale);
    }

    void Label::setScaleX(float scaleX)
    {
        if (_useDistanceField)
        {
            scaleX *= _fontScale;
        }
        Node::setScaleX(scaleX);
    }

    void Label::setScaleY(float scaleY)
    {
        if (_useDistanceField)
        {
            scaleY *= _fontScale;
        }
        Node::setScaleY(scaleY);
    }

    float Label::getScaleY() const
    {
        if (_useDistanceField)
        {
            return _scaleY / _fontScale;
        }
        else
        {
            return _scaleY;
        }
    }

    float Label::getScaleX() const
    {
        if (_useDistanceField)
        {
            return _scaleX / _fontScale;
        }
        else
        {
            return _scaleX;
        }
    }

    void Label::alignText()
    {
        if (_fontAtlas == nullptr || _currentUTF16String.empty())
        {
            setContentSize(MATH::SizefZERO);
            return;
        }

        _fontAtlas->prepareLetterDefinitions(_currentUTF16String);
        auto& textures = _fontAtlas->getTextures();
        if (textures.size() > _batchNodes.size())
        {
            for (auto index = _batchNodes.size(); index < textures.size(); ++index)
            {
                auto batchNode = SpriteBatchNode::createWithTexture(textures.at(index));
                if (batchNode)
                {
                    _blendFunc = batchNode->getBlendFunc();
                    batchNode->setAnchorPoint(MATH::Vec2fTOPLEFT);
                    batchNode->setPosition(MATH::Vec2fZERO);
                    _batchNodes.pushBack(batchNode);
                }
            }
        }
        if (_batchNodes.empty())
        {
            return;
        }
        _reusedLetter->setBatchNode(_batchNodes.at(0));

        LabelTextFormatter::createStringSprites(this);
        if(_maxLineWidth > 0 && _contentSize.width > _maxLineWidth && LabelTextFormatter::multilineText(this) )
            LabelTextFormatter::createStringSprites(this);

        if(_labelWidth > 0 || (_currNumLines > 1 && _hAlignment != TextHAlignment::LEFT))
            LabelTextFormatter::alignText(this);

        if (!_letters.empty())
        {
            MATH::Rectf uvRect;
            Sprite* letterSprite;
            int letterIndex;

            for (auto it = _letters.begin(); it != _letters.end();)
            {
                letterIndex = it->first;
                letterSprite = it->second;

                if (letterIndex >= _limitShowCount)
                {
                    Node::removeChild(letterSprite, true);
                    it = _letters.erase(it);
                }
                else
                {
                    auto& letterDef = _lettersInfo[letterIndex].def;
                    uvRect.size.height = letterDef.height;
                    uvRect.size.width = letterDef.width;
                    uvRect.origin.x = letterDef.U;
                    uvRect.origin.y = letterDef.V;

                    letterSprite->setBatchNode(_batchNodes.at(letterDef.textureID));
                    letterSprite->setTextureRect(uvRect, false, uvRect.size);
                    letterSprite->setPosition(_lettersInfo[letterIndex].position.x + letterDef.width / 2,
                        _lettersInfo[letterIndex].position.y - letterDef.height / 2);

                    ++it;
                }
            }
        }

        for (const auto& batchNode : _batchNodes)
        {
            batchNode->getTextureAtlas()->removeAllQuads();
        }

        updateQuads();

        updateColor();
    }

    bool Label::computeHorizontalKernings(const std::u16string& stringToRender)
    {
        if (_horizontalKernings)
        {
            delete [] _horizontalKernings;
            _horizontalKernings = nullptr;
        }

        int letterCount = 0;
        _horizontalKernings = _fontAtlas->getFont()->getHorizontalKerningForTextUTF16(stringToRender, letterCount);

        if(!_horizontalKernings)
            return false;
        else
            return true;
    }

    void Label::updateQuads()
    {
        int index;
        for (int ctr = 0; ctr < _limitShowCount; ++ctr)
        {
            auto &letterDef = _lettersInfo[ctr].def;

            if (letterDef.validDefinition)
            {
                _reusedRect.size.height = letterDef.height;
                _reusedRect.size.width  = letterDef.width;
                _reusedRect.origin.x    = letterDef.U;
                _reusedRect.origin.y    = letterDef.V;

                _reusedLetter->setTextureRect(_reusedRect,false,_reusedRect.size);

                _reusedLetter->setPosition(_lettersInfo[ctr].position);
                index = static_cast<int>(_batchNodes.at(letterDef.textureID)->getTextureAtlas()->getTotalQuads());
                _lettersInfo[ctr].atlasIndex = index;
                _batchNodes.at(letterDef.textureID)->insertQuadFromSprite(_reusedLetter,index);
            }
        }
    }

    bool Label::recordLetterInfo(const MATH::Vector2f& point,const FontLetterDefinition& letterDef, int spriteIndex)
    {
        if (static_cast<std::size_t>(spriteIndex) >= _lettersInfo.size())
        {
            LetterInfo tmpInfo;
            _lettersInfo.push_back(tmpInfo);
        }

        _lettersInfo[spriteIndex].def = letterDef;
        _lettersInfo[spriteIndex].position = point;
        _lettersInfo[spriteIndex].contentSize.width = letterDef.width;
        _lettersInfo[spriteIndex].contentSize.height = letterDef.height;
        _limitShowCount++;

        return _lettersInfo[spriteIndex].def.validDefinition;
    }

    bool Label::recordPlaceholderInfo(int spriteIndex)
    {
        if (static_cast<std::size_t>(spriteIndex) >= _lettersInfo.size())
        {
            LetterInfo tmpInfo;
            _lettersInfo.push_back(tmpInfo);
        }

        _lettersInfo[spriteIndex].def.validDefinition = false;
        _limitShowCount++;

        return false;
    }

    void Label::enableGlow(const Color4B& glowColor)
    {
        if (_currentLabelType == LabelType::TTF)
        {
            if (_fontConfig.distanceFieldEnabled == false)
            {
                auto config = _fontConfig;
                config.outlineSize = 0;
                config.distanceFieldEnabled = true;
                setTTFConfig(config);
                _contentDirty = true;
            }
            _currLabelEffect = LabelEffect::GLOW;
            _effectColor = glowColor;
            _effectColorF.red = _effectColor.red / 255.0f;
            _effectColorF.green = _effectColor.green / 255.0f;
            _effectColorF.blue = _effectColor.blue / 255.0f;
            _effectColorF.alpha = _effectColor.alpha / 255.0f;
            updateShaderProgram();
        }
    }

    void Label::enableOutline(const Color4B& outlineColor,int outlineSize /* = -1 */)
    {
        CCASSERT(_currentLabelType == LabelType::STRING_TEXTURE || _currentLabelType == LabelType::TTF, "Only supported system font and TTF!");

        if (outlineSize > 0 || _currLabelEffect == LabelEffect::OUTLINE)
        {
            if (_currentLabelType == LabelType::TTF)
            {
                _effectColorF.red = outlineColor.red / 255.0f;
                _effectColorF.green = outlineColor.green / 255.0f;
                _effectColorF.blue = outlineColor.blue / 255.0f;
                _effectColorF.alpha = outlineColor.alpha / 255.0f;

                if (outlineSize > 0 && _fontConfig.outlineSize != outlineSize)
                {
                    _fontConfig.outlineSize = outlineSize;
                    setTTFConfig(_fontConfig);
                }
            }
            else if (_effectColor != outlineColor || _outlineSize != outlineSize)
            {
                _effectColor = outlineColor;
                _outlineSize = outlineSize;
                _currLabelEffect = LabelEffect::OUTLINE;
                _contentDirty = true;
            }
        }
    }

    void Label::enableShadow(const Color4B& shadowColor /* = Color4B::BLACK */,const MATH::Sizef &offset /* = Size(2 ,-2)*/, int blurRadius /* = 0 */)
    {
        _shadowEnabled = true;
        _shadowDirty = true;

        auto contentScaleFactor = CC_CONTENT_SCALE_FACTOR();
        _shadowOffset.width = offset.width * contentScaleFactor;
        _shadowOffset.height = offset.height * contentScaleFactor;
        //TODO: support blur for shadow
        _shadowBlurRadius = 0;

        _shadowColor3B.red = shadowColor.red;
        _shadowColor3B.green = shadowColor.green;
        _shadowColor3B.blue = shadowColor.blue;
        _shadowOpacity = shadowColor.alpha;

        if (!_systemFontDirty && !_contentDirty && _textSprite)
        {
            if (_shadowNode)
            {
                if (shadowColor != _shadowColor4F)
                {
                    _shadowNode->release();
                    _shadowNode = nullptr;
                    createShadowSpriteForSystemFont();
                }
                else
                {
                    _shadowNode->setPosition(_shadowOffset.width, _shadowOffset.height);
                }
            }
            else
            {
                createShadowSpriteForSystemFont();
            }
        }

        _shadowColor4F.red = shadowColor.red / 255.0f;
        _shadowColor4F.green = shadowColor.green / 255.0f;
        _shadowColor4F.blue = shadowColor.blue / 255.0f;
        _shadowColor4F.alpha = shadowColor.alpha / 255.0f;
    }

    void Label::disableEffect()
    {
        disableEffect(LabelEffect::GLOW);
        disableEffect(LabelEffect::OUTLINE);
        disableEffect(LabelEffect::SHADOW);
    }

    void Label::disableEffect(LabelEffect effect)
    {
        switch (effect)
        {
        case LabelEffect::NORMAL:
            break;
        case LabelEffect::OUTLINE:
            if (_currLabelEffect == LabelEffect::OUTLINE)
            {
                if (_currentLabelType == LabelType::TTF)
                {
                    _fontConfig.outlineSize = 0;
                    setTTFConfig(_fontConfig);
                }

                _currLabelEffect = LabelEffect::NORMAL;
                _contentDirty = true;
            }
            break;
        case LabelEffect::SHADOW:
            if (_shadowEnabled)
            {
                _shadowEnabled = false;
                SAFE_RELEASE_NULL(_shadowNode);
            }
            break;
        case LabelEffect::GLOW:
            if (_currLabelEffect == LabelEffect::GLOW)
            {
                _currLabelEffect = LabelEffect::NORMAL;
                updateShaderProgram();
            }
            break;
        case LabelEffect::ALL:
            {
                disableEffect(LabelEffect::SHADOW);
                disableEffect(LabelEffect::GLOW);
                disableEffect(LabelEffect::OUTLINE);
            }
            break;
        default:
            break;
        }
    }

    void Label::setFontScale(float fontScale)
    {
        _fontScale = fontScale * CC_CONTENT_SCALE_FACTOR();
        Node::setScale(_fontScale);
    }

    void Label::createSpriteForSystemFont()
    {
        _currentLabelType = LabelType::STRING_TEXTURE;

        if (!_compatibleMode)
        {
            _fontDefinition._fontName = _systemFont;
            _fontDefinition._fontSize = _systemFontSize;

            _fontDefinition._alignment = _hAlignment;
            _fontDefinition._vertAlignment = _vAlignment;

            _fontDefinition._dimensions.width = _labelWidth;
            _fontDefinition._dimensions.height = _labelHeight;

            _fontDefinition._fontFillColor.red = _textColor.red;
            _fontDefinition._fontFillColor.green = _textColor.green;
            _fontDefinition._fontFillColor.blue = _textColor.blue;
            _fontDefinition._fontAlpha = _textColor.alpha;

            _fontDefinition._shadow._shadowEnabled = false;

            if (_currLabelEffect == LabelEffect::OUTLINE && _outlineSize > 0)
            {
                _fontDefinition._stroke._strokeEnabled = true;
                _fontDefinition._stroke._strokeSize = _outlineSize;
                _fontDefinition._stroke._strokeColor.r = _effectColor.r;
                _fontDefinition._stroke._strokeColor.g = _effectColor.g;
                _fontDefinition._stroke._strokeColor.b = _effectColor.b;
                _fontDefinition._stroke._strokeAlpha = _effectColor.a;
            }
            else
            {
                _fontDefinition._stroke._strokeEnabled = false;
            }

            _fontDefinition._stroke._strokeEnabled = false;
        }

        auto texture = new (std::nothrow) Texture2D;
        texture->initWithString(_originalUTF8String.c_str(), _fontDefinition);

        _textSprite = Sprite::createWithTexture(texture);
        //set camera mask using label's camera mask, because _textSprite may be null when setting camera mask to label
        _textSprite->setCameraMask(getCameraMask());
        _textSprite->setAnchorPoint(MATH::Vec2fBOTTOMLEFT);
        this->setContentSize(_textSprite->getContentSize());
        texture->release();
        if (_blendFuncDirty)
        {
            _textSprite->setBlendFunc(_blendFunc);
        }

        _textSprite->retain();
        _textSprite->updateDisplayedColor(_displayedColor);
        _textSprite->updateDisplayedOpacity(_displayedOpacity);
    }

    void Label::createShadowSpriteForSystemFont()
    {
        if (!_fontDefinition._stroke._strokeEnabled && _fontDefinition._fontFillColor == _shadowColor3B
            && (_fontDefinition._fontAlpha == _shadowOpacity))
        {
            _shadowNode = Sprite::createWithTexture(_textSprite->getTexture());
        }
        else
        {
            auto shadowFontDefinition = _fontDefinition;
            shadowFontDefinition._fontFillColor.red = _shadowColor3B.red;
            shadowFontDefinition._fontFillColor.green = _shadowColor3B.green;
            shadowFontDefinition._fontFillColor.blue = _shadowColor3B.blue;
            shadowFontDefinition._fontAlpha = _shadowOpacity;

            shadowFontDefinition._stroke._strokeColor = shadowFontDefinition._fontFillColor;
            shadowFontDefinition._stroke._strokeAlpha = shadowFontDefinition._fontAlpha;

            auto texture = new (std::nothrow) Texture2D;
            texture->initWithString(_originalUTF8String.c_str(), shadowFontDefinition);
            _shadowNode = Sprite::createWithTexture(texture);
            texture->release();
        }

        if (_shadowNode)
        {
            if (_blendFuncDirty)
            {
                _shadowNode->setBlendFunc(_blendFunc);
            }
            _shadowNode->setCameraMask(getCameraMask());
            _shadowNode->setAnchorPoint(MATH::Vec2fBOTTOMLEFT);
            _shadowNode->setPosition(_shadowOffset.width, _shadowOffset.height);

            _shadowNode->retain();
            _shadowNode->updateDisplayedColor(_displayedColor);
            _shadowNode->updateDisplayedOpacity(_displayedOpacity);
        }
    }

    void Label::setCameraMask(unsigned short mask, bool applyChildren)
    {
        Node::setCameraMask(mask, applyChildren);

        if (_textSprite)
        {
            _textSprite->setCameraMask(mask, applyChildren);
        }
        if (_shadowNode)
        {
            _shadowNode->setCameraMask(mask, applyChildren);
        }
    }

    void Label::setFontDefinition(const FontDefinition& textDefinition)
    {
        _fontDefinition = textDefinition;
        _fontDefinition._stroke._strokeEnabled = false;
        if (_fontDefinition._shadow._shadowEnabled)
        {
            _fontDefinition._shadow._shadowEnabled = false;
            enableShadow(Color4B(0,0,0,255 * _fontDefinition._shadow._shadowOpacity),_fontDefinition._shadow._shadowOffset,_fontDefinition._shadow._shadowBlur);
        }
        _compatibleMode = true;
    }

    void Label::updateContent()
    {
        if (_systemFontDirty)
        {
            if (_fontAtlas)
            {
                _batchNodes.clear();

                FontAtlasCache::releaseFontAtlas(_fontAtlas);
                _fontAtlas = nullptr;
            }

            _systemFontDirty = false;
        }

        SAFE_RELEASE_NULL(_textSprite);
        SAFE_RELEASE_NULL(_shadowNode);

        if (_fontAtlas)
        {
            std::u16string utf16String;
            if (StringUtils::UTF8ToUTF16(_originalUTF8String, utf16String))
            {
                _currentUTF16String = utf16String;
            }

            computeStringNumLines();
            computeHorizontalKernings(_currentUTF16String);
            alignText();
        }
        else
        {
            createSpriteForSystemFont();
            if (_shadowEnabled)
            {
                createShadowSpriteForSystemFont();
            }
        }
        _contentDirty = false;
    }

    void Label::onDrawShadow(GLProgram* glProgram)
    {
        if (_currentLabelType == LabelType::TTF)
        {
            glProgram->setUniformLocationWith4f(_uniformTextColor,
                _shadowColor4F.r, _shadowColor4F.g, _shadowColor4F.b, _shadowColor4F.a);
            if (_currLabelEffect == LabelEffect::OUTLINE || _currLabelEffect == LabelEffect::GLOW)
            {
                glProgram->setUniformLocationWith4f(_uniformEffectColor,
                    _shadowColor4F.r, _shadowColor4F.g, _shadowColor4F.b, _shadowColor4F.a);
            }

            glProgram->setUniformsForBuiltins(_shadowTransform);
            for (auto it : _letters)
            {
                it.second->updateTransform();
            }
            for (const auto& batchNode : _batchNodes)
            {
                batchNode->getTextureAtlas()->drawQuads();
            }
        }
        else
        {
            Color3B oldColor = _realColor;
            GLubyte oldOPacity = _displayedOpacity;
            _displayedOpacity = _shadowOpacity;
            setColor(_shadowColor3B);

            glProgram->setUniformsForBuiltins(_shadowTransform);
            for (auto it : _letters)
            {
                it.second->updateTransform();
            }
            for (const auto& batchNode : _batchNodes)
            {
                batchNode->getTextureAtlas()->drawQuads();
            }

            _displayedOpacity = oldOPacity;
            setColor(oldColor);
        }
    }

    void Label::onDraw(const Mat4& transform, bool transformUpdated)
    {
        auto glprogram = getGLProgram();
        glprogram->use();
        GL::blendFunc(_blendFunc.src, _blendFunc.dst);

        if (_shadowEnabled)
        {
            onDrawShadow(glprogram);
        }

        glprogram->setUniformsForBuiltins(transform);
        for (auto it : _letters)
        {
            it.second->updateTransform();
        }

        if (_currentLabelType == LabelType::TTF)
        {
            switch (_currLabelEffect) {
            case LabelEffect::OUTLINE:
                //draw text with outline
                glprogram->setUniformLocationWith4f(_uniformTextColor,
                    _textColorF.r, _textColorF.g, _textColorF.b, _textColorF.a);
                glprogram->setUniformLocationWith4f(_uniformEffectColor,
                    _effectColorF.r, _effectColorF.g, _effectColorF.b, _effectColorF.a);
                for (const auto& batchNode : _batchNodes)
                {
                    batchNode->getTextureAtlas()->drawQuads();
                }

                //draw text without outline
                glprogram->setUniformLocationWith4f(_uniformEffectColor,
                    _effectColorF.r, _effectColorF.g, _effectColorF.b, 0.f);
                break;
            case LabelEffect::GLOW:
                glprogram->setUniformLocationWith4f(_uniformEffectColor,
                    _effectColorF.r, _effectColorF.g, _effectColorF.b, _effectColorF.a);
            case LabelEffect::NORMAL:
                glprogram->setUniformLocationWith4f(_uniformTextColor,
                    _textColorF.r, _textColorF.g, _textColorF.b, _textColorF.a);
                break;
            default:
                break;
            }
        }

        for (const auto& batchNode : _batchNodes)
        {
            batchNode->getTextureAtlas()->drawQuads();
        }
    }

    void Label::draw(Renderer *renderer, const Mat4 &transform, uint32_t flags)
    {
        if (_batchNodes.empty() || _limitShowCount <= 0)
        {
            return;
        }
        // Don't do calculate the culling if the transform was not updated
        bool transformUpdated = flags & FLAGS_TRANSFORM_DIRTY;
    #if CC_USE_CULLING
        _insideBounds = transformUpdated ? renderer->checkVisibility(transform, _contentSize) : _insideBounds;

        if (_insideBounds)
    #endif
        {
            _customCommand.init(_globalZOrder, transform, flags);
            _customCommand.func = CC_CALLBACK_0(Label::onDraw, this, transform, transformUpdated);

            renderer->addCommand(&_customCommand);
        }
    }

    void Label::visit(Renderer *renderer, const Mat4 &parentTransform, uint32_t parentFlags)
    {
        if (! _visible || (_originalUTF8String.empty() && _children.empty()) )
        {
            return;
        }

        if (_systemFontDirty || _contentDirty)
        {
            updateContent();
        }

        uint32_t flags = processParentFlags(parentTransform, parentFlags);

        if (!_originalUTF8String.empty() && _shadowEnabled && _shadowBlurRadius <= 0
            && (_shadowDirty || (flags & FLAGS_DIRTY_MASK)))
        {
            _position.x += _shadowOffset.width;
            _position.y += _shadowOffset.height;
            _transformDirty = _inverseDirty = true;

            _shadowTransform = transform(parentTransform);

            _position.x -= _shadowOffset.width;
            _position.y -= _shadowOffset.height;
            _transformDirty = _inverseDirty = true;

            _shadowDirty = false;
        }

        bool visibleByCamera = isVisitableByVisitingCamera();
        if (_children.empty() && !_textSprite && !visibleByCamera)
        {
            return;
        }

        // IMPORTANT:
        // To ease the migration to v3.0, we still support the Mat4 stack,
        // but it is deprecated and your code should not rely on it
        _director->pushMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW);
        _director->loadMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW, _modelViewTransform);

        if (!_children.empty())
        {
            sortAllChildren();

            int i = 0;
            // draw children zOrder < 0
            for (; i < _children.size(); i++)
            {
                auto node = _children.at(i);

                if (node && node->getLocalZOrder() < 0)
                    node->visit(renderer, _modelViewTransform, flags);
                else
                    break;
            }
            // self draw
            if (visibleByCamera)
                this->drawSelf(renderer, flags);

            for (auto it = _children.cbegin() + i; it != _children.cend(); ++it)
            {
                (*it)->visit(renderer, _modelViewTransform, flags);
            }

        }
        else if (visibleByCamera)
        {
            this->drawSelf(renderer, flags);
        }

        _director->popMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW);
    }

    void Label::drawSelf(Renderer* renderer, uint32_t flags)
    {
        if (_textSprite)
        {
            if (_shadowNode)
            {
                _shadowNode->visit(renderer, _modelViewTransform, flags);
            }
            _textSprite->visit(renderer, _modelViewTransform, flags);
        }
        else
        {
            draw(renderer, _modelViewTransform, flags);
        }
    }

    void Label::setSystemFontName(const std::string& systemFont)
    {
        if (systemFont != _systemFont)
        {
            _systemFont = systemFont;
            _systemFontDirty = true;
        }
    }

    void Label::setSystemFontSize(float fontSize)
    {
        if (_systemFontSize != fontSize)
        {
            _systemFontSize = fontSize;
            _systemFontDirty = true;
        }
    }

    ///// PROTOCOL STUFF
    Sprite * Label::getLetter(int letterIndex)
    {
        Sprite* letter = nullptr;
        do
        {
            if (_systemFontDirty || _currentLabelType == LabelType::STRING_TEXTURE)
            {
                break;
            }

            auto contentDirty = _contentDirty;
            if (contentDirty)
            {
                updateContent();
            }

            if (_textSprite == nullptr && letterIndex < _limitShowCount)
            {
                const auto &letterInfo = _lettersInfo[letterIndex];
                if (!letterInfo.def.validDefinition)
                {
                    break;
                }

                if (_letters.find(letterIndex) != _letters.end())
                {
                    letter = _letters[letterIndex];
                }

                auto textureID = letterInfo.def.textureID;
                Rect uvRect;
                uvRect.size.height = letterInfo.def.height;
                uvRect.size.width = letterInfo.def.width;
                uvRect.origin.x = letterInfo.def.U;
                uvRect.origin.y = letterInfo.def.V;

                if (letter == nullptr)
                {
                    letter = LabelLetter::createWithTexture(_fontAtlas->getTexture(textureID), uvRect);
                    letter->setTextureAtlas(_batchNodes.at(textureID)->getTextureAtlas());
                    letter->setAtlasIndex(letterInfo.atlasIndex);

                    letter->setPosition(letterInfo.position.x + uvRect.size.width / 2,
                        letterInfo.position.y - uvRect.size.height / 2);
                    letter->setOpacity(_realOpacity);
                    addChild(letter);

                    _letters[letterIndex] = letter;
                }
                else if (contentDirty)
                {
                    letter->setTexture(_fontAtlas->getTexture(textureID));
                    letter->setTextureRect(uvRect, false, uvRect.size);
                    letter->setTextureAtlas(_batchNodes.at(textureID)->getTextureAtlas());
                }
            }
        } while (false);

        return letter;
    }

    void Label::setLineHeight(float height)
    {
        CCASSERT(_currentLabelType != LabelType::STRING_TEXTURE, "Not supported system font!");

        if (_commonLineHeight != height)
        {
            _commonLineHeight = height;
            _contentDirty = true;
        }
    }

    float Label::getLineHeight() const
    {
        CCASSERT(_currentLabelType != LabelType::STRING_TEXTURE, "Not supported system font!");
        return _textSprite ? 0.0f : _commonLineHeight;
    }

    void Label::setAdditionalKerning(float space)
    {
        CCASSERT(_currentLabelType != LabelType::STRING_TEXTURE, "Not supported system font!");
        if (_additionalKerning != space)
        {
            _additionalKerning = space;
            _contentDirty = true;
        }
    }

    float Label::getAdditionalKerning() const
    {
        CCASSERT(_currentLabelType != LabelType::STRING_TEXTURE, "Not supported system font!");

        return _additionalKerning;
    }

    void Label::computeStringNumLines()
    {
        int quantityOfLines = 1;

        if (_currentUTF16String.empty())
        {
            _currNumLines = 0;
            return;
        }

        // count number of lines
        size_t stringLen = _currentUTF16String.length();
        for (size_t i = 0; i < stringLen-1; ++i)
        {
            if (_currentUTF16String[i] == '\n')
            {
                quantityOfLines++;
            }
        }

        _currNumLines = quantityOfLines;
    }

    int Label::getStringNumLines() const {
        if (_contentDirty)
        {
            const_cast<Label*>(this)->updateContent();
        }

        return _currNumLines;
    }

    int Label::getStringLength() const
    {
        return static_cast<int>(_currentUTF16String.length());
    }

    // RGBA protocol
    bool Label::isOpacityModifyRGB() const
    {
        return _isOpacityModifyRGB;
    }

    void Label::setOpacityModifyRGB(bool isOpacityModifyRGB)
    {
        if (isOpacityModifyRGB != _isOpacityModifyRGB)
        {
            _isOpacityModifyRGB = isOpacityModifyRGB;
            updateColor();
        }
    }

    void Label::updateDisplayedColor(const Color3B& parentColor)
    {
        Node::updateDisplayedColor(parentColor);

        if (_textSprite)
        {
            _textSprite->updateDisplayedColor(_displayedColor);
            if (_shadowNode)
            {
                _shadowNode->updateDisplayedColor(_displayedColor);
            }
        }
    }

    void Label::updateDisplayedOpacity(GLubyte parentOpacity)
    {
        Node::updateDisplayedOpacity(parentOpacity);

        if (_textSprite)
        {
            _textSprite->updateDisplayedOpacity(_displayedOpacity);
            if (_shadowNode)
            {
                _shadowNode->updateDisplayedOpacity(_displayedOpacity);
            }
        }
    }

    void Label::setTextColor(const Color4B &color)
    {
        CCASSERT(_currentLabelType == LabelType::TTF || _currentLabelType == LabelType::STRING_TEXTURE, "Only supported system font and ttf!");

        _textColor = color;
        _textColorF.r = _textColor.r / 255.0f;
        _textColorF.g = _textColor.g / 255.0f;
        _textColorF.b = _textColor.b / 255.0f;
        _textColorF.a = _textColor.a / 255.0f;

        if (_currentLabelType == LabelType::STRING_TEXTURE)
        {
            if (_fontDefinition._fontFillColor.r != _textColor.r || _fontDefinition._fontFillColor.g != _textColor.g
                || _fontDefinition._fontFillColor.b != _textColor.b || _fontDefinition._fontAlpha != _textColor.a)
            {
                _contentDirty = true;
            }
        }
    }

    void Label::updateColor()
    {
        if (_batchNodes.empty())
        {
            return;
        }

        Color4B color4( _displayedColor.r, _displayedColor.g, _displayedColor.b, _displayedOpacity );

        // special opacity for premultiplied textures
        if (_isOpacityModifyRGB)
        {
            color4.r *= _displayedOpacity/255.0f;
            color4.g *= _displayedOpacity/255.0f;
            color4.b *= _displayedOpacity/255.0f;
        }

        cocos2d::TextureAtlas* textureAtlas;
        V3F_C4B_T2F_Quad *quads;
        for (const auto& batchNode:_batchNodes)
        {
            textureAtlas = batchNode->getTextureAtlas();
            quads = textureAtlas->getQuads();
            auto count = textureAtlas->getTotalQuads();

            for (int index = 0; index < count; ++index)
            {
                quads[index].bl.colors = color4;
                quads[index].br.colors = color4;
                quads[index].tl.colors = color4;
                quads[index].tr.colors = color4;
                textureAtlas->updateQuad(&quads[index], index);
            }
        }
    }

    std::string Label::getDescription() const
    {
        std::string utf8str;
        StringUtils::UTF16ToUTF8(_currentUTF16String, utf8str);
        return StringUtils::format("<Label | Tag = %d, Label = '%s'>", _tag, utf8str.c_str());
    }

    const Size& Label::getContentSize() const
    {
        if (_systemFontDirty || _contentDirty)
        {
            const_cast<Label*>(this)->updateContent();
        }
        return _contentSize;
    }

    Rect Label::getBoundingBox() const
    {
        const_cast<Label*>(this)->getContentSize();

        return Node::getBoundingBox();
    }

    void Label::setBlendFunc(const BlendFunc &blendFunc)
    {
        _blendFunc = blendFunc;
        _blendFuncDirty = true;
        if (_textSprite)
        {
            _textSprite->setBlendFunc(blendFunc);
            if (_shadowNode)
            {
                _shadowNode->setBlendFunc(blendFunc);
            }
        }
    }

    void Label::removeAllChildrenWithCleanup(bool cleanup)
    {
        Node::removeAllChildrenWithCleanup(cleanup);
        _letters.clear();
    }

    void Label::removeChild(Node* child, bool cleanup /* = true */)
    {
        Node::removeChild(child, cleanup);
        for (auto it : _letters)
        {
            if (it.second == child)
            {
                _letters.erase(it.first);
                break;
            }
        }
    }
}
