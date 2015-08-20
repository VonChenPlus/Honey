#ifndef UIBUTTON_H
#define UIBUTTON_H

#include "GRAPH/UI/BASE/UIWidget.h"

namespace GRAPH
{
    class Label;
    class SpriteFrame;

    namespace UI
    {
        class Scale9Sprite;

        class Button : public Widget
        {

            DECLARE_CLASS_GUI_INFO

        public:
            Button();
            virtual ~Button();

            static Button* create();
            static Button* create(const std::string& normalImage,
                                  const std::string& selectedImage = "",
                                  const std::string& disableImage = "",
                                  TextureResType texType = TextureResType::LOCAL);

            void loadTextures(const std::string& normal,
                              const std::string& selected,
                              const std::string& disabled = "",
                              TextureResType texType = TextureResType::LOCAL);
            void loadTextureNormal(const std::string& normal, TextureResType texType = TextureResType::LOCAL);
            void loadTexturePressed(const std::string& selected, TextureResType texType = TextureResType::LOCAL);
            void loadTextureDisabled(const std::string& disabled, TextureResType texType = TextureResType::LOCAL);

            void setCapInsets(const MATH::Rectf &capInsets);
            void setCapInsetsNormalRenderer(const MATH::Rectf &capInsets);
            const MATH::Rectf& getCapInsetsNormalRenderer()const;
            void setCapInsetsPressedRenderer(const MATH::Rectf &capInsets);
            const MATH::Rectf& getCapInsetsPressedRenderer()const;
            void setCapInsetsDisabledRenderer(const MATH::Rectf &capInsets);
            const MATH::Rectf& getCapInsetsDisabledRenderer()const;

            virtual void setScale9Enabled(bool enable);
            bool isScale9Enabled()const;

            void setPressedActionEnabled(bool enabled);

            //override methods
            virtual void ignoreContentAdaptWithSize(bool ignore) override;
            virtual MATH::Sizef getVirtualRendererSize() const override;
            virtual Node* getVirtualRenderer() override;
            virtual std::string getDescription() const override;

            Label* getTitleRenderer()const;
            void setTitleText(const std::string& text);

            const std::string getTitleText() const;
            void setTitleColor(const Color3B& color);
            Color3B getTitleColor() const;
            void setTitleFontSize(float size);
            float getTitleFontSize() const;
            void setTitleFontName(const std::string& fontName);
            const std::string getTitleFontName() const;
            void setTitleAlignment(TextHAlignment hAlignment);
            void setTitleAlignment(TextHAlignment hAlignment, TextVAlignment vAlignment);

            void setZoomScale(float scale);
            float getZoomScale()const;

        public:
            virtual bool init() override;
            virtual bool init(const std::string& normalImage,
                              const std::string& selectedImage = "",
                              const std::string& disableImage = "",
                              TextureResType texType = TextureResType::LOCAL);

            virtual MATH::Sizef getNormalTextureSize() const;

        protected:
            virtual void initRenderer() override;
            virtual void onPressStateChangedToNormal() override;
            virtual void onPressStateChangedToPressed() override;
            virtual void onPressStateChangedToDisabled() override;
            virtual void onSizeChanged() override;

            void loadTextureNormal(SpriteFrame* normalSpriteFrame);
            void setupNormalTexture();
            void loadTexturePressed(SpriteFrame* pressedSpriteFrame);
            void setupPressedTexture();
            void loadTextureDisabled(SpriteFrame* disabledSpriteFrame);
            void setupDisabledTexture();

            void normalTextureScaleChangedWithSize();
            void pressedTextureScaleChangedWithSize();
            void disabledTextureScaleChangedWithSize();

            virtual void adaptRenderers() override;
            void updateTitleLocation();
            void updateContentSize();
            void createTitleRenderer();

            virtual Widget* createCloneInstance() override;
            virtual void copySpecialProperties(Widget* model) override;

            virtual MATH::Sizef getNormalSize() const;
        protected:
            Scale9Sprite* _buttonNormalRenderer;
            Scale9Sprite* _buttonClickedRenderer;
            Scale9Sprite* _buttonDisableRenderer;
            Label* _titleRenderer;

            float _zoomScale;
            bool _prevIgnoreSize;
            bool _scale9Enabled;
            bool _pressedActionEnabled;

            MATH::Rectf _capInsetsNormal;
            MATH::Rectf _capInsetsPressed;
            MATH::Rectf _capInsetsDisabled;

            MATH::Sizef _normalTextureSize;
            MATH::Sizef _pressedTextureSize;
            MATH::Sizef _disabledTextureSize;

            float _normalTextureScaleXInSize;
            float _normalTextureScaleYInSize;
            float _pressedTextureScaleXInSize;
            float _pressedTextureScaleYInSize;

            bool _normalTextureLoaded;
            bool _pressedTextureLoaded;
            bool _disabledTextureLoaded;
            bool _normalTextureAdaptDirty;
            bool _pressedTextureAdaptDirty;
            bool _disabledTextureAdaptDirty;

        private:
            enum class FontType
            {
                SYSTEM,
                TTF,
                BMFONT
            };

            int _fontSize;
            FontType _type;
        };
    }
}

#endif /* defined(__CocoGUI__Button__) */
