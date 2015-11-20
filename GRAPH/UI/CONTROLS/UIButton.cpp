#include <algorithm>
#include "GRAPH/UI/CONTROLS/UIButton.h"
#include "GRAPH/UI/UIScale9Sprite.h"
#include "GRAPH/UI/CONTROLS//UILabel.h"
#include "GRAPH/UI/UIHelper.h"
#include "GRAPH/Action.h"
#include "IO/FileUtils.h"

namespace GRAPH
{
    namespace UI
    {
        static const int NORMAL_RENDERER_Z = (-2);
        static const int PRESSED_RENDERER_Z = (-2);
        static const int DISABLED_RENDERER_Z = (-2);
        static const int TITLE_RENDERER_Z = (-1);
        static const float ZOOM_ACTION_TIME_STEP = 0.05f;

        IMPLEMENT_CLASS_GUI_INFO(Button)

        Button::Button():
        buttonNormalRenderer_(nullptr),
        buttonClickedRenderer_(nullptr),
        buttonDisableRenderer_(nullptr),
        titleRenderer_(nullptr),
        zoomScale_(0.1f),
        prevIgnoreSize_(true),
        scale9Enabled_(false),
        pressedActionEnabled_(false),
        capInsetsNormal_(MATH::RectfZERO),
        capInsetsPressed_(MATH::RectfZERO),
        capInsetsDisabled_(MATH::RectfZERO),
        normalTextureSize_(contentSize_),
        pressedTextureSize_(contentSize_),
        disabledTextureSize_(contentSize_),
        normalTextureScaleXInSize_(1.0f),
        normalTextureScaleYInSize_(1.0f),
        pressedTextureScaleXInSize_(1.0f),
        pressedTextureScaleYInSize_(1.0f),
        normalTextureLoaded_(false),
        pressedTextureLoaded_(false),
        disabledTextureLoaded_(false),
        normalTextureAdaptDirty_(true),
        pressedTextureAdaptDirty_(true),
        disabledTextureAdaptDirty_(true),
        fontSize_(10),
        type_(FontType::SYSTEM)
        {
            setTouchEnabled(true);
        }

        Button::~Button()
        {
        }

        Button* Button::create()
        {
            Button* widget = new (std::nothrow) Button();
            if (widget && widget->init())
            {
                widget->autorelease();
                return widget;
            }
            SAFE_DELETE(widget);
            return nullptr;
        }

        Button* Button::create(const std::string &normalImage,
                               const std::string& selectedImage ,
                               const std::string& disableImage,
                               TextureResType texType)
        {
            Button *btn = new (std::nothrow) Button;
            if (btn && btn->init(normalImage,selectedImage,disableImage,texType))
            {
                btn->autorelease();
                return btn;
            }
            SAFE_DELETE(btn);
            return nullptr;
        }

        bool Button::init(const std::string &normalImage,
                          const std::string& selectedImage ,
                          const std::string& disableImage,
                          TextureResType texType)
        {
            bool ret = true;
            do
            {
                if (!Widget::init())
                {
                    ret = false;
                    break;
                }

                this->loadTextures(normalImage, selectedImage, disableImage,texType);
            } while (0);
            return ret;
        }

        bool Button::init()
        {
            if (Widget::init())
            {
                return true;
            }
            return false;
        }

        void Button::initRenderer()
        {
            buttonNormalRenderer_ = Scale9Sprite::create();
            buttonClickedRenderer_ = Scale9Sprite::create();
            buttonDisableRenderer_ = Scale9Sprite::create();
            buttonClickedRenderer_->setScale9Enabled(false);
            buttonNormalRenderer_->setScale9Enabled(false);
            buttonDisableRenderer_->setScale9Enabled(false);

            addProtectedChild(buttonNormalRenderer_, NORMAL_RENDERER_Z, -1);
            addProtectedChild(buttonClickedRenderer_, PRESSED_RENDERER_Z, -1);
            addProtectedChild(buttonDisableRenderer_, DISABLED_RENDERER_Z, -1);
        }

        void Button::createTitleRenderer()
        {
            titleRenderer_ = UI::Label::create();
            titleRenderer_->setAnchorPoint(MATH::Vec2fMIDDLE);
            addProtectedChild(titleRenderer_, TITLE_RENDERER_Z, -1);
        }

