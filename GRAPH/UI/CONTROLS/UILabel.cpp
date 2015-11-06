#include "GRAPH/UI/CONTROLS/UILabel.h"
#include "GRAPH/Director.h"
#include "GRAPH/Sprite.h"
#include "GRAPH/EventDispatcher.h"
#include "GRAPH/UNITY3D/ShaderState.h"
#include "GRAPH/UNITY3D/GLStateCache.h"
#include "GRAPH/UNITY3D/Renderer.h"
#include "GRAPH/UNITY3D/TextureAtlas.h"
#include "IO/FileUtils.h"
#include "UTILS/STRING/UTFUtils.h"

namespace GRAPH
{
    namespace UI
    {
        class LabelLetter : public Sprite
        {
        public:
            LabelLetter()
            {
                textureAtlas_ = nullptr;
            }

            static LabelLetter* createWithTexture(Unity3DTexture *texture, const MATH::Rectf& rect, bool rotated = false)
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

            static LabelLetter* create()
            {
                LabelLetter *pRet = new(std::nothrow) LabelLetter();
                if (pRet && pRet->init())
                {
                    pRet->autorelease();
                    return pRet;
                }
                else
                {
                    delete pRet;
                    pRet = NULL;
                    return NULL;
                }
            }

            virtual void updateTransform() override
            {
                if (isDirty())
                {
                    transformToBatch_ = getNodeToParentTransform();
                    MATH::Sizef &size = rect_.size;

                    float x1 = offsetPosition_.x;
                    float y1 = offsetPosition_.y;
                    float x2 = x1 + size.width;
                    float y2 = y1 + size.height;
                    if (flippedX_)
                    {
                        std::swap(x1, x2);
                    }
                    if (flippedY_)
                    {
                        std::swap(y1, y2);
                    }

                    float x = transformToBatch_.m[12];
                    float y = transformToBatch_.m[13];

                    float cr = transformToBatch_.m[0];
                    float sr = transformToBatch_.m[1];
                    float cr2 = transformToBatch_.m[5];
                    float sr2 = -transformToBatch_.m[4];
                    float ax = x1 * cr - y1 * sr2 + x;
                    float ay = x1 * sr + y1 * cr2 + y;

                    float bx = x2 * cr - y1 * sr2 + x;
                    float by = x2 * sr + y1 * cr2 + y;
                    float cx = x2 * cr - y2 * sr2 + x;
                    float cy = x2 * sr + y2 * cr2 + y;
                    float dx = x1 * cr - y2 * sr2 + x;
                    float dy = x1 * sr + y2 * cr2 + y;

                    quad_.bl.vertices.set(ax, ay, positionZ_);
                    quad_.br.vertices.set(bx, by, positionZ_);
                    quad_.tl.vertices.set(dx, dy, positionZ_);
                    quad_.tr.vertices.set(cx, cy, positionZ_);

                    if (textureAtlas_)
                    {
                        textureAtlas_->updateQuad(&quad_, atlasIndex_);
                    }

                    recursiveDirty_ = false;
                    setDirty(false);
                }

                Node::updateTransform();
            }

            virtual void updateColor()
            {
                if (textureAtlas_ == nullptr)
                {
                    return;
                }

                Color4B color4(displayedColor_.red, displayedColor_.green, displayedColor_.blue, displayedOpacity_);
                // special opacity for premultiplied textures
                if (opacityModifyRGB_)
                {
                    color4.red *= displayedOpacity_ / 255.0f;
                    color4.green *= displayedOpacity_ / 255.0f;
                    color4.blue *= displayedOpacity_ / 255.0f;
                }
                quad_.bl.colors = color4;
                quad_.br.colors = color4;
                quad_.tl.colors = color4;
                quad_.tr.colors = color4;

                textureAtlas_->updateQuad(&quad_, atlasIndex_);
            }

            //LabelLetter doesn't need to draw directly.
            void draw(Renderer *, const MATH::Matrix4 &, uint32_t)
            {
            }
        };

        Label* Label::create() {
            auto ret = new (std::nothrow) Label();
            if (ret) {
                ret->autorelease();
            }

            return ret;
        }

        Label* Label::createWithCustomLoader(const char *string, U3DStringToTexture loader, void *loaderOwner) {
            auto ret = new (std::nothrow) Label();
            if (ret) {
                ret->setString(string);
                ret->stringToTextureLoader_ = loader;
                ret->stringtoTextureOwner_ = loaderOwner;
                ret->autorelease();
            }
            return ret;
        }

