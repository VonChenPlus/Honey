#include "GRAPH/UI/CONTROLS/UILabel.h"
#include "GRAPH/Director.h"
#include "GRAPH/Sprite.h"
#include "GRAPH/EventDispatcher.h"
#include "GRAPH/UNITY3D/ShaderState.h"
#include "GRAPH/UNITY3D/Unity3DGLState.h"
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

            static LabelLetter* create() {
                LabelLetter *pRet = new(std::nothrow) LabelLetter();
                if (pRet && pRet->init()) {
                    pRet->autorelease();
                    return pRet;
                }
                else {
                    delete pRet;
                    return nullptr;
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
            : textSprite_(nullptr)
            , shadowNode_(nullptr)
            , fontAtlas_(nullptr)
            , reusedLetter_(nullptr)
            , horizontalKernings_(nullptr)
        {
            setAnchorPoint(MATH::Vec2fMIDDLE);
            reset();
        }

        Label::~Label()
        {
            delete [] horizontalKernings_;

            if (fontAtlas_)
            {
                Node::removeAllChildrenWithCleanup(true);
                SAFE_RELEASE_NULL(reusedLetter_);
                batchNodes_.clear();
                FontAtlasCache::releaseFontAtlas(fontAtlas_);
            }

            SAFE_RELEASE_NULL(textSprite_);
            SAFE_RELEASE_NULL(shadowNode_);
        }

        void Label::reset()
        {
            SAFE_RELEASE_NULL(textSprite_);
            SAFE_RELEASE_NULL(shadowNode_);
            Node::removeAllChildrenWithCleanup(true);
            SAFE_RELEASE_NULL(reusedLetter_);
            letters_.clear();
            batchNodes_.clear();
            lettersInfo_.clear();
            if (fontAtlas_)
            {
                FontAtlasCache::releaseFontAtlas(fontAtlas_);
                fontAtlas_ = nullptr;
            }

            currLabelEffect_ = LabelEffect::NORMAL;
            contentDirty_ = false;
            numberOfLines_ = 0;
            lengthOfString_ = 0;
            utf16Text_.clear();
            utf8Text_.clear();

            outlineSize_ = 0.f;
            bmFontPath_ = "";
            systemFontDirty_ = false;
            systemFont_ = "Helvetica";
            systemFontSize_ = 12;

            if (horizontalKernings_)
            {
                delete[] horizontalKernings_;
                horizontalKernings_ = nullptr;
            }
            additionalKerning_ = 0.f;
            lineHeight_ = 0.f;
            maxLineWidth_ = 0.f;
            labelDimensions_.width = 0.f;
            labelDimensions_.height = 0.f;
            labelWidth_ = 0.f;
            labelHeight_ = 0.f;
            lineBreakWithoutSpaces_ = false;

            effectColorF_ = Color4F::BLACK;
            textColor_ = Color4B::WHITE;
            textColorF_ = Color4F::WHITE;
            setColor(Color3B::WHITE);

            shadowDirty_ = false;
            shadowEnabled_ = false;
            shadowBlurRadius_ = 0.f;

            useDistanceField_ = false;
            useA8Shader_ = false;
            clipEnabled_ = false;
            blendFuncDirty_ = false;
            blendFunc_ = BlendFunc::ALPHA_PREMULTIPLIED;
            isOpacityModifyRGB_ = false;
            insideBounds_ = true;
        }

        void Label::updateShaderProgram()
        {
            switch (currLabelEffect_)
            {
            case LabelEffect::NORMAL:
                if (useDistanceField_)
                    setU3DShaderState(ShaderState::getOrCreateWithShaderName(Unity3DShader::SHADER_NAME_LABEL_DISTANCEFIELD_NORMAL));
                else if (useA8Shader_)
                    setU3DShaderState(ShaderState::getOrCreateWithShaderName(Unity3DShader::SHADER_NAME_LABEL_NORMAL));
                else if (shadowEnabled_)
                    setU3DShaderState(ShaderState::getOrCreateWithShaderName(Unity3DShader::SHADER_NAME_POSITION_TEXTURE_COLOR));
                else
                    setU3DShaderState(ShaderState::getOrCreateWithShaderName(Unity3DShader::SHADER_NAME_POSITION_TEXTURE_COLOR_NO_MVP));

                break;
            case LabelEffect::OUTLINE:
                setU3DShaderState(ShaderState::getOrCreateWithShaderName(Unity3DShader::SHADER_NAME_LABEL_OUTLINE));
                uniformEffectColor_ = getU3DShader()->getUniformLocation("u_effectColor");
                break;
            case LabelEffect::GLOW:
                if (useDistanceField_)
                {
                    setU3DShaderState(ShaderState::getOrCreateWithShaderName(Unity3DShader::SHADER_NAME_LABEL_DISTANCEFIELD_GLOW));
                    uniformEffectColor_ = getU3DShader()->getUniformLocation("u_effectColor");
                }
                break;
            default:
                return;
            }

            uniformTextColor_ = getU3DShader()->getUniformLocation("u_textColor");
        }

        void Label::setFontAtlas(FontAtlas* atlas,bool distanceFieldEnabled /* = false */, bool useA8Shader /* = false */)
        {
            if (atlas == fontAtlas_)
            {
                FontAtlasCache::releaseFontAtlas(atlas);
                return;
            }

            if (fontAtlas_)
            {
                batchNodes_.clear();
                FontAtlasCache::releaseFontAtlas(fontAtlas_);
                fontAtlas_ = nullptr;
            }

            fontAtlas_ = atlas;
            if (reusedLetter_ == nullptr)
            {
                reusedLetter_ = Sprite::create();
                reusedLetter_->setOpacityModifyRGB(isOpacityModifyRGB_);
                reusedLetter_->retain();
                reusedLetter_->setAnchorPoint(MATH::Vec2fTOPLEFT);
            }

            if (fontAtlas_)
            {
                lineHeight_ = fontAtlas_->getCommonLineHeight();
                contentDirty_ = true;
            }
            useDistanceField_ = distanceFieldEnabled;
            useA8Shader_ = useA8Shader;

            currLabelEffect_ = LabelEffect::NORMAL;
            updateShaderProgram();
        }

        void Label::setString(const std::string& text)
        {
            if (text.compare(utf8Text_))
            {
                utf8Text_ = text;
                contentDirty_ = true;

                std::u16string utf16String;
                UTILS::STRING::UTF8ToUTF16(utf8Text_, utf16String);
                utf16Text_  = utf16String;
            }
        }

        void Label::setMaxLineWidth(float maxLineWidth)
        {
            if (labelWidth_ == 0 && maxLineWidth_ != maxLineWidth)
            {
                maxLineWidth_ = maxLineWidth;
                contentDirty_ = true;
            }
        }

        void Label::setDimensions(float width, float height)
        {
            if (height != labelHeight_ || width != labelWidth_)
            {
                labelWidth_ = width;
                labelHeight_ = height;
                labelDimensions_.width = width;
                labelDimensions_.height = height;

                maxLineWidth_ = width;
                contentDirty_ = true;
            }
        }

        void Label::setLineBreakWithoutSpace(bool breakWithoutSpace)
        {
            if (breakWithoutSpace != lineBreakWithoutSpaces_)
            {
                lineBreakWithoutSpaces_ = breakWithoutSpace;
                contentDirty_ = true;
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
                auto character = utf16Text_[index];
                if (character == '\r') {
                    recordPlaceholderInfo(index, character);
                    continue;
                }
                if (character == '\n') {
                    linesWidth_.push_back(letterRight);
                    letterRight = 0.f;
                    lineIndex++;
                    nextLetterX = 0.f;
                    nextLetterY -= lineHeight_;
                    recordPlaceholderInfo(index, character);
                    continue;
                }

                if (fontAtlas_->getLetterDefinitionForChar(character, letterDef) == false)
                {
                    recordPlaceholderInfo(index, character);
                    continue;
                }

                auto letterX = nextLetterX;
                if (maxLineWidth_ > 0.f && nextLetterX > 0.f && letterX + letterDef.width > maxLineWidth_)
                {
                    linesWidth_.push_back(letterRight);
                    letterRight = 0.f;
                    lineIndex++;
                    nextLetterX = 0.f;
                    nextLetterY -= lineHeight_;
                }
                else
                {
                    letterPosition.x = letterX;
                }
                letterPosition.y = nextLetterY;
                recordLetterInfo(letterPosition, character, index, lineIndex);

                if (horizontalKernings_ && index < textLen - 1)
                    nextLetterX += horizontalKernings_[index + 1];
                nextLetterX += additionalKerning_;

                letterRight = letterPosition.x + letterDef.width;

                if (highestY < letterPosition.y)
                    highestY = letterPosition.y;
                if (lowestY > letterPosition.y - letterDef.height)
                    lowestY = letterPosition.y - letterDef.height;
                if (longestLine < letterRight)
                    longestLine = letterRight;
            }

            linesWidth_.push_back(letterRight);

            numberOfLines_ = lineIndex + 1;
            textDesiredHeight_ = (numberOfLines_ * lineHeight_);
            MATH::Sizef contentSize(labelWidth_, labelHeight_);
            if (labelWidth_ <= 0.f)
                contentSize.width = longestLine;
            if (labelHeight_ <= 0.f)
                contentSize.height = textDesiredHeight_;
            setContentSize(contentSize);

            tailoredTopY_ = contentSize.height;
            tailoredBottomY_ = 0.f;
            if (highestY > 0.f)
                tailoredTopY_ = contentSize.height + highestY;
            if (lowestY < -textDesiredHeight_)
                tailoredBottomY_ = textDesiredHeight_ + lowestY;

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
                auto character = utf16Text_[index];
                if (character == '\n')
                {
                    linesWidth_.push_back(letterRight);
                    letterRight = 0.f;
                    lineIndex++;
                    nextWordX = 0.f;
                    nextWordY -= lineHeight_;
                    recordPlaceholderInfo(index, character);
                    index++;
                    continue;
                }

                auto wordLen = UTILS::STRING::GetWordLen(utf16Text_, index, textLen);
                float wordHighestY = highestY;;
                float wordLowestY = lowestY;
                float wordRight = letterRight;
                float nextLetterX = nextWordX;
                bool newLine = false;
                for (int tmp = 0; tmp < wordLen; ++tmp)
                {
                    int letterIndex = index + tmp;
                    character = utf16Text_[letterIndex];
                    if (character == '\r')
                    {
                        recordPlaceholderInfo(letterIndex, character);
                        continue;
                    }
                    if (fontAtlas_->getLetterDefinitionForChar(character, letterDef) == false)
                    {
                        recordPlaceholderInfo(letterIndex, character);
                        continue;
                    }

                    auto letterX = nextLetterX;
                    if (maxLineWidth_ > 0.f && nextWordX > 0.f && letterX + letterDef.width > maxLineWidth_)
                    {
                        linesWidth_.push_back(letterRight);
                        letterRight = 0.f;
                        lineIndex++;
                        nextWordX = 0.f;
                        nextWordY -= lineHeight_;
                        newLine = true;
                        break;
                    }
                    else
                    {
                        letterPosition.x = letterX;
                    }
                    letterPosition.y = nextWordY;
                    recordLetterInfo(letterPosition, character, letterIndex, lineIndex);

                    if (horizontalKernings_ && letterIndex < textLen - 1)
                        nextLetterX += horizontalKernings_[letterIndex + 1];
                    nextLetterX += additionalKerning_;

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

            linesWidth_.push_back(letterRight);

            numberOfLines_ = lineIndex + 1;
            textDesiredHeight_ = numberOfLines_ * lineHeight_;
            MATH::Sizef contentSize(labelWidth_, labelHeight_);
            if (labelWidth_ <= 0.f)
                contentSize.width = longestLine;
            if (labelHeight_ <= 0.f)
                contentSize.height = textDesiredHeight_;
            setContentSize(contentSize);

            tailoredTopY_ = contentSize.height;
            tailoredBottomY_ = 0.f;
            if (highestY > 0.f)
                tailoredTopY_ = contentSize.height + highestY;
            if (lowestY < -textDesiredHeight_)
                tailoredBottomY_ = textDesiredHeight_ + lowestY;

            return true;
        }

        void Label::updateLabelLetters()
        {
            if (!letters_.empty())
            {
                MATH::Rectf uvRect;
                LabelLetter* letterSprite;
                int letterIndex;

                for (auto it = letters_.begin(); it != letters_.end();)
                {
                    letterIndex = it->first;
                    letterSprite = (LabelLetter*)it->second;

                    if (letterIndex >= lengthOfString_)
                    {
                        Node::removeChild(letterSprite, true);
                        it = letters_.erase(it);
                    }
                    else
                    {
                        auto& letterInfo = lettersInfo_[letterIndex];
                        auto& letterDef = fontAtlas_->letterDefinitions_[letterInfo.utf16Char];
                        uvRect.size.height = letterDef.height;
                        uvRect.size.width = letterDef.width;
                        uvRect.origin.x = letterDef.U;
                        uvRect.origin.y = letterDef.V;

                        letterSprite->setTexture(fontAtlas_->getTexture(letterDef.textureID));
                        if (letterDef.width <= 0.f || letterDef.height <= 0.f)
                        {
                            letterSprite->setTextureAtlas(nullptr);
                        }
                        else
                        {
                            letterSprite->setTextureRect(uvRect, false, uvRect.size);
                            letterSprite->setTextureAtlas(batchNodes_.at(letterDef.textureID)->getTextureAtlas());
                            letterSprite->setAtlasIndex(lettersInfo_[letterIndex].atlasIndex);
                        }

                        auto px = letterInfo.positionX + letterDef.width / 2 + linesOffsetX_[letterInfo.lineIndex];
                        auto py = letterInfo.positionY - letterDef.height / 2 + letterOffsetY_;
                        letterSprite->setPosition(px, py);

                        ++it;
                    }
                }
            }
        }

        void Label::alignText()
        {
            if (fontAtlas_ == nullptr || utf16Text_.empty())
            {
                setContentSize(MATH::SizefZERO);
                return;
            }

            fontAtlas_->prepareLetterDefinitions(utf16Text_);
            auto& textures = fontAtlas_->getTextures();
            if (textures.size() > batchNodes_.size())
            {
                for (auto index = batchNodes_.size(); index < textures.size(); ++index)
                {
                    auto batchNode = SpriteBatchNode::createWithTexture(textures.at(index));
                    if (batchNode)
                    {
                        isOpacityModifyRGB_ = batchNode->getTexture()->hasPremultipliedAlpha();
                        blendFunc_ = batchNode->getBlendFunc();
                        batchNode->setAnchorPoint(MATH::Vec2fTOPLEFT);
                        batchNode->setPosition(MATH::Vec2fZERO);
                        batchNodes_.pushBack(batchNode);
                    }
                }
            }
            if (batchNodes_.empty())
            {
                return;
            }
            reusedLetter_->setBatchNode(batchNodes_.at(0));

            lengthOfString_ = 0;
            textDesiredHeight_ = 0.f;
            linesWidth_.clear();
            if (maxLineWidth_ > 0.f && !lineBreakWithoutSpaces_)
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
            if (static_cast<uint64>(letterIndex) >= lettersInfo_.size())
            {
                LetterInfo tmpInfo;
                lettersInfo_.push_back(tmpInfo);
            }
            lettersInfo_[letterIndex].lineIndex = lineIndex;
            lettersInfo_[letterIndex].utf16Char = utf16Char;
            lettersInfo_[letterIndex].valid = true;
            lettersInfo_[letterIndex].positionX = point.x;
            lettersInfo_[letterIndex].positionY = point.y;
        }

        void Label::recordPlaceholderInfo(int letterIndex, char16_t utf16Char)
        {
            if (static_cast<uint64>(letterIndex) >= lettersInfo_.size())
            {
                LetterInfo tmpInfo;
                lettersInfo_.push_back(tmpInfo);
            }
            lettersInfo_[letterIndex].utf16Char = utf16Char;
            lettersInfo_[letterIndex].valid = false;
        }

        void Label::updateQuads()
        {
            for (auto&& batchNode : batchNodes_)
            {
                batchNode->getTextureAtlas()->removeAllQuads();
            }

            for (int ctr = 0; ctr < lengthOfString_; ++ctr)
            {
                if (lettersInfo_[ctr].valid)
                {
                    auto& letterDef = fontAtlas_->letterDefinitions_[lettersInfo_[ctr].utf16Char];

                    reusedRect_.size.height = letterDef.height;
                    reusedRect_.size.width  = letterDef.width;
                    reusedRect_.origin.x    = letterDef.U;
                    reusedRect_.origin.y    = letterDef.V;

                    auto py = lettersInfo_[ctr].positionY + letterOffsetY_;
                    if (labelHeight_ > 0.f) {
                        if (py > tailoredTopY_)
                        {
                            auto clipTop = py - tailoredTopY_;
                            reusedRect_.origin.y += clipTop;
                            reusedRect_.size.height -= clipTop;
                            py -= clipTop;
                        }
                        if (py - letterDef.height < tailoredBottomY_)
                        {
                            reusedRect_.size.height = (py < tailoredBottomY_) ? 0.f : (py - tailoredBottomY_);
                        }
                    }

                    if (reusedRect_.size.height > 0.f && reusedRect_.size.width > 0.f)
                    {
                        reusedLetter_->setTextureRect(reusedRect_, false, reusedRect_.size);
                        reusedLetter_->setPosition(lettersInfo_[ctr].positionX + linesOffsetX_[lettersInfo_[ctr].lineIndex], py);
                        auto index = static_cast<int>(batchNodes_.at(letterDef.textureID)->getTextureAtlas()->getTotalQuads());
                        lettersInfo_[ctr].atlasIndex = index;
                        batchNodes_.at(letterDef.textureID)->insertQuadFromSprite(reusedLetter_, index);
                    }
                }
            }
        }

        void Label::enableOutline(const Color4B& outlineColor,int outlineSize /* = -1 */)
        {
            if (outlineSize > 0 || currLabelEffect_ == LabelEffect::OUTLINE)
            {
               if (effectColorF_ != outlineColor || outlineSize_ != outlineSize)
                {
                    effectColorF_.red = outlineColor.red / 255.f;
                    effectColorF_.green = outlineColor.green / 255.f;
                    effectColorF_.blue = outlineColor.blue / 255.f;
                    effectColorF_.alpha = outlineColor.alpha / 255.f;
                    outlineSize_ = outlineSize;
                    currLabelEffect_ = LabelEffect::OUTLINE;
                    contentDirty_ = true;
                }
            }
        }

        void Label::enableShadow(const Color4B& shadowColor /* = Color4B::BLACK */,const MATH::Sizef &offset /* = Size(2 ,-2)*/)
        {
            shadowEnabled_ = true;
            shadowDirty_ = true;

            shadowOffset_.width = offset.width;
            shadowOffset_.height = offset.height;
            //TODO: support blur for shadow

            shadowColor3B_.red = shadowColor.red;
            shadowColor3B_.green = shadowColor.green;
            shadowColor3B_.blue = shadowColor.blue;
            shadowOpacity_ = shadowColor.alpha;

            if (!systemFontDirty_ && !contentDirty_ && textSprite_)
            {
                // TODO
            }

            shadowColor4F_.red = shadowColor.red / 255.0f;
            shadowColor4F_.green = shadowColor.green / 255.0f;
            shadowColor4F_.blue = shadowColor.blue / 255.0f;
            shadowColor4F_.alpha = shadowColor.alpha / 255.0f;
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
                if (currLabelEffect_ == LabelEffect::OUTLINE)
                {
                    currLabelEffect_ = LabelEffect::NORMAL;
                    contentDirty_ = true;
                }
                break;
            case LabelEffect::SHADOW:
                if (shadowEnabled_)
                {
                    shadowEnabled_ = false;
                    SAFE_RELEASE_NULL(shadowNode_);
                }
                break;
            case LabelEffect::GLOW:
                if (currLabelEffect_ == LabelEffect::GLOW)
                {
                    currLabelEffect_ = LabelEffect::NORMAL;
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

            if (textSprite_)
            {
                textSprite_->setCameraMask(mask, applyChildren);
            }
            if (shadowNode_)
            {
                shadowNode_->setCameraMask(mask, applyChildren);
            }
        }

        void Label::updateContent() {
            if (systemFontDirty_) {
                if (fontAtlas_) {
                    batchNodes_.clear();
                    FontAtlasCache::releaseFontAtlas(fontAtlas_);
                    fontAtlas_ = nullptr;
                }

                systemFontDirty_ = false;
            }

            SAFE_RELEASE_NULL(textSprite_);
            SAFE_RELEASE_NULL(shadowNode_);

            if (fontAtlas_) {
                std::u16string utf16String;
                UTILS::STRING::UTF8ToUTF16(utf8Text_, utf16String);
                utf16Text_ = utf16String;
                alignText();
            }
            else {
                auto texture = Unity3DCreator::CreateTexture();
                texture->initWithString(utf8Text_.c_str(), stringToTextureLoader_, stringtoTextureOwner_);
                textSprite_ = Sprite::createWithTexture(texture);
                //set camera mask using label's camera mask, because _textSprite may be null when setting camera mask to label
                textSprite_->setCameraMask(getCameraMask());
                textSprite_->setGlobalZOrder(getGlobalZOrder());
                textSprite_->setAnchorPoint(MATH::Vec2fBOTTOMLEFT);
                this->setContentSize(textSprite_->getContentSize());
                texture->release();
                if (blendFuncDirty_) {
                    textSprite_->setBlendFunc(blendFunc_);
                }

                textSprite_->retain();
                textSprite_->updateDisplayedColor(displayedColor_);
                textSprite_->updateDisplayedOpacity(displayedOpacity_);
            }
            contentDirty_ = false;
        }

        void Label::onDrawShadow(Unity3DShaderSet* u3dShader)
        {
            Color3B oldColor = realColor_;
            GLubyte oldOPacity = displayedOpacity_;
            displayedOpacity_ = shadowOpacity_;
            setColor(shadowColor3B_);

            u3dShader->setUniformsForBuiltins(shadowTransform_);
            for (auto&& it : letters_)
            {
                it.second->updateTransform();
            }
            for (auto&& batchNode : batchNodes_)
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
            Unity3DGLState::OpenGLState().blendFunc.set(blendFunc_.src, blendFunc_.dst);

            if (shadowEnabled_)
            {
                onDrawShadow(u3dShader);
            }

            u3dShader->setUniformsForBuiltins(transform);
            for (auto&& it : letters_)
            {
                it.second->updateTransform();
            }

            for (auto&& batchNode : batchNodes_)
            {
                batchNode->getTextureAtlas()->drawQuads();
            }
        }

        void Label::draw(Renderer *renderer, const MATH::Matrix4 &transform, uint32_t flags)
        {
            if (batchNodes_.empty() || lengthOfString_ <= 0)
            {
                return;
            }
            // Don't do calculate the culling if the transform was not updated
            bool transformUpdated = flags & FLAGS_TRANSFORM_DIRTY;
                insideBounds_ = transformUpdated ? Director::getInstance().checkVisibility(transform, contentSize_) : insideBounds_;

            if (insideBounds_)
            {
                customCommand_.init(globalZOrder_, transform, flags);
                customCommand_.func = std::bind(&Label::onDraw, this, transform, transformUpdated);

                renderer->addCommand(&customCommand_);
            }
        }

        void Label::visit(Renderer *renderer, const MATH::Matrix4 &parentTransform, uint32_t parentFlags)
        {
            if (! visible_ || (utf8Text_.empty() && children_.empty()) )
            {
                return;
            }

            if (systemFontDirty_ || contentDirty_)
            {
                updateContent();
            }

            uint32_t flags = processParentFlags(parentTransform, parentFlags);

            if (!utf8Text_.empty() && shadowEnabled_ && (shadowDirty_ || (flags & FLAGS_DIRTY_MASK)))
            {
                position_.x += shadowOffset_.width;
                position_.y += shadowOffset_.height;
                transformDirty_ = inverseDirty_ = true;

                shadowTransform_ = transform(parentTransform);

                position_.x -= shadowOffset_.width;
                position_.y -= shadowOffset_.height;
                transformDirty_ = inverseDirty_ = true;

                shadowDirty_ = false;
            }

            if (children_.empty() && !textSprite_)
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

                uint64 i = 0;
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
            if (textSprite_)
            {
                if (shadowNode_)
                {
                    shadowNode_->visit(renderer, modelViewTransform_, flags);
                }
                textSprite_->visit(renderer, modelViewTransform_, flags);
            }
            else if (!utf8Text_.empty())
            {
                draw(renderer, modelViewTransform_, flags);
            }
        }

        Sprite* Label::getLetter(int letterIndex)
        {
            Sprite* letter = nullptr;
            do
            {
                if (systemFontDirty_)
                {
                    break;
                }

                auto contentDirty = contentDirty_;
                if (contentDirty)
                {
                    updateContent();
                }

                if (textSprite_ == nullptr && letterIndex < lengthOfString_)
                {
                    const auto &letterInfo = lettersInfo_[letterIndex];
                    if (!letterInfo.valid)
                    {
                        break;
                    }

                    if (letters_.find(letterIndex) != letters_.end())
                    {
                        letter = letters_[letterIndex];
                    }

                    if (letter == nullptr)
                    {
                        auto& letterDef = fontAtlas_->letterDefinitions_[letterInfo.utf16Char];
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
                            letter = LabelLetter::createWithTexture(fontAtlas_->getTexture(textureID), uvRect);
                            letter->setTextureAtlas(batchNodes_.at(textureID)->getTextureAtlas());
                            letter->setAtlasIndex(letterInfo.atlasIndex);
                            auto px = letterInfo.positionX + uvRect.size.width / 2 + linesOffsetX_[letterInfo.lineIndex];
                            auto py = letterInfo.positionY - uvRect.size.height / 2 + letterOffsetY_;
                            letter->setPosition(px,py);
                            letter->setOpacity(realOpacity_);
                        }

                        addChild(letter);
                        letters_[letterIndex] = letter;
                    }
                }
            } while (false);

            return letter;
        }

        void Label::setLineHeight(float height)
        {
            if (lineHeight_ != height)
            {
                lineHeight_ = height;
                contentDirty_ = true;
            }
        }

        float Label::getLineHeight() const
        {
            return textSprite_ ? 0.0f : lineHeight_;
        }

        void Label::setAdditionalKerning(float space)
        {
            if (additionalKerning_ != space)
            {
                additionalKerning_ = space;
                contentDirty_ = true;
            }
        }

        float Label::getAdditionalKerning() const
        {
            return additionalKerning_;
        }

        void Label::computeStringNumLines()
        {
            int quantityOfLines = 1;

            if (utf16Text_.empty())
            {
                numberOfLines_ = 0;
                return;
            }

            // count number of lines
            uint64 stringLen = utf16Text_.length();
            for (uint64 i = 0; i < stringLen - 1; ++i)
            {
                if (utf16Text_[i] == '\n')
                {
                    quantityOfLines++;
                }
            }

            numberOfLines_ = quantityOfLines;
        }

        int Label::getStringNumLines()
        {
            if (contentDirty_)
            {
                updateContent();
            }

            return numberOfLines_;
        }

        int Label::getStringLength()
        {
            lengthOfString_ = static_cast<int>(utf16Text_.length());
            return lengthOfString_;
        }

        void Label::setOpacityModifyRGB(bool isOpacityModifyRGB)
        {
            if (isOpacityModifyRGB != isOpacityModifyRGB_)
            {
                isOpacityModifyRGB_ = isOpacityModifyRGB;
                updateColor();
            }
        }

        void Label::updateDisplayedColor(const Color3B& parentColor)
        {
            Node::updateDisplayedColor(parentColor);

            if (textSprite_)
            {
                textSprite_->updateDisplayedColor(displayedColor_);
                if (shadowNode_)
                {
                    shadowNode_->updateDisplayedColor(displayedColor_);
                }
            }

            for (auto&& it : letters_)
            {
                it.second->updateDisplayedColor(displayedColor_);;
            }
        }

        void Label::updateDisplayedOpacity(GLubyte parentOpacity)
        {
            Node::updateDisplayedOpacity(parentOpacity);

            if (textSprite_)
            {
                textSprite_->updateDisplayedOpacity(displayedOpacity_);
                if (shadowNode_)
                {
                    shadowNode_->updateDisplayedOpacity(displayedOpacity_);
                }
            }

            for (auto&& it : letters_)
            {
                it.second->updateDisplayedOpacity(displayedOpacity_);;
            }
        }

        void Label::setTextColor(const Color4B &color)
        {
            textColor_ = color;
            textColorF_.red = textColor_.red / 255.0f;
            textColorF_.green = textColor_.green / 255.0f;
            textColorF_.blue = textColor_.blue / 255.0f;
            textColorF_.alpha = textColor_.alpha / 255.0f;
        }

        void Label::updateColor()
        {
            if (batchNodes_.empty())
            {
                return;
            }

            Color4B color4( displayedColor_.red, displayedColor_.green, displayedColor_.blue, displayedOpacity_ );

            // special opacity for premultiplied textures
            if (isOpacityModifyRGB_)
            {
                color4.red *= displayedOpacity_/255.0f;
                color4.green *= displayedOpacity_/255.0f;
                color4.blue *= displayedOpacity_/255.0f;
            }

            TextureAtlas* textureAtlas;
            V3F_C4B_T2F_Quad *quads;
            for (auto&& batchNode:batchNodes_)
            {
                textureAtlas = batchNode->getTextureAtlas();
                quads = textureAtlas->getQuads();
                auto count = textureAtlas->getTotalQuads();

                for (uint64 index = 0; index < count; ++index)
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
            if (systemFontDirty_ || contentDirty_)
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
            blendFunc_ = blendFunc;
            blendFuncDirty_ = true;
            if (textSprite_)
            {
                textSprite_->setBlendFunc(blendFunc);
                if (shadowNode_)
                {
                    shadowNode_->setBlendFunc(blendFunc);
                }
            }
        }

        void Label::removeAllChildrenWithCleanup(bool cleanup)
        {
            Node::removeAllChildrenWithCleanup(cleanup);
            letters_.clear();
        }

        void Label::removeChild(Node* child, bool cleanup /* = true */)
        {
            Node::removeChild(child, cleanup);
            for (auto&& it : letters_)
            {
                if (it.second == child)
                {
                    letters_.erase(it.first);
                    break;
                }
            }
        }

        void Label::setGlobalZOrder(float globalZOrder)
        {
            Node::setGlobalZOrder(globalZOrder);
            if (textSprite_)
            {
                textSprite_->setGlobalZOrder(globalZOrder);
                if (shadowNode_)
                {
                    shadowNode_->setGlobalZOrder(globalZOrder);
                }
            }
        }
    }
}