        void Button::setScale9Enabled(bool able)
        {
            if (scale9Enabled_ == able)
            {
                return;
            }

            scale9Enabled_ = able;

            buttonNormalRenderer_->setScale9Enabled(scale9Enabled_);
            buttonClickedRenderer_->setScale9Enabled(scale9Enabled_);
            buttonDisableRenderer_->setScale9Enabled(scale9Enabled_);

            if (scale9Enabled_)
            {
                bool ignoreBefore = ignoreSize_;
                ignoreContentAdaptWithSize(false);
                prevIgnoreSize_ = ignoreBefore;
            }
            else
            {
                ignoreContentAdaptWithSize(prevIgnoreSize_);
            }

            setCapInsetsNormalRenderer(capInsetsNormal_);
            setCapInsetsPressedRenderer(capInsetsPressed_);
            setCapInsetsDisabledRenderer(capInsetsDisabled_);

            brightStyle_ = BrightStyle::NONE;
            setBright(bright_);

            normalTextureAdaptDirty_ = true;
            pressedTextureAdaptDirty_ = true;
            disabledTextureAdaptDirty_ = true;
        }

        bool Button::isScale9Enabled()const
        {
            return scale9Enabled_;
        }

        void Button::ignoreContentAdaptWithSize(bool ignore)
        {
            if (unifySize_)
            {
                this->updateContentSize();
                return;
            }

            if (!scale9Enabled_ || (scale9Enabled_ && !ignore))
            {
                Widget::ignoreContentAdaptWithSize(ignore);
                prevIgnoreSize_ = ignore;
            }
        }

        void Button::loadTextures(const std::string& normal,
                                  const std::string& selected,
                                  const std::string& disabled,
                                  TextureResType texType)
        {
            loadTextureNormal(normal,texType);
            loadTexturePressed(selected,texType);
            loadTextureDisabled(disabled,texType);
        }

        void Button::loadTextureNormal(const std::string& normal,TextureResType texType)
        {
            if(normal.empty())
            {
                return;
            }
            switch (texType)
            {
                case TextureResType::LOCAL:
                    buttonNormalRenderer_->initWithFile(normal);
                    break;
                case TextureResType::PLIST:
                    buttonNormalRenderer_->initWithSpriteFrameName(normal);
                    break;
                default:
                    break;
            }
            this->setupNormalTexture();

        }

        void Button::setupNormalTexture()
        {
            normalTextureSize_ = buttonNormalRenderer_->getContentSize();

            this->updateChildrenDisplayedRGBA();

            if (unifySize_ )
            {
                if (!scale9Enabled_)
                {
                    updateContentSizeWithTextureSize(this->getNormalSize());
                }
            }
            else
            {
                updateContentSizeWithTextureSize(normalTextureSize_);
            }
            normalTextureLoaded_ = true;
            normalTextureAdaptDirty_ = true;
        }

        void Button::loadTextureNormal(SpriteFrame* normalSpriteFrame)
        {
            buttonNormalRenderer_->initWithSpriteFrame(normalSpriteFrame);
            this->setupNormalTexture();
        }

        void Button::loadTexturePressed(const std::string& selected,TextureResType texType)
        {
            if (selected.empty())
            {
                return;
            }

            switch (texType)
            {
                case TextureResType::LOCAL:
                    buttonClickedRenderer_->initWithFile(selected);
                    break;
                case TextureResType::PLIST:
                    buttonClickedRenderer_->initWithSpriteFrameName(selected);
                    break;
                default:
                    break;
            }

            this->setupPressedTexture();
        }

        void Button::setupPressedTexture()
        {
            pressedTextureSize_ = buttonClickedRenderer_->getContentSize();

            this->updateChildrenDisplayedRGBA();

            pressedTextureLoaded_ = true;
            pressedTextureAdaptDirty_ = true;
        }

        void Button::loadTexturePressed(SpriteFrame* pressedSpriteFrame)
        {
            buttonClickedRenderer_->initWithSpriteFrame(pressedSpriteFrame);
            this->setupPressedTexture();
        }

        void Button::loadTextureDisabled(const std::string& disabled,TextureResType texType)
        {
            if (disabled.empty())
            {
                return;
            }

            switch (texType)
            {
                case TextureResType::LOCAL:
                    buttonDisableRenderer_->initWithFile(disabled);
                    break;
                case TextureResType::PLIST:
                    buttonDisableRenderer_->initWithSpriteFrameName(disabled);
                    break;
                default:
                    break;
            }
            this->setupDisabledTexture();
        }

