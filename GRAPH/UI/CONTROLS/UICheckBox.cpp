#include "GRAPH/UI/CONTROLS/UICheckBox.h"

namespace GRAPH
{
    namespace UI
    {
        static const int BACKGROUNDBOX_RENDERER_Z = (-1);
        static const int BACKGROUNDSELECTEDBOX_RENDERER_Z = (-1);
        static const int FRONTCROSS_RENDERER_Z = (-1);
        static const int BACKGROUNDBOXDISABLED_RENDERER_Z = (-1);
        static const int FRONTCROSSDISABLED_RENDERER_Z = (-1);

        AbstractCheckButton::AbstractCheckButton():
        backGroundBoxRenderer_(nullptr),
        backGroundSelectedBoxRenderer_(nullptr),
        frontCrossRenderer_(nullptr),
        backGroundBoxDisabledRenderer_(nullptr),
        frontCrossDisabledRenderer_(nullptr),
        isSelected_(true),
        isBackgroundSelectedTextureLoaded_(false),
        isBackgroundDisabledTextureLoaded_(false),
        isFrontCrossDisabledTextureLoaded_(false),
        backGroundTexType_(TextureResType::LOCAL),
        backGroundSelectedTexType_(TextureResType::LOCAL),
        frontCrossTexType_(TextureResType::LOCAL),
        backGroundDisabledTexType_(TextureResType::LOCAL),
        frontCrossDisabledTexType_(TextureResType::LOCAL),
        zoomScale_(0.1f),
        backgroundTextureScaleX_(1.0),
        backgroundTextureScaleY_(1.0),
        backGroundBoxRendererAdaptDirty_(true),
        backGroundSelectedBoxRendererAdaptDirty_(true),
        frontCrossRendererAdaptDirty_(true),
        backGroundBoxDisabledRendererAdaptDirty_(true),
        frontCrossDisabledRendererAdaptDirty_(true)
        {
            setTouchEnabled(true);
        }

        AbstractCheckButton::~AbstractCheckButton()
        {
        }

        bool AbstractCheckButton::init(const std::string& backGround,
                            const std::string& backGroundSeleted,
                            const std::string& cross,
                            const std::string& backGroundDisabled,
                            const std::string& frontCrossDisabled,
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

                setSelected(false);
                loadTextures(backGround, backGroundSeleted, cross, backGroundDisabled, frontCrossDisabled,texType);
            } while (0);
            return ret;
        }

        bool AbstractCheckButton::init()
        {
            if (Widget::init())
            {
                setSelected(false);
                return true;
            }
            return false;
        }

        void AbstractCheckButton::initRenderer()
        {
            backGroundBoxRenderer_ = Sprite::create();
            backGroundSelectedBoxRenderer_ = Sprite::create();
            frontCrossRenderer_ = Sprite::create();
            backGroundBoxDisabledRenderer_ = Sprite::create();
            frontCrossDisabledRenderer_ = Sprite::create();

            addProtectedChild(backGroundBoxRenderer_, BACKGROUNDBOX_RENDERER_Z, -1);
            addProtectedChild(backGroundSelectedBoxRenderer_, BACKGROUNDSELECTEDBOX_RENDERER_Z, -1);
            addProtectedChild(frontCrossRenderer_, FRONTCROSS_RENDERER_Z, -1);
            addProtectedChild(backGroundBoxDisabledRenderer_, BACKGROUNDBOXDISABLED_RENDERER_Z, -1);
            addProtectedChild(frontCrossDisabledRenderer_, FRONTCROSSDISABLED_RENDERER_Z, -1);
        }

        void AbstractCheckButton::loadTextures(const std::string& backGround,
                                    const std::string& backGroundSelected,
                                    const std::string& cross,
                                    const std::string& backGroundDisabled,
                                    const std::string& frontCrossDisabled,
                                    TextureResType texType)
        {
            loadTextureBackGround(backGround,texType);
            loadTextureBackGroundSelected(backGroundSelected,texType);
            loadTextureFrontCross(cross,texType);
            loadTextureBackGroundDisabled(backGroundDisabled,texType);
            loadTextureFrontCrossDisabled(frontCrossDisabled,texType);
        }