        Label::Label()
            : _textSprite(nullptr)
            , _shadowNode(nullptr)
            , _fontAtlas(nullptr)
            , _reusedLetter(nullptr)
            , _horizontalKernings(nullptr)
        {
            setAnchorPoint(MATH::Vec2fMIDDLE);
            reset();
        }

        Label::~Label()
        {
            delete [] _horizontalKernings;

            if (_fontAtlas)
            {
                Node::removeAllChildrenWithCleanup(true);
                SAFE_RELEASE_NULL(_reusedLetter);
                _batchNodes.clear();
                FontAtlasCache::releaseFontAtlas(_fontAtlas);
            }

            SAFE_RELEASE_NULL(_textSprite);
            SAFE_RELEASE_NULL(_shadowNode);
        }

        void Label::reset()
        {
            SAFE_RELEASE_NULL(_textSprite);
            SAFE_RELEASE_NULL(_shadowNode);
            Node::removeAllChildrenWithCleanup(true);
            SAFE_RELEASE_NULL(_reusedLetter);
            _letters.clear();
            _batchNodes.clear();
            _lettersInfo.clear();
            if (_fontAtlas)
            {
                FontAtlasCache::releaseFontAtlas(_fontAtlas);
                _fontAtlas = nullptr;
            }

            _currLabelEffect = LabelEffect::NORMAL;
            _contentDirty = false;
            _numberOfLines = 0;
            _lengthOfString = 0;
            _utf16Text.clear();
            _utf8Text.clear();

            _outlineSize = 0.f;
            _bmFontPath = "";
            _systemFontDirty = false;
            _systemFont = "Helvetica";
            _systemFontSize = 12;

            if (_horizontalKernings)
            {
                delete[] _horizontalKernings;
                _horizontalKernings = nullptr;
            }
            _additionalKerning = 0.f;
            _lineHeight = 0.f;
            _maxLineWidth = 0.f;
            _labelDimensions.width = 0.f;
            _labelDimensions.height = 0.f;
            _labelWidth = 0.f;
            _labelHeight = 0.f;
            _lineBreakWithoutSpaces = false;

            _effectColorF = Color4F::BLACK;
            _textColor = Color4B::WHITE;
            _textColorF = Color4F::WHITE;
            setColor(Color3B::WHITE);

            _shadowDirty = false;
            _shadowEnabled = false;
            _shadowBlurRadius = 0.f;

            _useDistanceField = false;
            _useA8Shader = false;
            _clipEnabled = false;
            _blendFuncDirty = false;
            _blendFunc = BlendFunc::ALPHA_PREMULTIPLIED;
            _isOpacityModifyRGB = false;
            _insideBounds = true;
        }

        void Label::updateShaderProgram()
        {
            switch (_currLabelEffect)
            {
            case LabelEffect::NORMAL:
                if (_useDistanceField)
                    setU3DShaderState(ShaderState::getOrCreateWithShaderName(Unity3DShader::SHADER_NAME_LABEL_DISTANCEFIELD_NORMAL));
                else if (_useA8Shader)
                    setU3DShaderState(ShaderState::getOrCreateWithShaderName(Unity3DShader::SHADER_NAME_LABEL_NORMAL));
                else if (_shadowEnabled)
                    setU3DShaderState(ShaderState::getOrCreateWithShaderName(Unity3DShader::SHADER_NAME_POSITION_TEXTURE_COLOR));
                else
                    setU3DShaderState(ShaderState::getOrCreateWithShaderName(Unity3DShader::SHADER_NAME_POSITION_TEXTURE_COLOR_NO_MVP));

                break;
            case LabelEffect::OUTLINE:
                setU3DShaderState(ShaderState::getOrCreateWithShaderName(Unity3DShader::SHADER_NAME_LABEL_OUTLINE));
                _uniformEffectColor = getU3DShader()->getUniformLocation("u_effectColor");
                break;
            case LabelEffect::GLOW:
                if (_useDistanceField)
                {
                    setU3DShaderState(ShaderState::getOrCreateWithShaderName(Unity3DShader::SHADER_NAME_LABEL_DISTANCEFIELD_GLOW));
                    _uniformEffectColor = getU3DShader()->getUniformLocation("u_effectColor");
                }
                break;
            default:
                return;
            }

            _uniformTextColor = getU3DShader()->getUniformLocation("u_textColor");
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
                _lineHeight = _fontAtlas->getCommonLineHeight();
                _contentDirty = true;
            }
            _useDistanceField = distanceFieldEnabled;
            _useA8Shader = useA8Shader;