        void Button::setupDisabledTexture()
        {
            disabledTextureSize_ = buttonDisableRenderer_->getContentSize();

            this->updateChildrenDisplayedRGBA();

            disabledTextureLoaded_ = true;
            disabledTextureAdaptDirty_ = true;
        }

        void Button::loadTextureDisabled(SpriteFrame* disabledSpriteFrame)
        {
            buttonDisableRenderer_->initWithSpriteFrame(disabledSpriteFrame);
            this->setupDisabledTexture();
        }

        void Button::setCapInsets(const MATH::Rectf &capInsets)
        {
            setCapInsetsNormalRenderer(capInsets);
            setCapInsetsPressedRenderer(capInsets);
            setCapInsetsDisabledRenderer(capInsets);
        }


        void Button::setCapInsetsNormalRenderer(const MATH::Rectf &capInsets)
        {
            capInsetsNormal_ = Helper::restrictCapInsetRect(capInsets, this->normalTextureSize_);

            //for performance issue
            if (!scale9Enabled_)
            {
                return;
            }
            buttonNormalRenderer_->setCapInsets(capInsetsNormal_);
        }

        void Button::setCapInsetsPressedRenderer(const MATH::Rectf &capInsets)
        {
            capInsetsPressed_ = Helper::restrictCapInsetRect(capInsets, this->pressedTextureSize_);

            //for performance issue
            if (!scale9Enabled_)
            {
                return;
            }
            buttonClickedRenderer_->setCapInsets(capInsetsPressed_);
        }

        void Button::setCapInsetsDisabledRenderer(const MATH::Rectf &capInsets)
        {
            capInsetsDisabled_ = Helper::restrictCapInsetRect(capInsets, this->disabledTextureSize_);

            //for performance issue
            if (!scale9Enabled_)
            {
                return;
            }
            buttonDisableRenderer_->setCapInsets(capInsetsDisabled_);
        }

        const MATH::Rectf& Button::getCapInsetsNormalRenderer()const
        {
            return capInsetsNormal_;
        }

        const MATH::Rectf& Button::getCapInsetsPressedRenderer()const
        {
            return capInsetsPressed_;
        }

        const MATH::Rectf& Button::getCapInsetsDisabledRenderer()const
        {
            return capInsetsDisabled_;
        }

        void Button::onPressStateChangedToNormal()
        {
            buttonNormalRenderer_->setVisible(true);
            buttonClickedRenderer_->setVisible(false);
            buttonDisableRenderer_->setVisible(false);
            buttonNormalRenderer_->setState(Scale9Sprite::State::NORMAL);

            if (pressedTextureLoaded_)
            {
                if (pressedActionEnabled_)
                {
                    buttonNormalRenderer_->stopAllActions();
                    buttonClickedRenderer_->stopAllActions();

        //            Action *zoomAction = ScaleTo::create(ZOOM_ACTION_TIME_STEP, normalTextureScaleXInSize_, normalTextureScaleYInSize_);
                    //fixme: the zoomAction will run in the next frame which will cause the buttonNormalRenderer_ to a wrong scale
                    buttonNormalRenderer_->setScale(normalTextureScaleXInSize_, normalTextureScaleYInSize_);
                    buttonClickedRenderer_->setScale(pressedTextureScaleXInSize_, pressedTextureScaleYInSize_);

                    if(nullptr != titleRenderer_)
                    {
                        titleRenderer_->stopAllActions();
                        if (unifySize_)
                        {
                            Action *zoomTitleAction = ScaleTo::create(ZOOM_ACTION_TIME_STEP, 1.0f, 1.0f);
                            titleRenderer_->runAction(zoomTitleAction);
                        }
                        else
                        {
                            titleRenderer_->setScaleX(1.0f);
                            titleRenderer_->setScaleY(1.0f);
                        }
                    }
                }
            }
            else
            {
                buttonNormalRenderer_->stopAllActions();
                buttonNormalRenderer_->setScale(normalTextureScaleXInSize_, normalTextureScaleYInSize_);

                if(nullptr != titleRenderer_)
                {
                    titleRenderer_->stopAllActions();
                    titleRenderer_->setScaleX(1.0f);
                    titleRenderer_->setScaleY(1.0f);
                }

            }
        }