        void AbstractCheckButton::loadTextureBackGround(const std::string& backGround,TextureResType texType)
        {
            if (backGround.empty())
            {
                return;
            }
            backGroundTexType_ = texType;
            switch (backGroundTexType_)
            {
                case TextureResType::LOCAL:
                    backGroundBoxRenderer_->setTexture(backGround);
                    break;
                case TextureResType::PLIST:
                    backGroundBoxRenderer_->setSpriteFrame(backGround);
                    break;
                default:
                    break;
            }

            this->setupBackgroundTexture();
        }

        void AbstractCheckButton::setupBackgroundTexture()
        {

            this->updateChildrenDisplayedRGBA();

            updateContentSizeWithTextureSize(backGroundBoxRenderer_->getContentSize());
            backGroundBoxRendererAdaptDirty_ = true;
        }

        void AbstractCheckButton::loadTextureBackGround(SpriteFrame* spriteFrame)
        {
            backGroundBoxRenderer_->setSpriteFrame(spriteFrame);
            this->setupBackgroundTexture();
        }

        void AbstractCheckButton::loadTextureBackGroundSelected(const std::string& backGroundSelected,TextureResType texType)
        {
            if (backGroundSelected.empty())
            {
                return;
            }

            backGroundSelectedTexType_ = texType;
            isBackgroundSelectedTextureLoaded_ = true;
            switch (backGroundSelectedTexType_)
            {
                case TextureResType::LOCAL:
                    backGroundSelectedBoxRenderer_->setTexture(backGroundSelected);
                    break;
                case TextureResType::PLIST:
                    backGroundSelectedBoxRenderer_->setSpriteFrame(backGroundSelected);
                    break;
                default:
                    break;
            }
            this->setupBackgroundSelectedTexture();
        }

        void AbstractCheckButton::loadTextureBackGroundSelected(SpriteFrame* spriteframe)
        {
            this->backGroundSelectedBoxRenderer_->setSpriteFrame(spriteframe);
            this->setupBackgroundSelectedTexture();
        }

        void AbstractCheckButton::setupBackgroundSelectedTexture()
        {
            this->updateChildrenDisplayedRGBA();
            backGroundSelectedBoxRendererAdaptDirty_ = true;
        }

        void AbstractCheckButton::loadTextureFrontCross(const std::string& cross,TextureResType texType)
        {
            if (cross.empty())
            {
                return;
            }
            frontCrossTexType_ = texType;
            switch (frontCrossTexType_)
            {
                case TextureResType::LOCAL:
                    frontCrossRenderer_->setTexture(cross);
                    break;
                case TextureResType::PLIST:
                    frontCrossRenderer_->setSpriteFrame(cross);
                    break;
                default:
                    break;
            }
            this->setupFrontCrossTexture();
        }

        void AbstractCheckButton::loadTextureFrontCross(SpriteFrame* spriteFrame)
        {
            this->frontCrossRenderer_->setSpriteFrame(spriteFrame);
            this->setupFrontCrossTexture();
        }

        void AbstractCheckButton::setupFrontCrossTexture()
        {
            this->updateChildrenDisplayedRGBA();
            frontCrossRendererAdaptDirty_ = true;
        }

        void AbstractCheckButton::loadTextureBackGroundDisabled(const std::string& backGroundDisabled,TextureResType texType)
        {
            if (backGroundDisabled.empty())
            {
                return;
            }
            backGroundDisabledTexType_ = texType;
            isBackgroundDisabledTextureLoaded_ = true;
            switch (backGroundDisabledTexType_)
            {
                case TextureResType::LOCAL:
                    backGroundBoxDisabledRenderer_->setTexture(backGroundDisabled);
                    break;
                case TextureResType::PLIST:
                    backGroundBoxDisabledRenderer_->setSpriteFrame(backGroundDisabled);
                    break;
                default:
                    break;
            }
            this->setupBackgroundDisable();
        }