            _currLabelEffect = LabelEffect::NORMAL;
            updateShaderProgram();
        }

        void Label::setString(const std::string& text)
        {
            if (text.compare(_utf8Text))
            {
                _utf8Text = text;
                _contentDirty = true;

                std::u16string utf16String;
                UTILS::STRING::UTF8ToUTF16(_utf8Text, utf16String);
                _utf16Text  = utf16String;
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

        bool Label::multilineTextWrapByChar() {
            int textLen = getStringLength();
            int lineIndex = 0;
            float nextLetterX = 0.f;
            float nextLetterY = 0.f;
            float longestLine = 0.f;
            float letterRight = 0.f;

            float highestY = 0.f;
            float lowestY = 0.f;
            FontLetterDefinition letterDef;
            MATH::Vector2f letterPosition;

            for (int index = 0; index < textLen; index++) {
                auto character = _utf16Text[index];
                if (character == '\r') {
                    recordPlaceholderInfo(index, character);
                    continue;
                }
                if (character == '\n') {
                    _linesWidth.push_back(letterRight);
                    letterRight = 0.f;
                    lineIndex++;
                    nextLetterX = 0.f;
                    nextLetterY -= _lineHeight;
                    recordPlaceholderInfo(index, character);
                    continue;
                }

                if (_fontAtlas->getLetterDefinitionForChar(character, letterDef) == false)
                {
                    recordPlaceholderInfo(index, character);
                    continue;
                }

                auto letterX = nextLetterX;
                if (_maxLineWidth > 0.f && nextLetterX > 0.f && letterX + letterDef.width > _maxLineWidth)
                {
                    _linesWidth.push_back(letterRight);
                    letterRight = 0.f;
                    lineIndex++;
                    nextLetterX = 0.f;
                    nextLetterY -= _lineHeight;
                }
                else
                {
                    letterPosition.x = letterX;
                }
                letterPosition.y = nextLetterY;
                recordLetterInfo(letterPosition, character, index, lineIndex);

                if (_horizontalKernings && index < textLen - 1)
                    nextLetterX += _horizontalKernings[index + 1];
                nextLetterX += _additionalKerning;

                letterRight = letterPosition.x + letterDef.width;

                if (highestY < letterPosition.y)
                    highestY = letterPosition.y;
                if (lowestY > letterPosition.y - letterDef.height)
                    lowestY = letterPosition.y - letterDef.height;
                if (longestLine < letterRight)
                    longestLine = letterRight;
            }

            _linesWidth.push_back(letterRight);

            _numberOfLines = lineIndex + 1;
            _textDesiredHeight = (_numberOfLines * _lineHeight);
            MATH::Sizef contentSize(_labelWidth, _labelHeight);
            if (_labelWidth <= 0.f)
                contentSize.width = longestLine;
            if (_labelHeight <= 0.f)
                contentSize.height = _textDesiredHeight;
            setContentSize(contentSize);

            _tailoredTopY = contentSize.height;
            _tailoredBottomY = 0.f;
            if (highestY > 0.f)
                _tailoredTopY = contentSize.height + highestY;
            if (lowestY < -_textDesiredHeight)
                _tailoredBottomY = _textDesiredHeight + lowestY;

            return true;
        }

        bool Label::multilineTextWrapByWord()
        {
            int textLen = getStringLength();
            int lineIndex = 0;
            float nextWordX = 0.f;
            float nextWordY = 0.f;
            float longestLine = 0.f;
            float letterRight = 0.f;

            float highestY = 0.f;
            float lowestY = 0.f;
            FontLetterDefinition letterDef;
            MATH::Vector2f letterPosition;

            for (int index = 0; index < textLen;)
            {
                auto character = _utf16Text[index];
                if (character == '\n')
                {
                    _linesWidth.push_back(letterRight);
                    letterRight = 0.f;
                    lineIndex++;
                    nextWordX = 0.f;
                    nextWordY -= _lineHeight;
                    recordPlaceholderInfo(index, character);
                    index++;
                    continue;
                }

                auto wordLen = UTILS::STRING::GetWordLen(_utf16Text, index, textLen);
                float wordHighestY = highestY;;
                float wordLowestY = lowestY;
                float wordRight = letterRight;
                float nextLetterX = nextWordX;
                bool newLine = false;
                for (int tmp = 0; tmp < wordLen; ++tmp)
                {
                    int letterIndex = index + tmp;
                    character = _utf16Text[letterIndex];
                    if (character == '\r')
                    {
                        recordPlaceholderInfo(letterIndex, character);
                        continue;
                    }
                    if (_fontAtlas->getLetterDefinitionForChar(character, letterDef) == false)
                    {
                        recordPlaceholderInfo(letterIndex, character);
                        continue;
                    }

                    auto letterX = nextLetterX;
                    if (_maxLineWidth > 0.f && nextWordX > 0.f && letterX + letterDef.width > _maxLineWidth)
                    {
                        _linesWidth.push_back(letterRight);
                        letterRight = 0.f;
                        lineIndex++;
                        nextWordX = 0.f;
                        nextWordY -= _lineHeight;
                        newLine = true;
                        break;
                    }
                    else
                    {
                        letterPosition.x = letterX;
                    }
                    letterPosition.y = nextWordY;
                    recordLetterInfo(letterPosition, character, letterIndex, lineIndex);

                    if (_horizontalKernings && letterIndex < textLen - 1)
                        nextLetterX += _horizontalKernings[letterIndex + 1];
                    nextLetterX += _additionalKerning;

                    wordRight = letterPosition.x + letterDef.width;

                    if (wordHighestY < letterPosition.y)
                        wordHighestY = letterPosition.y;
                    if (wordLowestY > letterPosition.y - letterDef.height)
                        wordLowestY = letterPosition.y - letterDef.height;
                }

                if (newLine)
                {
                    continue;
                }

                nextWordX = nextLetterX;
                letterRight = wordRight;
                if (highestY < wordHighestY)
                    highestY = wordHighestY;
                if (lowestY > wordLowestY)
                    lowestY = wordLowestY;
                if (longestLine < letterRight)
                    longestLine = letterRight;

                index += wordLen;
            }

            _linesWidth.push_back(letterRight);

            _numberOfLines = lineIndex + 1;
            _textDesiredHeight = _numberOfLines * _lineHeight;
            MATH::Sizef contentSize(_labelWidth, _labelHeight);
            if (_labelWidth <= 0.f)
                contentSize.width = longestLine;
            if (_labelHeight <= 0.f)
                contentSize.height = _textDesiredHeight;
            setContentSize(contentSize);

            _tailoredTopY = contentSize.height;
            _tailoredBottomY = 0.f;
            if (highestY > 0.f)
                _tailoredTopY = contentSize.height + highestY;
            if (lowestY < -_textDesiredHeight)
                _tailoredBottomY = _textDesiredHeight + lowestY;

            return true;
        }

        void Label::updateLabelLetters()
        {
            if (!_letters.empty())
            {
                MATH::Rectf uvRect;
                LabelLetter* letterSprite;
                int letterIndex;

                for (auto it = _letters.begin(); it != _letters.end();)
                {
                    letterIndex = it->first;
                    letterSprite = (LabelLetter*)it->second;

                    if (letterIndex >= _lengthOfString)
                    {
                        Node::removeChild(letterSprite, true);
                        it = _letters.erase(it);
                    }
                    else
                    {
                        auto& letterInfo = _lettersInfo[letterIndex];
                        auto& letterDef = _fontAtlas->letterDefinitions_[letterInfo.utf16Char];
                        uvRect.size.height = letterDef.height;
                        uvRect.size.width = letterDef.width;
                        uvRect.origin.x = letterDef.U;
                        uvRect.origin.y = letterDef.V;

                        letterSprite->setTexture(_fontAtlas->getTexture(letterDef.textureID));
                        if (letterDef.width <= 0.f || letterDef.height <= 0.f)
                        {
                            letterSprite->setTextureAtlas(nullptr);
                        }
                        else
                        {
                            letterSprite->setTextureRect(uvRect, false, uvRect.size);
                            letterSprite->setTextureAtlas(_batchNodes.at(letterDef.textureID)->getTextureAtlas());
                            letterSprite->setAtlasIndex(_lettersInfo[letterIndex].atlasIndex);
                        }

                        auto px = letterInfo.positionX + letterDef.width / 2 + _linesOffsetX[letterInfo.lineIndex];
                        auto py = letterInfo.positionY - letterDef.height / 2 + _letterOffsetY;
                        letterSprite->setPosition(px, py);

                        ++it;
                    }
                }
            }
        }

        void Label::alignText()
        {
            if (_fontAtlas == nullptr || _utf16Text.empty())
            {
                setContentSize(MATH::SizefZERO);
                return;
            }

            _fontAtlas->prepareLetterDefinitions(_utf16Text);
            auto& textures = _fontAtlas->getTextures();
            if (textures.size() > _batchNodes.size())
            {
                for (auto index = _batchNodes.size(); index < textures.size(); ++index)
                {
                    auto batchNode = SpriteBatchNode::createWithTexture(textures.at(index));
                    if (batchNode)
                    {
                        _isOpacityModifyRGB = batchNode->getTexture()->hasPremultipliedAlpha();
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

            _lengthOfString = 0;
            _textDesiredHeight = 0.f;
            _linesWidth.clear();
            if (_maxLineWidth > 0.f && !_lineBreakWithoutSpaces)
            {
                multilineTextWrapByWord();
            }
            else
            {
                multilineTextWrapByChar();
            }

            updateQuads();

            updateLabelLetters();

            updateColor();
        }

        void Label::recordLetterInfo(const MATH::Vector2f& point, char16_t utf16Char, int letterIndex, int lineIndex)
        {
            if (static_cast<uint64>(letterIndex) >= _lettersInfo.size())
            {
                LetterInfo tmpInfo;
                _lettersInfo.push_back(tmpInfo);
            }
            _lettersInfo[letterIndex].lineIndex = lineIndex;
            _lettersInfo[letterIndex].utf16Char = utf16Char;
            _lettersInfo[letterIndex].valid = true;
            _lettersInfo[letterIndex].positionX = point.x;
            _lettersInfo[letterIndex].positionY = point.y;
        }

        void Label::recordPlaceholderInfo(int letterIndex, char16_t utf16Char)
        {
            if (static_cast<uint64>(letterIndex) >= _lettersInfo.size())
            {
                LetterInfo tmpInfo;
                _lettersInfo.push_back(tmpInfo);
            }
            _lettersInfo[letterIndex].utf16Char = utf16Char;
            _lettersInfo[letterIndex].valid = false;
        }

        void Label::updateQuads()
        {
            for (auto&& batchNode : _batchNodes)
            {
                batchNode->getTextureAtlas()->removeAllQuads();
            }

            for (int ctr = 0; ctr < _lengthOfString; ++ctr)
            {
                if (_lettersInfo[ctr].valid)
                {
                    auto& letterDef = _fontAtlas->letterDefinitions_[_lettersInfo[ctr].utf16Char];

                    _reusedRect.size.height = letterDef.height;
                    _reusedRect.size.width  = letterDef.width;
                    _reusedRect.origin.x    = letterDef.U;
                    _reusedRect.origin.y    = letterDef.V;

                    auto py = _lettersInfo[ctr].positionY + _letterOffsetY;
                    if (_labelHeight > 0.f) {
                        if (py > _tailoredTopY)
                        {
                            auto clipTop = py - _tailoredTopY;
                            _reusedRect.origin.y += clipTop;
                            _reusedRect.size.height -= clipTop;
                            py -= clipTop;
                        }
                        if (py - letterDef.height < _tailoredBottomY)
                        {
                            _reusedRect.size.height = (py < _tailoredBottomY) ? 0.f : (py - _tailoredBottomY);
                        }
                    }

                    if (_reusedRect.size.height > 0.f && _reusedRect.size.width > 0.f)
                    {
                        _reusedLetter->setTextureRect(_reusedRect, false, _reusedRect.size);
                        _reusedLetter->setPosition(_lettersInfo[ctr].positionX + _linesOffsetX[_lettersInfo[ctr].lineIndex], py);
                        auto index = static_cast<int>(_batchNodes.at(letterDef.textureID)->getTextureAtlas()->getTotalQuads());
                        _lettersInfo[ctr].atlasIndex = index;
                        _batchNodes.at(letterDef.textureID)->insertQuadFromSprite(_reusedLetter, index);
                    }
                }
            }
        }

        void Label::enableOutline(const Color4B& outlineColor,int outlineSize /* = -1 */)
        {
            if (outlineSize > 0 || _currLabelEffect == LabelEffect::OUTLINE)
            {
               if (_effectColorF != outlineColor || _outlineSize != outlineSize)
                {
                    _effectColorF.red = outlineColor.red / 255.f;
                    _effectColorF.green = outlineColor.green / 255.f;
                    _effectColorF.blue = outlineColor.blue / 255.f;
                    _effectColorF.alpha = outlineColor.alpha / 255.f;
                    _outlineSize = outlineSize;
                    _currLabelEffect = LabelEffect::OUTLINE;
                    _contentDirty = true;
                }
            }
        }

        void Label::enableShadow(const Color4B& shadowColor /* = Color4B::BLACK */,const MATH::Sizef &offset /* = Size(2 ,-2)*/)
        {
            _shadowEnabled = true;
            _shadowDirty = true;

            _shadowOffset.width = offset.width;
            _shadowOffset.height = offset.height;
            //TODO: support blur for shadow

            _shadowColor3B.red = shadowColor.red;
            _shadowColor3B.green = shadowColor.green;
            _shadowColor3B.blue = shadowColor.blue;
            _shadowOpacity = shadowColor.alpha;

            if (!_systemFontDirty && !_contentDirty && _textSprite)
            {
                // TODO
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

        void Label::updateContent() {
            if (_systemFontDirty) {
                if (_fontAtlas) {
                    _batchNodes.clear();
                    FontAtlasCache::releaseFontAtlas(_fontAtlas);
                    _fontAtlas = nullptr;
                }

                _systemFontDirty = false;
            }

            SAFE_RELEASE_NULL(_textSprite);
            SAFE_RELEASE_NULL(_shadowNode);

            if (_fontAtlas) {
                std::u16string utf16String;
                UTILS::STRING::UTF8ToUTF16(_utf8Text, utf16String);
                _utf16Text = utf16String;
                alignText();
            }
            else {
                auto texture = Unity3DCreator::CreateTexture();
                texture->initWithString(_utf8Text.c_str(), stringToTextureLoader_, stringtoTextureOwner_);
                _textSprite = Sprite::createWithTexture(texture);
                //set camera mask using label's camera mask, because _textSprite may be null when setting camera mask to label
                _textSprite->setCameraMask(getCameraMask());
                _textSprite->setGlobalZOrder(getGlobalZOrder());
                _textSprite->setAnchorPoint(MATH::Vec2fBOTTOMLEFT);
                this->setContentSize(_textSprite->getContentSize());
                texture->release();
                if (_blendFuncDirty) {
                    _textSprite->setBlendFunc(_blendFunc);
                }

                _textSprite->retain();
                _textSprite->updateDisplayedColor(displayedColor_);
                _textSprite->updateDisplayedOpacity(displayedOpacity_);
            }
            _contentDirty = false;
        }

        void Label::onDrawShadow(Unity3DShaderSet* u3dShader)
        {
            Color3B oldColor = realColor_;
            GLubyte oldOPacity = displayedOpacity_;
            displayedOpacity_ = _shadowOpacity;
            setColor(_shadowColor3B);

            u3dShader->setUniformsForBuiltins(_shadowTransform);
            for (auto&& it : _letters)
            {
                it.second->updateTransform();
            }
            for (auto&& batchNode : _batchNodes)
            {
                batchNode->getTextureAtlas()->drawQuads();
            }

            displayedOpacity_ = oldOPacity;
            setColor(oldColor);
        }

        void Label::onDraw(const MATH::Matrix4& transform, bool)
        {
            auto u3dShader = getU3DShader();
            u3dShader->apply();
            GLStateCache::BlendFunc(_blendFunc.src, _blendFunc.dst);

            if (_shadowEnabled)
            {
                onDrawShadow(u3dShader);
            }

            u3dShader->setUniformsForBuiltins(transform);
            for (auto&& it : _letters)
            {
                it.second->updateTransform();
            }

            for (auto&& batchNode : _batchNodes)
            {
                batchNode->getTextureAtlas()->drawQuads();
            }
        }

        void Label::draw(Renderer *renderer, const MATH::Matrix4 &transform, uint32_t flags)
        {
            if (_batchNodes.empty() || _lengthOfString <= 0)
            {
                return;
            }
            // Don't do calculate the culling if the transform was not updated
            bool transformUpdated = flags & FLAGS_TRANSFORM_DIRTY;
                _insideBounds = transformUpdated ? Director::getInstance().checkVisibility(transform, contentSize_) : _insideBounds;

            if (_insideBounds)
            {
                _customCommand.init(globalZOrder_, transform, flags);
                _customCommand.func = std::bind(&Label::onDraw, this, transform, transformUpdated);

                renderer->addCommand(&_customCommand);
            }
        }

        void Label::visit(Renderer *renderer, const MATH::Matrix4 &parentTransform, uint32_t parentFlags)
        {
            if (! visible_ || (_utf8Text.empty() && children_.empty()) )
            {
                return;
            }

            if (_systemFontDirty || _contentDirty)
            {
                updateContent();
            }

            uint32_t flags = processParentFlags(parentTransform, parentFlags);

            if (!_utf8Text.empty() && _shadowEnabled && (_shadowDirty || (flags & FLAGS_DIRTY_MASK)))
            {
                position_.x += _shadowOffset.width;
                position_.y += _shadowOffset.height;
                transformDirty_ = inverseDirty_ = true;

                _shadowTransform = transform(parentTransform);

                position_.x -= _shadowOffset.width;
                position_.y -= _shadowOffset.height;
                transformDirty_ = inverseDirty_ = true;

                _shadowDirty = false;
            }

            if (children_.empty() && !_textSprite)
            {
                return;
            }

            // IMPORTANT:
            // To ease the migration to v3.0, we still support the MATH::Matrix4 stack,
            // but it is deprecated and your code should not rely on it
            director_->pushMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW);
            director_->loadMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW, modelViewTransform_);

            if (!children_.empty())
            {
                sortAllChildren();

                int i = 0;
                // draw children zOrder < 0
                for (; i < children_.size(); i++)
                {
                    auto node = children_.at(i);

                    if (node && node->getLocalZOrder() < 0)
                        node->visit(renderer, modelViewTransform_, flags);
                    else
                        break;
                }

                this->drawSelf(renderer, flags);

                for (auto it = children_.cbegin() + i; it != children_.cend(); ++it)
                {
                    (*it)->visit(renderer, modelViewTransform_, flags);
                }

            }
            else
            {
                this->drawSelf(renderer, flags);
            }

            director_->popMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW);
        }

        void Label::drawSelf(Renderer* renderer, uint32_t flags)
        {
            if (_textSprite)
            {
                if (_shadowNode)
                {
                    _shadowNode->visit(renderer, modelViewTransform_, flags);
                }
                _textSprite->visit(renderer, modelViewTransform_, flags);
            }
            else if (!_utf8Text.empty())
            {
                draw(renderer, modelViewTransform_, flags);
            }
        }

        Sprite* Label::getLetter(int letterIndex)
        {
            Sprite* letter = nullptr;
            do
            {
                if (_systemFontDirty)
                {
                    break;
                }

                auto contentDirty = _contentDirty;
                if (contentDirty)
                {
                    updateContent();
                }

                if (_textSprite == nullptr && letterIndex < _lengthOfString)
                {
                    const auto &letterInfo = _lettersInfo[letterIndex];
                    if (!letterInfo.valid)
                    {
                        break;
                    }

                    if (_letters.find(letterIndex) != _letters.end())
                    {
                        letter = _letters[letterIndex];
                    }

                    if (letter == nullptr)
                    {
                        auto& letterDef = _fontAtlas->letterDefinitions_[letterInfo.utf16Char];
                        auto textureID = letterDef.textureID;
                        MATH::Rectf uvRect;
                        uvRect.size.height = letterDef.height;
                        uvRect.size.width = letterDef.width;
                        uvRect.origin.x = letterDef.U;
                        uvRect.origin.y = letterDef.V;

                        if (letterDef.width <= 0.f || letterDef.height <= 0.f)
                        {
                            letter = LabelLetter::create();
                        }
                        else
                        {
                            letter = LabelLetter::createWithTexture(_fontAtlas->getTexture(textureID), uvRect);
                            letter->setTextureAtlas(_batchNodes.at(textureID)->getTextureAtlas());
                            letter->setAtlasIndex(letterInfo.atlasIndex);
                            auto px = letterInfo.positionX + uvRect.size.width / 2 + _linesOffsetX[letterInfo.lineIndex];
                            auto py = letterInfo.positionY - uvRect.size.height / 2 + _letterOffsetY;
                            letter->setPosition(px,py);
                            letter->setOpacity(realOpacity_);
                        }

                        addChild(letter);
                        _letters[letterIndex] = letter;
                    }
                }
            } while (false);

            return letter;
        }

        void Label::setLineHeight(float height)
        {
            if (_lineHeight != height)
            {
                _lineHeight = height;
                _contentDirty = true;
            }
        }

        float Label::getLineHeight() const
        {
            return _textSprite ? 0.0f : _lineHeight;
        }

        void Label::setAdditionalKerning(float space)
        {
            if (_additionalKerning != space)
            {
                _additionalKerning = space;
                _contentDirty = true;
            }
        }

        float Label::getAdditionalKerning() const
        {
            return _additionalKerning;
        }

        void Label::computeStringNumLines()
        {
            int quantityOfLines = 1;

            if (_utf16Text.empty())
            {
                _numberOfLines = 0;
                return;
            }

            // count number of lines
            uint64 stringLen = _utf16Text.length();
            for (uint64 i = 0; i < stringLen - 1; ++i)
            {
                if (_utf16Text[i] == '\n')
                {
                    quantityOfLines++;
                }
            }

            _numberOfLines = quantityOfLines;
        }

        int Label::getStringNumLines()
        {
            if (_contentDirty)
            {
                updateContent();
            }

            return _numberOfLines;
        }

        int Label::getStringLength()
        {
            _lengthOfString = static_cast<int>(_utf16Text.length());
            return _lengthOfString;
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
                _textSprite->updateDisplayedColor(displayedColor_);
                if (_shadowNode)
                {
                    _shadowNode->updateDisplayedColor(displayedColor_);
                }
            }

            for (auto&& it : _letters)
            {
                it.second->updateDisplayedColor(displayedColor_);;
            }
        }

        void Label::updateDisplayedOpacity(GLubyte parentOpacity)
        {
            Node::updateDisplayedOpacity(parentOpacity);

            if (_textSprite)
            {
                _textSprite->updateDisplayedOpacity(displayedOpacity_);
                if (_shadowNode)
                {
                    _shadowNode->updateDisplayedOpacity(displayedOpacity_);
                }
            }

            for (auto&& it : _letters)
            {
                it.second->updateDisplayedOpacity(displayedOpacity_);;
            }
        }

        void Label::setTextColor(const Color4B &color)
        {
            _textColor = color;
            _textColorF.red = _textColor.red / 255.0f;
            _textColorF.green = _textColor.green / 255.0f;
            _textColorF.blue = _textColor.blue / 255.0f;
            _textColorF.alpha = _textColor.alpha / 255.0f;
        }

        void Label::updateColor()
        {
            if (_batchNodes.empty())
            {
                return;
            }

            Color4B color4( displayedColor_.red, displayedColor_.green, displayedColor_.blue, displayedOpacity_ );

            // special opacity for premultiplied textures
            if (_isOpacityModifyRGB)
            {
                color4.red *= displayedOpacity_/255.0f;
                color4.green *= displayedOpacity_/255.0f;
                color4.blue *= displayedOpacity_/255.0f;
            }

            TextureAtlas* textureAtlas;
            V3F_C4B_T2F_Quad *quads;
            for (auto&& batchNode:_batchNodes)
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

        const MATH::Sizef& Label::getContentSize() const
        {
            if (_systemFontDirty || _contentDirty)
            {
                const_cast<Label*>(this)->updateContent();
            }
            return contentSize_;
        }

        MATH::Rectf Label::getBoundingBox() const
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
            for (auto&& it : _letters)
            {
                if (it.second == child)
                {
                    _letters.erase(it.first);
                    break;
                }
            }
        }

        void Label::setGlobalZOrder(float globalZOrder)
        {
            Node::setGlobalZOrder(globalZOrder);
            if (_textSprite)
            {
                _textSprite->setGlobalZOrder(globalZOrder);
                if (_shadowNode)
                {
                    _shadowNode->setGlobalZOrder(globalZOrder);
                }
            }
        }
    }
}