        void Button::onPressStateChangedToPressed()
        {
            buttonNormalRenderer_->setState(Scale9Sprite::State::NORMAL);

            if (pressedTextureLoaded_)
            {
                buttonNormalRenderer_->setVisible(false);
                buttonClickedRenderer_->setVisible(true);
                buttonDisableRenderer_->setVisible(false);

                if (pressedActionEnabled_)
                {
                    buttonNormalRenderer_->stopAllActions();
                    buttonClickedRenderer_->stopAllActions();

                    Action *zoomAction = ScaleTo::create(ZOOM_ACTION_TIME_STEP,
                                                         pressedTextureScaleXInSize_ + zoomScale_,
                                                         pressedTextureScaleYInSize_ + zoomScale_);
                    buttonClickedRenderer_->runAction(zoomAction);

                    buttonNormalRenderer_->setScale(pressedTextureScaleXInSize_ + zoomScale_,
                                                    pressedTextureScaleYInSize_ + zoomScale_);

                    if(nullptr != titleRenderer_)
                    {
                        titleRenderer_->stopAllActions();
                        Action *zoomTitleAction = ScaleTo::create(ZOOM_ACTION_TIME_STEP,
                                                                  1.0f + zoomScale_, 1.0f + zoomScale_);
                        titleRenderer_->runAction(zoomTitleAction);
                    }
                }
            }
            else
            {
                buttonNormalRenderer_->setVisible(true);
                buttonClickedRenderer_->setVisible(true);
                buttonDisableRenderer_->setVisible(false);

                buttonNormalRenderer_->stopAllActions();
                buttonNormalRenderer_->setScale(normalTextureScaleXInSize_ +zoomScale_, normalTextureScaleYInSize_ + zoomScale_);

                if(nullptr != titleRenderer_)
                {
                    titleRenderer_->stopAllActions();
                    titleRenderer_->setScaleX(1.0f + zoomScale_);
                    titleRenderer_->setScaleY(1.0f + zoomScale_);
                }
            }
        }

        void Button::onPressStateChangedToDisabled()
        {
            //if disable resource is null
            if (!disabledTextureLoaded_)
            {
                if (normalTextureLoaded_)
                {
                    buttonNormalRenderer_->setState(Scale9Sprite::State::GRAY);
                }
            }
            else
            {
                buttonNormalRenderer_->setVisible(false);
                buttonDisableRenderer_->setVisible(true);
            }

            buttonClickedRenderer_->setVisible(false);
            buttonNormalRenderer_->setScale(normalTextureScaleXInSize_, normalTextureScaleYInSize_);
            buttonClickedRenderer_->setScale(pressedTextureScaleXInSize_, pressedTextureScaleYInSize_);
        }

        void Button::updateTitleLocation()
        {
            titleRenderer_->setPosition(contentSize_.width * 0.5f, contentSize_.height * 0.5f);
        }

        void Button::updateContentSize()
        {
            if (unifySize_)
            {
                if (scale9Enabled_)
                {
                    ProtectedNode::setContentSize(customSize_);
                }
                else
                {
                    MATH::Sizef s = getNormalSize();
                    ProtectedNode::setContentSize(s);
                }
                onSizeChanged();
                return;
            }

            if (ignoreSize_)
            {
                this->setContentSize(getVirtualRendererSize());
            }
        }

        void Button::onSizeChanged()
        {
            Widget::onSizeChanged();
            if(nullptr != titleRenderer_)
            {
                updateTitleLocation();
            }
            normalTextureAdaptDirty_ = true;
            pressedTextureAdaptDirty_ = true;
            disabledTextureAdaptDirty_ = true;
        }

        void Button::adaptRenderers()
        {
            if (normalTextureAdaptDirty_)
            {
                normalTextureScaleChangedWithSize();
                normalTextureAdaptDirty_ = false;
            }

            if (pressedTextureAdaptDirty_)
            {
                pressedTextureScaleChangedWithSize();
                pressedTextureAdaptDirty_ = false;
            }

            if (disabledTextureAdaptDirty_)
            {
                disabledTextureScaleChangedWithSize();
                disabledTextureAdaptDirty_ = false;
            }
        }