        void AbstractCheckButton::loadTextureBackGroundDisabled(SpriteFrame* spriteframe)
        {
            this->backGroundBoxDisabledRenderer_->setSpriteFrame(spriteframe);
            this->setupBackgroundDisable();
        }

        void AbstractCheckButton::setupBackgroundDisable()
        {
            this->updateChildrenDisplayedRGBA();

            backGroundBoxDisabledRendererAdaptDirty_ = true;
        }

        void AbstractCheckButton::loadTextureFrontCrossDisabled(const std::string& frontCrossDisabled,TextureResType texType)
        {
            if (frontCrossDisabled.empty())
            {
                return;
            }
            frontCrossDisabledTexType_ = texType;
            isFrontCrossDisabledTextureLoaded_ = true;
            switch (frontCrossDisabledTexType_)
            {
                case TextureResType::LOCAL:
                    frontCrossDisabledRenderer_->setTexture(frontCrossDisabled);
                    break;
                case TextureResType::PLIST:
                    frontCrossDisabledRenderer_->setSpriteFrame(frontCrossDisabled);
                    break;
                default:
                    break;
            }
            this->setupFrontCrossDisableTexture();

        }

        void AbstractCheckButton::loadTextureFrontCrossDisabled(SpriteFrame* spriteframe)
        {
            this->frontCrossDisabledRenderer_->setSpriteFrame(spriteframe);
            this->setupFrontCrossDisableTexture();
        }

        void AbstractCheckButton::setupFrontCrossDisableTexture()
        {
            this->updateChildrenDisplayedRGBA();
            frontCrossDisabledRendererAdaptDirty_ = true;
        }

        void AbstractCheckButton::onPressStateChangedToNormal()
        {
            backGroundBoxRenderer_->setVisible(true);
            backGroundSelectedBoxRenderer_->setVisible(false);
            backGroundBoxDisabledRenderer_->setVisible(false);
            frontCrossDisabledRenderer_->setVisible(false);

            backGroundBoxRenderer_->setU3DShaderState(this->getNormalShaderState());
            frontCrossRenderer_->setU3DShaderState(this->getNormalShaderState());


            backGroundBoxRenderer_->setScale(backgroundTextureScaleX_, backgroundTextureScaleY_);
            frontCrossRenderer_->setScale(backgroundTextureScaleX_, backgroundTextureScaleY_);


            if (isSelected_)
            {
                frontCrossRenderer_->setVisible(true);
                frontCrossRendererAdaptDirty_ = true;
            }
        }

        void AbstractCheckButton::onPressStateChangedToPressed()
        {
            backGroundBoxRenderer_->setU3DShaderState(this->getNormalShaderState());
            frontCrossRenderer_->setU3DShaderState(this->getNormalShaderState());

            if (!isBackgroundSelectedTextureLoaded_)
            {
                backGroundBoxRenderer_->setScale(backgroundTextureScaleX_ + zoomScale_,
                                                 backgroundTextureScaleY_ + zoomScale_);
                frontCrossRenderer_->setScale(backgroundTextureScaleX_ + zoomScale_,
                                              backgroundTextureScaleY_ + zoomScale_);
            }
            else
            {
                backGroundBoxRenderer_->setVisible(false);
                backGroundSelectedBoxRenderer_->setVisible(true);
                backGroundBoxDisabledRenderer_->setVisible(false);
                frontCrossDisabledRenderer_->setVisible(false);
            }
        }

        void AbstractCheckButton::onPressStateChangedToDisabled()
        {
            if (!isBackgroundDisabledTextureLoaded_
                || !isFrontCrossDisabledTextureLoaded_)
            {
                backGroundBoxRenderer_->setU3DShaderState(this->getGrayShaderState());
                frontCrossRenderer_->setU3DShaderState(this->getGrayShaderState());
            }
            else
            {
                backGroundBoxRenderer_->setVisible(false);
                backGroundBoxDisabledRenderer_->setVisible(true);
            }

            backGroundSelectedBoxRenderer_->setVisible(false);
            frontCrossRenderer_->setVisible(false);
            backGroundBoxRenderer_->setScale(backgroundTextureScaleX_, backgroundTextureScaleY_);
            frontCrossRenderer_->setScale(backgroundTextureScaleX_, backgroundTextureScaleY_);

            if (isSelected_)
            {
                frontCrossDisabledRenderer_->setVisible(true);
                frontCrossDisabledRendererAdaptDirty_ = true;
            }
        }

