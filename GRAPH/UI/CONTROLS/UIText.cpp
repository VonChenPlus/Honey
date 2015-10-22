#include "GRAPH/UI/CONTROLS/UIText.h"
#include "IO/FileUtils.h"

namespace GRAPH
{
    namespace UI
    {
        static const int LABEL_RENDERER_Z = (-1);

        IMPLEMENT_CLASS_GUI_INFO(Text)

        Text::Text():
        _touchScaleChangeEnabled(false),
        _normalScaleValueX(1.0f),
        _normalScaleValueY(1.0f),
        _fontName("Thonburi"),
        _fontSize(10),
        _onSelectedScaleOffset(0.5),
        _labelRenderer(nullptr),
        _labelRendererAdaptDirty(true),
        _type(Type::SYSTEM)
        {
        }

        Text::~Text()
        {

        }

        Text* Text::create()
        {
            Text* widget = new (std::nothrow) Text();
            if (widget && widget->init())
            {
                widget->autorelease();
                return widget;
            }
            SAFE_DELETE(widget);
            return nullptr;
        }

        bool Text::init()
        {
            if (Widget::init())
            {
                return true;
            }
            return false;
        }

        Text* Text::create(const std::string &textContent, const std::string &fontName, int fontSize)
        {
            Text *text = new (std::nothrow) Text;
            if (text && text->init(textContent, fontName, fontSize))
            {
                text->autorelease();
                return text;
            }
            SAFE_DELETE(text);
            return nullptr;
        }

        bool Text::init(const std::string &textContent, const std::string &fontName, int fontSize)
        {
            bool ret = true;
            do
            {
                if (!Widget::init())
                {
                    ret = false;
                    break;
                }
                this->setFontName(fontName);
                this->setFontSize(fontSize);
                this->setString(textContent);
            } while (0);
            return ret;
        }

        void Text::initRenderer()
        {
            _labelRenderer = Label::create();
            addProtectedChild(_labelRenderer, LABEL_RENDERER_Z, -1);
        }


        void Text::setString(const std::string &text)
        {
            if (text == _labelRenderer->getString())
            {
                return;
            }
            _labelRenderer->setString(text);
            updateContentSizeWithTextureSize(_labelRenderer->getContentSize());
            _labelRendererAdaptDirty = true;
        }

        const std::string& Text::getString() const
        {
            return _labelRenderer->getString();
        }

        uint64 Text::getStringLength()const
        {
            return _labelRenderer->getStringLength();
        }

        void Text::setFontSize(int size)
        {
            if (_type == Type::SYSTEM)
            {
                _labelRenderer->setSystemFontSize(size);
            }
            _fontSize = size;
            updateContentSizeWithTextureSize(_labelRenderer->getContentSize());
            _labelRendererAdaptDirty = true;
        }

        int Text::getFontSize()const
        {
            return _fontSize;
        }

        void Text::setFontName(const std::string& name)
        {
            _labelRenderer->setSystemFontName(name);
            _type = Type::SYSTEM;
            _fontName = name;
            updateContentSizeWithTextureSize(_labelRenderer->getContentSize());
            _labelRendererAdaptDirty = true;
        }

        const std::string& Text::getFontName()const
        {
            return _fontName;
        }

        Text::Type Text::getType() const
        {
            return _type;
        }

        void Text::setTextAreaSize(const MATH::Sizef &size)
        {
            _labelRenderer->setDimensions(size.width,size.height);
            if (!ignoreSize_)
            {
                customSize_=size;
            }
            updateContentSizeWithTextureSize(_labelRenderer->getContentSize());
            _labelRendererAdaptDirty = true;
        }

        const MATH::Sizef& Text::getTextAreaSize()const
        {
            return _labelRenderer->getDimensions();
        }

        void Text::setTextHorizontalAlignment(TextHAlignment alignment)
        {
            _labelRenderer->setHorizontalAlignment(alignment);
        }

        TextHAlignment Text::getTextHorizontalAlignment()const
        {
            return _labelRenderer->getHorizontalAlignment();
        }

        void Text::setTextVerticalAlignment(TextVAlignment alignment)
        {
            _labelRenderer->setVerticalAlignment(alignment);
        }

        TextVAlignment Text::getTextVerticalAlignment()const
        {
            return _labelRenderer->getVerticalAlignment();
        }

        void Text::setTextColor(const Color4B color)
        {
            _labelRenderer->setTextColor(color);
        }

        const Color4B& Text::getTextColor() const
        {
            return _labelRenderer->getTextColor();
        }