        MATH::Sizef Button::getVirtualRendererSize() const
        {
            if (unifySize_)
            {
                return this->getNormalSize();
            }

            if (nullptr != titleRenderer_)
            {
                MATH::Sizef titleSize = titleRenderer_->getContentSize();
                if (!normalTextureLoaded_ && titleRenderer_->getString().size() > 0)
                {
                    return titleSize;
                }
            }
            return normalTextureSize_;
        }

        Node* Button::getVirtualRenderer()
        {
            if (bright_)
            {
                switch (brightStyle_)
                {
                    case BrightStyle::NORMAL:
                        return buttonNormalRenderer_;
                    case BrightStyle::HIGHLIGHT:
                        return buttonClickedRenderer_;
                    default:
                        return nullptr;
                }
            }
            else
            {
                return buttonDisableRenderer_;
            }
        }

        void Button::normalTextureScaleChangedWithSize()
        {

            if (ignoreSize_ && !unifySize_)
            {
                if (!scale9Enabled_)
                {
                    buttonNormalRenderer_->setScale(1.0f);
                    normalTextureScaleXInSize_ = normalTextureScaleYInSize_ = 1.0f;
                }
            }
            else
            {
                if (scale9Enabled_)
                {
                    buttonNormalRenderer_->setPreferredSize(contentSize_);
                    normalTextureScaleXInSize_ = normalTextureScaleYInSize_ = 1.0f;
                    buttonNormalRenderer_->setScale(normalTextureScaleXInSize_,normalTextureScaleYInSize_);
                }
                else
                {
                    MATH::Sizef textureSize = normalTextureSize_;
                    if (textureSize.width <= 0.0f || textureSize.height <= 0.0f)
                    {
                        buttonNormalRenderer_->setScale(1.0f);
                        return;
                    }
                    float scaleX = contentSize_.width / textureSize.width;
                    float scaleY = contentSize_.height / textureSize.height;
                    buttonNormalRenderer_->setScaleX(scaleX);
                    buttonNormalRenderer_->setScaleY(scaleY);
                    normalTextureScaleXInSize_ = scaleX;
                    normalTextureScaleYInSize_ = scaleY;
                }
            }

            buttonNormalRenderer_->setPosition(contentSize_.width / 2.0f, contentSize_.height / 2.0f);
        }

        void Button::pressedTextureScaleChangedWithSize()
        {

            if (ignoreSize_ && !unifySize_)
            {
                if (!scale9Enabled_)
                {
                    buttonClickedRenderer_->setScale(1.0f);
                    pressedTextureScaleXInSize_ = pressedTextureScaleYInSize_ = 1.0f;
                }
            }
            else
            {
                if (scale9Enabled_)
                {
                    buttonClickedRenderer_->setPreferredSize(contentSize_);
                    pressedTextureScaleXInSize_ = pressedTextureScaleYInSize_ = 1.0f;
                    buttonClickedRenderer_->setScale(pressedTextureScaleXInSize_,pressedTextureScaleYInSize_);
                }
                else
                {
                    MATH::Sizef textureSize = pressedTextureSize_;
                    if (textureSize.width <= 0.0f || textureSize.height <= 0.0f)
                    {
                        buttonClickedRenderer_->setScale(1.0f);
                        return;
                    }
                    float scaleX = contentSize_.width / pressedTextureSize_.width;
                    float scaleY = contentSize_.height / pressedTextureSize_.height;
                    buttonClickedRenderer_->setScaleX(scaleX);
                    buttonClickedRenderer_->setScaleY(scaleY);
                    pressedTextureScaleXInSize_ = scaleX;
                    pressedTextureScaleYInSize_ = scaleY;
                }
            }
            buttonClickedRenderer_->setPosition(contentSize_.width / 2.0f, contentSize_.height / 2.0f);
        }

        void Button::disabledTextureScaleChangedWithSize()
        {

            if (ignoreSize_ && !unifySize_)
            {
                if (!scale9Enabled_)
                {
                    buttonDisableRenderer_->setScale(1.0f);
                }
            }
            else
            {
                if (scale9Enabled_)
                {
                    buttonDisableRenderer_->setScale(1.0);
                    buttonDisableRenderer_->setPreferredSize(contentSize_);
                }
                else
                {
                    MATH::Sizef textureSize = disabledTextureSize_;
                    if (textureSize.width <= 0.0f || textureSize.height <= 0.0f)
                    {
                        buttonDisableRenderer_->setScale(1.0f);
                        return;
                    }
                    float scaleX = contentSize_.width / disabledTextureSize_.width;
                    float scaleY = contentSize_.height / disabledTextureSize_.height;
                    buttonDisableRenderer_->setScaleX(scaleX);
                    buttonDisableRenderer_->setScaleY(scaleY);
                }
            }
            buttonDisableRenderer_->setPosition(contentSize_.width / 2.0f, contentSize_.height / 2.0f);
        }

