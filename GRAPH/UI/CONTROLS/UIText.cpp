#include "GRAPH/UI/CONTROLS/UIText.h"
#include "IO/FileUtils.h"

namespace GRAPH
{
    namespace UI
    {
        static const int LABEL_RENDERER_Z = (-1);

        IMPLEMENT_CLASS_GUI_INFO(Text)

        Text::Text():
        touchScaleChangeEnabled_(false),
        normalScaleValueX_(1.0f),
        normalScaleValueY_(1.0f),
        fontName_("Thonburi"),
        fontSize_(10),
        onSelectedScaleOffset_(0.5),
        labelRenderer_(nullptr),
        labelRendererAdaptDirty_(true),
        type_(Type::SYSTEM)
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
            labelRenderer_ = Label::create();
            addProtectedChild(labelRenderer_, LABEL_RENDERER_Z, -1);
        }


        void Text::setString(const std::string &text)
        {
            if (text == labelRenderer_->getString())
            {
                return;
            }
            labelRenderer_->setString(text);
            updateContentSizeWithTextureSize(labelRenderer_->getContentSize());
            labelRendererAdaptDirty_ = true;
        }

        const std::string& Text::getString() const
        {
            return labelRenderer_->getString();
        }

        uint64 Text::getStringLength()const
        {
            return labelRenderer_->getStringLength();
        }

        void Text::setFontSize(int size)
        {
            fontSize_ = size;
            updateContentSizeWithTextureSize(labelRenderer_->getContentSize());
            labelRendererAdaptDirty_ = true;
        }

        int Text::getFontSize()const
        {
            return fontSize_;
        }

        void Text::setFontName(const std::string& name)
        {
            type_ = Type::SYSTEM;
            fontName_ = name;
            updateContentSizeWithTextureSize(labelRenderer_->getContentSize());
            labelRendererAdaptDirty_ = true;
        }

        const std::string& Text::getFontName()const
        {
            return fontName_;
        }

        Text::Type Text::getType() const
        {
            return type_;
        }

        void Text::setTextAreaSize(const MATH::Sizef &size)
        {
            labelRenderer_->setDimensions(size.width,size.height);
            if (!ignoreSize_)
            {
                customSize_=size;
            }
            updateContentSizeWithTextureSize(labelRenderer_->getContentSize());
            labelRendererAdaptDirty_ = true;
        }

        const MATH::Sizef& Text::getTextAreaSize()const
        {
            return labelRenderer_->getDimensions();
        }

        void Text::setTextColor(const Color4B color)
        {
            labelRenderer_->setTextColor(color);
        }

        const Color4B& Text::getTextColor() const
        {
            return labelRenderer_->getTextColor();
        }

        void Text::setTouchScaleChangeEnabled(bool enable)
        {
            touchScaleChangeEnabled_ = enable;
        }

        bool Text::isTouchScaleChangeEnabled()const
        {
            return touchScaleChangeEnabled_;
        }

        void Text::onPressStateChangedToNormal()
        {
            if (!touchScaleChangeEnabled_)
            {
                return;
            }
            labelRenderer_->setScaleX(normalScaleValueX_);
            labelRenderer_->setScaleY(normalScaleValueY_);
        }

        void Text::onPressStateChangedToPressed()
        {
            if (!touchScaleChangeEnabled_)
            {
                return;
            }
            labelRenderer_->setScaleX(normalScaleValueX_ + onSelectedScaleOffset_);
            labelRenderer_->setScaleY(normalScaleValueY_ + onSelectedScaleOffset_);
        }

        void Text::onPressStateChangedToDisabled()
        {

        }

        void Text::onSizeChanged()
        {
            Widget::onSizeChanged();
            labelRendererAdaptDirty_ = true;
        }

        void Text::adaptRenderers()
        {
            if (labelRendererAdaptDirty_)
            {
                labelScaleChangedWithSize();
                labelRendererAdaptDirty_ = false;
            }
        }

        MATH::Sizef Text::getVirtualRendererSize() const
        {
            return labelRenderer_->getContentSize();
        }

        MATH::Sizef Text::getAutoRenderSize()
        {
            MATH::Sizef virtualSize = labelRenderer_->getContentSize();
            if (!ignoreSize_)
            {
                labelRenderer_->setDimensions(0, 0);
                virtualSize = labelRenderer_->getContentSize();
                labelRenderer_->setDimensions(contentSize_.width, contentSize_.height);
            }

            return virtualSize;
        }

        Node* Text::getVirtualRenderer()
        {
            return labelRenderer_;
        }

        void Text::labelScaleChangedWithSize()
        {
            if (ignoreSize_)
            {
                labelRenderer_->setDimensions(0,0);
                labelRenderer_->setScale(1.0f);
                normalScaleValueX_ = normalScaleValueY_ = 1.0f;
            }
            else
            {
                labelRenderer_->setDimensions(contentSize_.width,contentSize_.height);
                MATH::Sizef textureSize = labelRenderer_->getContentSize();
                if (textureSize.width <= 0.0f || textureSize.height <= 0.0f)
                {
                    labelRenderer_->setScale(1.0f);
                    return;
                }
                float scaleX = contentSize_.width / textureSize.width;
                float scaleY = contentSize_.height / textureSize.height;
                labelRenderer_->setScaleX(scaleX);
                labelRenderer_->setScaleY(scaleY);
                normalScaleValueX_ = scaleX;
                normalScaleValueY_ = scaleY;
            }
            labelRenderer_->setPosition(contentSize_.width / 2.0f, contentSize_.height / 2.0f);
        }

        void Text::enableShadow(const Color4B& shadowColor,const MATH::Sizef &offset, int)
        {
            labelRenderer_->enableShadow(shadowColor, offset);
        }

        void Text::enableOutline(const Color4B& outlineColor,int outlineSize)
        {
            labelRenderer_->enableOutline(outlineColor, outlineSize);
            updateContentSizeWithTextureSize(labelRenderer_->getContentSize());
            labelRendererAdaptDirty_ = true;
        }

        void Text::disableEffect()
        {
            labelRenderer_->disableEffect();
            updateContentSizeWithTextureSize(labelRenderer_->getContentSize());
            labelRendererAdaptDirty_ = true;
        }

        void Text::disableEffect(LabelEffect effect)
        {
            labelRenderer_->disableEffect(effect);
            //only outline effect will affect the content size of label
            if(LabelEffect::OUTLINE == effect)
            {
                updateContentSizeWithTextureSize(labelRenderer_->getContentSize());
                labelRendererAdaptDirty_ = true;
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
                setFontName(label->fontName_);
                setFontSize(label->getFontSize());
                setTextColor(label->getTextColor());
                setString(label->getString());
                setTouchScaleChangeEnabled(label->touchScaleChangeEnabled_);
                setTextAreaSize(label->labelRenderer_->getDimensions());
                setContentSize(label->getContentSize());
            }
        }
    }
}