        void Text::setTouchScaleChangeEnabled(bool enable)
        {
            _touchScaleChangeEnabled = enable;
        }

        bool Text::isTouchScaleChangeEnabled()const
        {
            return _touchScaleChangeEnabled;
        }

        void Text::onPressStateChangedToNormal()
        {
            if (!_touchScaleChangeEnabled)
            {
                return;
            }
            _labelRenderer->setScaleX(_normalScaleValueX);
            _labelRenderer->setScaleY(_normalScaleValueY);
        }

        void Text::onPressStateChangedToPressed()
        {
            if (!_touchScaleChangeEnabled)
            {
                return;
            }
            _labelRenderer->setScaleX(_normalScaleValueX + _onSelectedScaleOffset);
            _labelRenderer->setScaleY(_normalScaleValueY + _onSelectedScaleOffset);
        }

        void Text::onPressStateChangedToDisabled()
        {

        }

        void Text::onSizeChanged()
        {
            Widget::onSizeChanged();
            _labelRendererAdaptDirty = true;
        }

        void Text::adaptRenderers()
        {
            if (_labelRendererAdaptDirty)
            {
                labelScaleChangedWithSize();
                _labelRendererAdaptDirty = false;
            }
        }

        MATH::Sizef Text::getVirtualRendererSize() const
        {
            return _labelRenderer->getContentSize();
        }

        MATH::Sizef Text::getAutoRenderSize()
        {
            MATH::Sizef virtualSize = _labelRenderer->getContentSize();
            if (!ignoreSize_)
            {
                _labelRenderer->setDimensions(0, 0);
                virtualSize = _labelRenderer->getContentSize();
                _labelRenderer->setDimensions(contentSize_.width, contentSize_.height);
            }

            return virtualSize;
        }

        Node* Text::getVirtualRenderer()
        {
            return _labelRenderer;
        }

        void Text::labelScaleChangedWithSize()
        {
            if (ignoreSize_)
            {
                _labelRenderer->setDimensions(0,0);
                _labelRenderer->setScale(1.0f);
                _normalScaleValueX = _normalScaleValueY = 1.0f;
            }
            else
            {
                _labelRenderer->setDimensions(contentSize_.width,contentSize_.height);
                MATH::Sizef textureSize = _labelRenderer->getContentSize();
                if (textureSize.width <= 0.0f || textureSize.height <= 0.0f)
                {
                    _labelRenderer->setScale(1.0f);
                    return;
                }
                float scaleX = contentSize_.width / textureSize.width;
                float scaleY = contentSize_.height / textureSize.height;
                _labelRenderer->setScaleX(scaleX);
                _labelRenderer->setScaleY(scaleY);
                _normalScaleValueX = scaleX;
                _normalScaleValueY = scaleY;
            }
            _labelRenderer->setPosition(contentSize_.width / 2.0f, contentSize_.height / 2.0f);
        }

        void Text::enableShadow(const Color4B& shadowColor,const MATH::Sizef &offset, int)
        {
            _labelRenderer->enableShadow(shadowColor, offset);
        }

        void Text::enableOutline(const Color4B& outlineColor,int outlineSize)
        {
            _labelRenderer->enableOutline(outlineColor, outlineSize);
            updateContentSizeWithTextureSize(_labelRenderer->getContentSize());
            _labelRendererAdaptDirty = true;
        }

        void Text::disableEffect()
        {
            _labelRenderer->disableEffect();
            updateContentSizeWithTextureSize(_labelRenderer->getContentSize());
            _labelRendererAdaptDirty = true;
        }

        void Text::disableEffect(LabelEffect effect)
        {
            _labelRenderer->disableEffect(effect);
            //only outline effect will affect the content size of label
            if(LabelEffect::OUTLINE == effect)
            {
                updateContentSizeWithTextureSize(_labelRenderer->getContentSize());
                _labelRendererAdaptDirty = true;
            }
        }

        Widget* Text::createCloneInstance()
        {
            return Text::create();
        }

        void Text::copySpecialProperties(Widget *widget)
        {
            Text* label = dynamic_cast<Text*>(widget);
            if (label)
            {
                setFontName(label->_fontName);
                setFontSize(label->getFontSize());
                setTextColor(label->getTextColor());
                setString(label->getString());
                setTouchScaleChangeEnabled(label->_touchScaleChangeEnabled);
                setTextHorizontalAlignment(label->_labelRenderer->getHorizontalAlignment());
                setTextVerticalAlignment(label->_labelRenderer->getVerticalAlignment());
                setTextAreaSize(label->_labelRenderer->getDimensions());
                setContentSize(label->getContentSize());
            }
        }
    }
}
