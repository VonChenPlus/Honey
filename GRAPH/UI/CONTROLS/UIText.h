#ifndef UITEXT_H
#define UITEXT_H

#include "GRAPH/UI/UIWidget.h"
#include "GRAPH/UI/CONTROLS/UILabel.h"

namespace GRAPH
{
    namespace UI
    {
        class Text : public Widget
        {
            DECLARE_CLASS_GUI_INFO

        public:
            enum class Type
            {
                SYSTEM,
                TTF
            };

            Text();
            virtual ~Text();

            static Text* create();
            static Text* create(const std::string& textContent,
                                const std::string& fontName,
                                int fontSize);

            void setString(const std::string& text);
            const std::string& getString()const;

            uint64 getStringLength()const;

            void setFontSize(int size);
            int getFontSize()const;

            void setFontName(const std::string& name);
            const std::string& getFontName()const;

            Type getType() const;

            void setTouchScaleChangeEnabled(bool enabled);
            bool isTouchScaleChangeEnabled()const;

            virtual MATH::Sizef getVirtualRendererSize() const override;
            virtual Node* getVirtualRenderer() override;

            virtual MATH::Sizef getAutoRenderSize();

            void setTextAreaSize(const MATH::Sizef &size);
            const MATH::Sizef& getTextAreaSize()const;

            void setTextHorizontalAlignment(TextHAlignment alignment);
            TextHAlignment getTextHorizontalAlignment()const;

            void setTextVerticalAlignment(TextVAlignment alignment);
            TextVAlignment getTextVerticalAlignment()const;

            void setTextColor(const Color4B color);
            const Color4B& getTextColor() const;

            void enableShadow(const Color4B& shadowColor = Color4B::BLACK,
                              const MATH::Sizef &offset = MATH::Sizef(2,-2),
                              int blurRadius = 0);
            void enableOutline(const Color4B& outlineColor,int outlineSize = 1);
            void disableEffect();
            void disableEffect(LabelEffect effect);

        public:
            virtual bool init() override;
            virtual bool init(const std::string& textContent,
                              const std::string& fontName,
                              int fontSize);

        protected:
            virtual void initRenderer() override;
            virtual void onPressStateChangedToNormal() override;
            virtual void onPressStateChangedToPressed() override;
            virtual void onPressStateChangedToDisabled() override;
            virtual void onSizeChanged() override;

            void labelScaleChangedWithSize();
            virtual Widget* createCloneInstance() override;
            virtual void copySpecialProperties(Widget* model) override;
            virtual void adaptRenderers() override;
        protected:
            bool _touchScaleChangeEnabled;
            float _normalScaleValueX;
            float _normalScaleValueY;
            std::string _fontName;
            int _fontSize;
            float _onSelectedScaleOffset;
            Label* _labelRenderer;
            bool _labelRendererAdaptDirty;
            Type _type;
        };
    }
}

#endif // UITEXT_H