        void Button::setPressedActionEnabled(bool enabled)
        {
            pressedActionEnabled_ = enabled;
        }

        void Button::setTitleText(const std::string& text)
        {
            if (text == getTitleText())
            {
                return;
            }
            if(nullptr == titleRenderer_)
            {
                this->createTitleRenderer();
            }
            titleRenderer_->setString(text);
            updateContentSize();
        }

        const std::string Button::getTitleText() const
        {
            if(nullptr == titleRenderer_)
            {
                return "";
            }
            return titleRenderer_->getString();
        }

        void Button::setTitleColor(const Color3B& color)
        {
            if(nullptr == titleRenderer_)
            {
                this->createTitleRenderer();
            }
            titleRenderer_->setTextColor(Color4B(color));
        }

        Color3B Button::getTitleColor() const
        {
            if(nullptr == titleRenderer_)
            {
                return Color3B::WHITE;
            }
            return Color3B(titleRenderer_->getTextColor());
        }

        void Button::setTitleFontSize(float size)
        {
            if (nullptr == titleRenderer_)
            {
                this->createTitleRenderer();
            }

            fontSize_ = size;
            //we can't change font size of BMFont.
            if(FontType::BMFONT != type_)
            {
                updateContentSize();
            }
        }

        float Button::getTitleFontSize() const
        {
            return fontSize_;
        }

        void Button::setZoomScale(float scale)
        {
            zoomScale_ = scale;
        }

        float Button::getZoomScale()const
        {
            return zoomScale_;
        }

        Label* Button::getTitleRenderer()const
        {
            return titleRenderer_;
        }

        Widget* Button::createCloneInstance()
        {
            return Button::create();
        }

        void Button::copySpecialProperties(Widget *widget)
        {
            Button* button = dynamic_cast<Button*>(widget);
            if (button)
            {
                prevIgnoreSize_ = button->prevIgnoreSize_;
                setScale9Enabled(button->scale9Enabled_);
                auto normalSprite = button->buttonNormalRenderer_->getSprite();
                if (nullptr != normalSprite)
                {
                    loadTextureNormal(normalSprite->getSpriteFrame());
                }
                auto clickedSprite = button->buttonClickedRenderer_->getSprite();
                if (nullptr != clickedSprite)
                {
                    loadTexturePressed(clickedSprite->getSpriteFrame());
                }
                auto disabledSprite = button->buttonDisableRenderer_->getSprite();
                if (nullptr != disabledSprite)
                {
                    loadTextureDisabled(disabledSprite->getSpriteFrame());
                }
                setCapInsetsNormalRenderer(button->capInsetsNormal_);
                setCapInsetsPressedRenderer(button->capInsetsPressed_);
                setCapInsetsDisabledRenderer(button->capInsetsDisabled_);
                if(nullptr != button->getTitleRenderer())
                {
                    setTitleText(button->getTitleText());
                    setTitleFontSize(button->getTitleFontSize());
                    setTitleColor(button->getTitleColor());
                }
                setPressedActionEnabled(button->pressedActionEnabled_);
                setZoomScale(button->zoomScale_);
            }

        }

        MATH::Sizef Button::getNormalSize() const
        {
            MATH::Sizef titleSize;
            if (titleRenderer_ != nullptr)
            {
                titleSize = titleRenderer_->getContentSize();
            }
            MATH::Sizef imageSize;
            if (buttonNormalRenderer_ != nullptr)
            {
                imageSize = buttonNormalRenderer_->getContentSize();
            }
            float width = titleSize.width > imageSize.width ? titleSize.width : imageSize.width;
            float height = titleSize.height > imageSize.height ? titleSize.height : imageSize.height;

            return MATH::Sizef(width,height);
        }

        MATH::Sizef Button::getNormalTextureSize() const
        {
            return normalTextureSize_;
        }
    }
}
