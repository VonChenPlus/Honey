#ifndef UIBUTTON_H
#define UIBUTTON_H

#include "GRAPH/UI/UIWidget.h"

namespace GRAPH
{
    class SpriteFrame;

    namespace UI
    {
        class Scale9Sprite;
        class Label;

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

            Label* getTitleRenderer()const;
            void setTitleText(const std::string& text);

            const std::string getTitleText() const;
            void setTitleColor(const Color3B& color);
            Color3B getTitleColor() const;
            void setTitleFontSize(float size);
            float getTitleFontSize() const;

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
            Scale9Sprite* buttonNormalRenderer_;
            Scale9Sprite* buttonClickedRenderer_;
            Scale9Sprite* buttonDisableRenderer_;
            Label* titleRenderer_;

            float zoomScale_;
            bool prevIgnoreSize_;
            bool scale9Enabled_;
            bool pressedActionEnabled_;

            MATH::Rectf capInsetsNormal_;
            MATH::Rectf capInsetsPressed_;
            MATH::Rectf capInsetsDisabled_;

            MATH::Sizef normalTextureSize_;
            MATH::Sizef pressedTextureSize_;
            MATH::Sizef disabledTextureSize_;

            float normalTextureScaleXInSize_;
            float normalTextureScaleYInSize_;
            float pressedTextureScaleXInSize_;
            float pressedTextureScaleYInSize_;

            bool normalTextureLoaded_;
            bool pressedTextureLoaded_;
            bool disabledTextureLoaded_;
            bool normalTextureAdaptDirty_;
            bool pressedTextureAdaptDirty_;
            bool disabledTextureAdaptDirty_;

        private:
            enum class FontType
            {
                SYSTEM,
                TTF,
                BMFONT
            };

            int fontSize_;
            FontType type_;
        };
    }
}

#endif // UIBUTTON_H
