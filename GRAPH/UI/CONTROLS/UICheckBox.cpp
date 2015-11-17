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
        _backGroundBoxRenderer(nullptr),
        _backGroundSelectedBoxRenderer(nullptr),
        _frontCrossRenderer(nullptr),
        _backGroundBoxDisabledRenderer(nullptr),
        _frontCrossDisabledRenderer(nullptr),
        _isSelected(true),
        _isBackgroundSelectedTextureLoaded(false),
        _isBackgroundDisabledTextureLoaded(false),
        _isFrontCrossDisabledTextureLoaded(false),
        _backGroundTexType(TextureResType::LOCAL),
        _backGroundSelectedTexType(TextureResType::LOCAL),
        _frontCrossTexType(TextureResType::LOCAL),
        _backGroundDisabledTexType(TextureResType::LOCAL),
        _frontCrossDisabledTexType(TextureResType::LOCAL),
        _zoomScale(0.1f),
        _backgroundTextureScaleX(1.0),
        _backgroundTextureScaleY(1.0),
        _backGroundBoxRendererAdaptDirty(true),
        _backGroundSelectedBoxRendererAdaptDirty(true),
        _frontCrossRendererAdaptDirty(true),
        _backGroundBoxDisabledRendererAdaptDirty(true),
        _frontCrossDisabledRendererAdaptDirty(true)
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
            _backGroundBoxRenderer = Sprite::create();
            _backGroundSelectedBoxRenderer = Sprite::create();
            _frontCrossRenderer = Sprite::create();
            _backGroundBoxDisabledRenderer = Sprite::create();
            _frontCrossDisabledRenderer = Sprite::create();

            addProtectedChild(_backGroundBoxRenderer, BACKGROUNDBOX_RENDERER_Z, -1);
            addProtectedChild(_backGroundSelectedBoxRenderer, BACKGROUNDSELECTEDBOX_RENDERER_Z, -1);
            addProtectedChild(_frontCrossRenderer, FRONTCROSS_RENDERER_Z, -1);
            addProtectedChild(_backGroundBoxDisabledRenderer, BACKGROUNDBOXDISABLED_RENDERER_Z, -1);
            addProtectedChild(_frontCrossDisabledRenderer, FRONTCROSSDISABLED_RENDERER_Z, -1);
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
            _backGroundTexType = texType;
            switch (_backGroundTexType)
            {
                case TextureResType::LOCAL:
                    _backGroundBoxRenderer->setTexture(backGround);
                    break;
                case TextureResType::PLIST:
                    _backGroundBoxRenderer->setSpriteFrame(backGround);
                    break;
                default:
                    break;
            }

            this->setupBackgroundTexture();
        }

        void AbstractCheckButton::setupBackgroundTexture()
        {

            this->updateChildrenDisplayedRGBA();

            updateContentSizeWithTextureSize(_backGroundBoxRenderer->getContentSize());
            _backGroundBoxRendererAdaptDirty = true;
        }

        void AbstractCheckButton::loadTextureBackGround(SpriteFrame* spriteFrame)
        {
            _backGroundBoxRenderer->setSpriteFrame(spriteFrame);
            this->setupBackgroundTexture();
        }

        void AbstractCheckButton::loadTextureBackGroundSelected(const std::string& backGroundSelected,TextureResType texType)
        {
            if (backGroundSelected.empty())
            {
                return;
            }

            _backGroundSelectedTexType = texType;
            _isBackgroundSelectedTextureLoaded = true;
            switch (_backGroundSelectedTexType)
            {
                case TextureResType::LOCAL:
                    _backGroundSelectedBoxRenderer->setTexture(backGroundSelected);
                    break;
                case TextureResType::PLIST:
                    _backGroundSelectedBoxRenderer->setSpriteFrame(backGroundSelected);
                    break;
                default:
                    break;
            }
            this->setupBackgroundSelectedTexture();
        }

        void AbstractCheckButton::loadTextureBackGroundSelected(SpriteFrame* spriteframe)
        {
            this->_backGroundSelectedBoxRenderer->setSpriteFrame(spriteframe);
            this->setupBackgroundSelectedTexture();
        }

        void AbstractCheckButton::setupBackgroundSelectedTexture()
        {
            this->updateChildrenDisplayedRGBA();
            _backGroundSelectedBoxRendererAdaptDirty = true;
        }

        void AbstractCheckButton::loadTextureFrontCross(const std::string& cross,TextureResType texType)
        {
            if (cross.empty())
            {
                return;
            }
            _frontCrossTexType = texType;
            switch (_frontCrossTexType)
            {
                case TextureResType::LOCAL:
                    _frontCrossRenderer->setTexture(cross);
                    break;
                case TextureResType::PLIST:
                    _frontCrossRenderer->setSpriteFrame(cross);
                    break;
                default:
                    break;
            }
            this->setupFrontCrossTexture();
        }

        void AbstractCheckButton::loadTextureFrontCross(SpriteFrame* spriteFrame)
        {
            this->_frontCrossRenderer->setSpriteFrame(spriteFrame);
            this->setupFrontCrossTexture();
        }

        void AbstractCheckButton::setupFrontCrossTexture()
        {
            this->updateChildrenDisplayedRGBA();
            _frontCrossRendererAdaptDirty = true;
        }

        void AbstractCheckButton::loadTextureBackGroundDisabled(const std::string& backGroundDisabled,TextureResType texType)
        {
            if (backGroundDisabled.empty())
            {
                return;
            }
            _backGroundDisabledTexType = texType;
            _isBackgroundDisabledTextureLoaded = true;
            switch (_backGroundDisabledTexType)
            {
                case TextureResType::LOCAL:
                    _backGroundBoxDisabledRenderer->setTexture(backGroundDisabled);
                    break;
                case TextureResType::PLIST:
                    _backGroundBoxDisabledRenderer->setSpriteFrame(backGroundDisabled);
                    break;
                default:
                    break;
            }
            this->setupBackgroundDisable();
        }

        void AbstractCheckButton::loadTextureBackGroundDisabled(SpriteFrame* spriteframe)
        {
            this->_backGroundBoxDisabledRenderer->setSpriteFrame(spriteframe);
            this->setupBackgroundDisable();
        }

        void AbstractCheckButton::setupBackgroundDisable()
        {
            this->updateChildrenDisplayedRGBA();

            _backGroundBoxDisabledRendererAdaptDirty = true;
        }

        void AbstractCheckButton::loadTextureFrontCrossDisabled(const std::string& frontCrossDisabled,TextureResType texType)
        {
            if (frontCrossDisabled.empty())
            {
                return;
            }
            _frontCrossDisabledTexType = texType;
            _isFrontCrossDisabledTextureLoaded = true;
            switch (_frontCrossDisabledTexType)
            {
                case TextureResType::LOCAL:
                    _frontCrossDisabledRenderer->setTexture(frontCrossDisabled);
                    break;
                case TextureResType::PLIST:
                    _frontCrossDisabledRenderer->setSpriteFrame(frontCrossDisabled);
                    break;
                default:
                    break;
            }
            this->setupFrontCrossDisableTexture();

        }

        void AbstractCheckButton::loadTextureFrontCrossDisabled(SpriteFrame* spriteframe)
        {
            this->_frontCrossDisabledRenderer->setSpriteFrame(spriteframe);
            this->setupFrontCrossDisableTexture();
        }

        void AbstractCheckButton::setupFrontCrossDisableTexture()
        {
            this->updateChildrenDisplayedRGBA();
            _frontCrossDisabledRendererAdaptDirty = true;
        }

        void AbstractCheckButton::onPressStateChangedToNormal()
        {
            _backGroundBoxRenderer->setVisible(true);
            _backGroundSelectedBoxRenderer->setVisible(false);
            _backGroundBoxDisabledRenderer->setVisible(false);
            _frontCrossDisabledRenderer->setVisible(false);

            _backGroundBoxRenderer->setU3DShaderState(this->getNormalShaderState());
            _frontCrossRenderer->setU3DShaderState(this->getNormalShaderState());


            _backGroundBoxRenderer->setScale(_backgroundTextureScaleX, _backgroundTextureScaleY);
            _frontCrossRenderer->setScale(_backgroundTextureScaleX, _backgroundTextureScaleY);


            if (_isSelected)
            {
                _frontCrossRenderer->setVisible(true);
                _frontCrossRendererAdaptDirty = true;
            }
        }

        void AbstractCheckButton::onPressStateChangedToPressed()
        {
            _backGroundBoxRenderer->setU3DShaderState(this->getNormalShaderState());
            _frontCrossRenderer->setU3DShaderState(this->getNormalShaderState());

            if (!_isBackgroundSelectedTextureLoaded)
            {
                _backGroundBoxRenderer->setScale(_backgroundTextureScaleX + _zoomScale,
                                                 _backgroundTextureScaleY + _zoomScale);
                _frontCrossRenderer->setScale(_backgroundTextureScaleX + _zoomScale,
                                              _backgroundTextureScaleY + _zoomScale);
            }
            else
            {
                _backGroundBoxRenderer->setVisible(false);
                _backGroundSelectedBoxRenderer->setVisible(true);
                _backGroundBoxDisabledRenderer->setVisible(false);
                _frontCrossDisabledRenderer->setVisible(false);
            }
        }

        void AbstractCheckButton::onPressStateChangedToDisabled()
        {
            if (!_isBackgroundDisabledTextureLoaded
                || !_isFrontCrossDisabledTextureLoaded)
            {
                _backGroundBoxRenderer->setU3DShaderState(this->getGrayShaderState());
                _frontCrossRenderer->setU3DShaderState(this->getGrayShaderState());
            }
            else
            {
                _backGroundBoxRenderer->setVisible(false);
                _backGroundBoxDisabledRenderer->setVisible(true);
            }

            _backGroundSelectedBoxRenderer->setVisible(false);
            _frontCrossRenderer->setVisible(false);
            _backGroundBoxRenderer->setScale(_backgroundTextureScaleX, _backgroundTextureScaleY);
            _frontCrossRenderer->setScale(_backgroundTextureScaleX, _backgroundTextureScaleY);

            if (_isSelected)
            {
                _frontCrossDisabledRenderer->setVisible(true);
                _frontCrossDisabledRendererAdaptDirty = true;
            }
        }

        void AbstractCheckButton::setZoomScale(float scale)
        {
            _zoomScale = scale;
        }

        float AbstractCheckButton::getZoomScale()const
        {
            return _zoomScale;
        }

        void AbstractCheckButton::setSelected(bool selected)
        {
            if (selected == _isSelected)
            {
                return;
            }
            _isSelected = selected;
            _frontCrossRenderer->setVisible(_isSelected);
        }

        bool AbstractCheckButton::isSelected()const
        {
            return _isSelected;
        }

        void AbstractCheckButton::onSizeChanged()
        {
            Widget::onSizeChanged();
            _backGroundBoxRendererAdaptDirty = true;
            _backGroundSelectedBoxRendererAdaptDirty = true;
            _frontCrossRendererAdaptDirty = true;
            _backGroundBoxDisabledRendererAdaptDirty = true;
            _frontCrossDisabledRendererAdaptDirty = true;
        }

        void AbstractCheckButton::adaptRenderers()
        {
            if (_backGroundBoxRendererAdaptDirty)
            {
                backGroundTextureScaleChangedWithSize();
                _backGroundBoxRendererAdaptDirty = false;
            }
            if (_backGroundSelectedBoxRendererAdaptDirty)
            {
                backGroundSelectedTextureScaleChangedWithSize();
                _backGroundSelectedBoxRendererAdaptDirty = false;
            }
            if (_frontCrossRendererAdaptDirty)
            {
                frontCrossTextureScaleChangedWithSize();
                _frontCrossRendererAdaptDirty = false;
            }
            if (_backGroundBoxDisabledRendererAdaptDirty)
            {
                backGroundDisabledTextureScaleChangedWithSize();
                _backGroundBoxDisabledRendererAdaptDirty = false;
            }
            if (_frontCrossDisabledRendererAdaptDirty)
            {
                frontCrossDisabledTextureScaleChangedWithSize();
                _frontCrossDisabledRendererAdaptDirty = false;
            }
        }

        MATH::Sizef AbstractCheckButton::getVirtualRendererSize() const
        {
            return _backGroundBoxRenderer->getContentSize();
        }

        Node* AbstractCheckButton::getVirtualRenderer()
        {
            return _backGroundBoxRenderer;
        }

        void AbstractCheckButton::backGroundTextureScaleChangedWithSize()
        {
            if (ignoreSize_)
            {
                _backGroundBoxRenderer->setScale(1.0f);
                _backgroundTextureScaleX = _backgroundTextureScaleY = 1.0f;
            }
            else
            {
                MATH::Sizef textureSize = _backGroundBoxRenderer->getContentSize();
                if (textureSize.width <= 0.0f || textureSize.height <= 0.0f)
                {
                    _backGroundBoxRenderer->setScale(1.0f);
                    _backgroundTextureScaleX = _backgroundTextureScaleY = 1.0f;
                    return;
                }
                float scaleX = contentSize_.width / textureSize.width;
                float scaleY = contentSize_.height / textureSize.height;
                _backgroundTextureScaleX = scaleX;
                _backgroundTextureScaleY = scaleY;
                _backGroundBoxRenderer->setScaleX(scaleX);
                _backGroundBoxRenderer->setScaleY(scaleY);
            }
            _backGroundBoxRenderer->setPosition(contentSize_.width / 2, contentSize_.height / 2);
        }

        void AbstractCheckButton::backGroundSelectedTextureScaleChangedWithSize()
        {
            if (ignoreSize_)
            {
                _backGroundSelectedBoxRenderer->setScale(1.0f);
            }
            else
            {
                MATH::Sizef textureSize = _backGroundSelectedBoxRenderer->getContentSize();
                if (textureSize.width <= 0.0f || textureSize.height <= 0.0f)
                {
                    _backGroundSelectedBoxRenderer->setScale(1.0f);
                    return;
                }
                float scaleX = contentSize_.width / textureSize.width;
                float scaleY = contentSize_.height / textureSize.height;
                _backGroundSelectedBoxRenderer->setScaleX(scaleX);
                _backGroundSelectedBoxRenderer->setScaleY(scaleY);
            }
            _backGroundSelectedBoxRenderer->setPosition(contentSize_.width / 2, contentSize_.height / 2);
        }

        void AbstractCheckButton::frontCrossTextureScaleChangedWithSize()
        {
            if (ignoreSize_)
            {
                _frontCrossRenderer->setScale(1.0f);
            }
            else
            {
                MATH::Sizef textureSize = _frontCrossRenderer->getContentSize();
                if (textureSize.width <= 0.0f || textureSize.height <= 0.0f)
                {
                    _frontCrossRenderer->setScale(1.0f);
                    return;
                }
                float scaleX = contentSize_.width / textureSize.width;
                float scaleY = contentSize_.height / textureSize.height;
                _frontCrossRenderer->setScaleX(scaleX);
                _frontCrossRenderer->setScaleY(scaleY);
            }
            _frontCrossRenderer->setPosition(contentSize_.width / 2, contentSize_.height / 2);
        }

        void AbstractCheckButton::backGroundDisabledTextureScaleChangedWithSize()
        {
            if (ignoreSize_)
            {
                _backGroundBoxDisabledRenderer->setScale(1.0f);
            }
            else
            {
                MATH::Sizef textureSize = _backGroundBoxDisabledRenderer->getContentSize();
                if (textureSize.width <= 0.0f || textureSize.height <= 0.0f)
                {
                    _backGroundBoxDisabledRenderer->setScale(1.0f);
                    return;
                }
                float scaleX = contentSize_.width / textureSize.width;
                float scaleY = contentSize_.height / textureSize.height;
                _backGroundBoxDisabledRenderer->setScaleX(scaleX);
                _backGroundBoxDisabledRenderer->setScaleY(scaleY);
            }
            _backGroundBoxDisabledRenderer->setPosition(contentSize_.width / 2, contentSize_.height / 2);
        }

        void AbstractCheckButton::frontCrossDisabledTextureScaleChangedWithSize()
        {
            if (ignoreSize_)
            {
                _frontCrossDisabledRenderer->setScale(1.0f);
            }
            else
            {
                MATH::Sizef textureSize = _frontCrossDisabledRenderer->getContentSize();
                if (textureSize.width <= 0.0f || textureSize.height <= 0.0f)
                {
                    _frontCrossDisabledRenderer->setScale(1.0f);
                    return;
                }
                float scaleX = contentSize_.width / textureSize.width;
                float scaleY = contentSize_.height / textureSize.height;
                _frontCrossDisabledRenderer->setScaleX(scaleX);
                _frontCrossDisabledRenderer->setScaleY(scaleY);
            }
            _frontCrossDisabledRenderer->setPosition(contentSize_.width / 2, contentSize_.height / 2);
        }

        void AbstractCheckButton::copySpecialProperties(Widget *widget)
        {
            AbstractCheckButton* abstractCheckButton = dynamic_cast<AbstractCheckButton*>(widget);
            if (abstractCheckButton)
            {
                loadTextureBackGround(abstractCheckButton->_backGroundBoxRenderer->getSpriteFrame());
                loadTextureBackGroundSelected(abstractCheckButton->_backGroundSelectedBoxRenderer->getSpriteFrame());
                loadTextureFrontCross(abstractCheckButton->_frontCrossRenderer->getSpriteFrame());
                loadTextureBackGroundDisabled(abstractCheckButton->_backGroundBoxDisabledRenderer->getSpriteFrame());
                loadTextureFrontCrossDisabled(abstractCheckButton->_frontCrossDisabledRenderer->getSpriteFrame());
                setSelected(abstractCheckButton->_isSelected);
                _zoomScale = abstractCheckButton->_zoomScale;
                _backgroundTextureScaleX = abstractCheckButton->_backgroundTextureScaleX;
                _backgroundTextureScaleY = abstractCheckButton->_backgroundTextureScaleY;
                _isBackgroundSelectedTextureLoaded = abstractCheckButton->_isBackgroundSelectedTextureLoaded;
                _isBackgroundDisabledTextureLoaded = abstractCheckButton->_isBackgroundDisabledTextureLoaded;
                _isFrontCrossDisabledTextureLoaded = abstractCheckButton->_isFrontCrossDisabledTextureLoaded;
            }
        }
        IMPLEMENT_CLASS_GUI_INFO(CheckBox)

        CheckBox::CheckBox():
        _checkBoxEventListener(nullptr),
        _checkBoxEventSelector(nullptr)
        {
        }

        CheckBox::~CheckBox()
        {
            _checkBoxEventSelector = nullptr;
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

            if (_isSelected)
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
            if (_checkBoxEventCallback)
            {
                _checkBoxEventCallback(this, eventType);
            }
            if (EventCallback_)
            {
                EventCallback_(this, static_cast<int>(eventType));
            }

            if (_checkBoxEventListener && _checkBoxEventSelector)
            {
                (_checkBoxEventListener->*_checkBoxEventSelector)(this, checkBoxEventType);
            }
            this->release();

        }

        void CheckBox::addEventListener(const CheckBoxCallback& callback)
        {
            _checkBoxEventCallback = callback;
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
                _checkBoxEventListener = checkBox->_checkBoxEventListener;
                _checkBoxEventSelector = checkBox->_checkBoxEventSelector;
                _checkBoxEventCallback = checkBox->_checkBoxEventCallback;
                EventCallback_ = checkBox->EventCallback_;
            }
        }
    }
}