        void AbstractCheckButton::setZoomScale(float scale)
        {
            zoomScale_ = scale;
        }

        float AbstractCheckButton::getZoomScale()const
        {
            return zoomScale_;
        }

        void AbstractCheckButton::setSelected(bool selected)
        {
            if (selected == isSelected_)
            {
                return;
            }
            isSelected_ = selected;
            frontCrossRenderer_->setVisible(isSelected_);
        }

        bool AbstractCheckButton::isSelected()const
        {
            return isSelected_;
        }

        void AbstractCheckButton::onSizeChanged()
        {
            Widget::onSizeChanged();
            backGroundBoxRendererAdaptDirty_ = true;
            backGroundSelectedBoxRendererAdaptDirty_ = true;
            frontCrossRendererAdaptDirty_ = true;
            backGroundBoxDisabledRendererAdaptDirty_ = true;
            frontCrossDisabledRendererAdaptDirty_ = true;
        }

        void AbstractCheckButton::adaptRenderers()
        {
            if (backGroundBoxRendererAdaptDirty_)
            {
                backGroundTextureScaleChangedWithSize();
                backGroundBoxRendererAdaptDirty_ = false;
            }
            if (backGroundSelectedBoxRendererAdaptDirty_)
            {
                backGroundSelectedTextureScaleChangedWithSize();
                backGroundSelectedBoxRendererAdaptDirty_ = false;
            }
            if (frontCrossRendererAdaptDirty_)
            {
                frontCrossTextureScaleChangedWithSize();
                frontCrossRendererAdaptDirty_ = false;
            }
            if (backGroundBoxDisabledRendererAdaptDirty_)
            {
                backGroundDisabledTextureScaleChangedWithSize();
                backGroundBoxDisabledRendererAdaptDirty_ = false;
            }
            if (frontCrossDisabledRendererAdaptDirty_)
            {
                frontCrossDisabledTextureScaleChangedWithSize();
                frontCrossDisabledRendererAdaptDirty_ = false;
            }
        }

        MATH::Sizef AbstractCheckButton::getVirtualRendererSize() const
        {
            return backGroundBoxRenderer_->getContentSize();
        }

        Node* AbstractCheckButton::getVirtualRenderer()
        {
            return backGroundBoxRenderer_;
        }

        void AbstractCheckButton::backGroundTextureScaleChangedWithSize()
        {
            if (ignoreSize_)
            {
                backGroundBoxRenderer_->setScale(1.0f);
                backgroundTextureScaleX_ = backgroundTextureScaleY_ = 1.0f;
            }
            else
            {
                MATH::Sizef textureSize = backGroundBoxRenderer_->getContentSize();
                if (textureSize.width <= 0.0f || textureSize.height <= 0.0f)
                {
                    backGroundBoxRenderer_->setScale(1.0f);
                    backgroundTextureScaleX_ = backgroundTextureScaleY_ = 1.0f;
                    return;
                }
                float scaleX = contentSize_.width / textureSize.width;
                float scaleY = contentSize_.height / textureSize.height;
                backgroundTextureScaleX_ = scaleX;
                backgroundTextureScaleY_ = scaleY;
                backGroundBoxRenderer_->setScaleX(scaleX);
                backGroundBoxRenderer_->setScaleY(scaleY);
            }
            backGroundBoxRenderer_->setPosition(contentSize_.width / 2, contentSize_.height / 2);
        }

        void AbstractCheckButton::backGroundSelectedTextureScaleChangedWithSize()
        {
            if (ignoreSize_)
            {
                backGroundSelectedBoxRenderer_->setScale(1.0f);
            }
            else
            {
                MATH::Sizef textureSize = backGroundSelectedBoxRenderer_->getContentSize();
                if (textureSize.width <= 0.0f || textureSize.height <= 0.0f)
                {
                    backGroundSelectedBoxRenderer_->setScale(1.0f);
                    return;
                }
                float scaleX = contentSize_.width / textureSize.width;
                float scaleY = contentSize_.height / textureSize.height;
                backGroundSelectedBoxRenderer_->setScaleX(scaleX);
                backGroundSelectedBoxRenderer_->setScaleY(scaleY);
            }
            backGroundSelectedBoxRenderer_->setPosition(contentSize_.width / 2, contentSize_.height / 2);
        }

        void AbstractCheckButton::frontCrossTextureScaleChangedWithSize()
        {
            if (ignoreSize_)
            {
                frontCrossRenderer_->setScale(1.0f);
            }
            else
            {
                MATH::Sizef textureSize = frontCrossRenderer_->getContentSize();
                if (textureSize.width <= 0.0f || textureSize.height <= 0.0f)
                {
                    frontCrossRenderer_->setScale(1.0f);
                    return;
                }
                float scaleX = contentSize_.width / textureSize.width;
                float scaleY = contentSize_.height / textureSize.height;
                frontCrossRenderer_->setScaleX(scaleX);
                frontCrossRenderer_->setScaleY(scaleY);
            }
            frontCrossRenderer_->setPosition(contentSize_.width / 2, contentSize_.height / 2);
        }

        void AbstractCheckButton::backGroundDisabledTextureScaleChangedWithSize()
        {
            if (ignoreSize_)
            {
                backGroundBoxDisabledRenderer_->setScale(1.0f);
            }
            else
            {
                MATH::Sizef textureSize = backGroundBoxDisabledRenderer_->getContentSize();
                if (textureSize.width <= 0.0f || textureSize.height <= 0.0f)
                {
                    backGroundBoxDisabledRenderer_->setScale(1.0f);
                    return;
                }
                float scaleX = contentSize_.width / textureSize.width;
                float scaleY = contentSize_.height / textureSize.height;
                backGroundBoxDisabledRenderer_->setScaleX(scaleX);
                backGroundBoxDisabledRenderer_->setScaleY(scaleY);
            }
            backGroundBoxDisabledRenderer_->setPosition(contentSize_.width / 2, contentSize_.height / 2);
        }

        void AbstractCheckButton::frontCrossDisabledTextureScaleChangedWithSize()
        {
            if (ignoreSize_)
            {
                frontCrossDisabledRenderer_->setScale(1.0f);
            }
            else
            {
                MATH::Sizef textureSize = frontCrossDisabledRenderer_->getContentSize();
                if (textureSize.width <= 0.0f || textureSize.height <= 0.0f)
                {
                    frontCrossDisabledRenderer_->setScale(1.0f);
                    return;
                }
                float scaleX = contentSize_.width / textureSize.width;
                float scaleY = contentSize_.height / textureSize.height;
                frontCrossDisabledRenderer_->setScaleX(scaleX);
                frontCrossDisabledRenderer_->setScaleY(scaleY);
            }
            frontCrossDisabledRenderer_->setPosition(contentSize_.width / 2, contentSize_.height / 2);
        }

        void AbstractCheckButton::copySpecialProperties(Widget *widget)
        {
            AbstractCheckButton* abstractCheckButton = dynamic_cast<AbstractCheckButton*>(widget);
            if (abstractCheckButton)
            {
                loadTextureBackGround(abstractCheckButton->backGroundBoxRenderer_->getSpriteFrame());
                loadTextureBackGroundSelected(abstractCheckButton->backGroundSelectedBoxRenderer_->getSpriteFrame());
                loadTextureFrontCross(abstractCheckButton->frontCrossRenderer_->getSpriteFrame());
                loadTextureBackGroundDisabled(abstractCheckButton->backGroundBoxDisabledRenderer_->getSpriteFrame());
                loadTextureFrontCrossDisabled(abstractCheckButton->frontCrossDisabledRenderer_->getSpriteFrame());
                setSelected(abstractCheckButton->isSelected_);
                zoomScale_ = abstractCheckButton->zoomScale_;
                backgroundTextureScaleX_ = abstractCheckButton->backgroundTextureScaleX_;
                backgroundTextureScaleY_ = abstractCheckButton->backgroundTextureScaleY_;
                isBackgroundSelectedTextureLoaded_ = abstractCheckButton->isBackgroundSelectedTextureLoaded_;
                isBackgroundDisabledTextureLoaded_ = abstractCheckButton->isBackgroundDisabledTextureLoaded_;
                isFrontCrossDisabledTextureLoaded_ = abstractCheckButton->isFrontCrossDisabledTextureLoaded_;
            }
        }
        IMPLEMENT_CLASS_GUI_INFO(CheckBox)

        CheckBox::CheckBox():
        checkBoxEventListener_(nullptr),
        checkBoxEventSelector_(nullptr)
        {
        }

        CheckBox::~CheckBox()
        {
            checkBoxEventSelector_ = nullptr;
        }

        CheckBox* CheckBox::create()
        {
            CheckBox* widget = new (std::nothrow) CheckBox();
            if (widget && widget->init())
            {
                widget->autorelease();
                return widget;
            }
            SAFE_DELETE(widget);
            return nullptr;
        }

        CheckBox* CheckBox::create(const std::string& backGround,
                                   const std::string& backGroundSeleted,
                                   const std::string& cross,
                                   const std::string& backGroundDisabled,
                                   const std::string& frontCrossDisabled,
                                   TextureResType texType)
        {
            CheckBox *pWidget = new (std::nothrow) CheckBox;
            if (pWidget && pWidget->init(backGround,
                                         backGroundSeleted,
                                         cross,
                                         backGroundDisabled,
                                         frontCrossDisabled,
                                         texType))
            {
                pWidget->autorelease();
                return pWidget;
            }
            SAFE_DELETE(pWidget);
            return nullptr;
        }

        CheckBox* CheckBox::create(const std::string& backGround,
                                   const std::string& cross,
                                   TextureResType texType)
        {
            CheckBox *pWidget = new (std::nothrow) CheckBox;
            if (pWidget && pWidget->init(backGround,
                                         "",
                                         cross,
                                         "",
                                         "",
                                         texType))
            {
                pWidget->autorelease();
                return pWidget;
            }
            SAFE_DELETE(pWidget);
            return nullptr;
        }

        void CheckBox::releaseUpEvent()
        {
            Widget::releaseUpEvent();

            if (isSelected_)
            {
                setSelected(false);
                dispatchSelectChangedEvent(false);
            }
            else
            {
                setSelected(true);
                dispatchSelectChangedEvent(true);
            }
        }

        void CheckBox::dispatchSelectChangedEvent(bool selected)
        {
            EventType eventType = (selected ? EventType::SELECTED : EventType::UNSELECTED);
            CheckBoxEventType checkBoxEventType = (selected ? CHECKBOX_STATE_EVENT_SELECTED : CHECKBOX_STATE_EVENT_UNSELECTED);

            this->retain();
            if (checkBoxEventCallback_)
            {
                checkBoxEventCallback_(this, eventType);
            }
            if (EventCallback_)
            {
                EventCallback_(this, static_cast<int>(eventType));
            }

            if (checkBoxEventListener_ && checkBoxEventSelector_)
            {
                (checkBoxEventListener_->*checkBoxEventSelector_)(this, checkBoxEventType);
            }
            this->release();

        }

        void CheckBox::addEventListener(const CheckBoxCallback& callback)
        {
            checkBoxEventCallback_ = callback;
        }

        Widget* CheckBox::createCloneInstance()
        {
            return CheckBox::create();
        }

        void CheckBox::copySpecialProperties(Widget *widget)
        {
            CheckBox* checkBox = dynamic_cast<CheckBox*>(widget);
            if (checkBox)
            {
                AbstractCheckButton::copySpecialProperties(widget);
                checkBoxEventListener_ = checkBox->checkBoxEventListener_;
                checkBoxEventSelector_ = checkBox->checkBoxEventSelector_;
                checkBoxEventCallback_ = checkBox->checkBoxEventCallback_;
                EventCallback_ = checkBox->EventCallback_;
            }
        }
    }
}
