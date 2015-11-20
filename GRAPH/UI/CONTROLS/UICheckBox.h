#ifndef UICHECKBOX_H
#define UICHECKBOX_H

#include "GRAPH/UI/UIWidget.h"
#include "GRAPH/Sprite.h"

namespace GRAPH
{
    namespace UI
    {
        class AbstractCheckButton : public Widget
        {
        public:
            void loadTextures(const std::string& background,
                              const std::string& backgroundSelected,
                              const std::string& cross,
                              const std::string& backgroundDisabled,
                              const std::string& frontCrossDisabled,
                              TextureResType texType = TextureResType::LOCAL);

            void loadTextureBackGround(const std::string& backGround,TextureResType type = TextureResType::LOCAL);
            void loadTextureBackGroundSelected(const std::string& backGroundSelected,TextureResType texType = TextureResType::LOCAL);
            void loadTextureFrontCross(const std::string& crossTextureName,TextureResType texType = TextureResType::LOCAL);
            void loadTextureBackGroundDisabled(const std::string& backGroundDisabled,TextureResType texType = TextureResType::LOCAL);
            void loadTextureFrontCrossDisabled(const std::string& frontCrossDisabled,TextureResType texType = TextureResType::LOCAL);

            bool isSelected()const;
            void setSelected(bool selected);

            virtual MATH::Sizef getVirtualRendererSize() const override;
            virtual Node* getVirtualRenderer() override;

            void setZoomScale(float scale);
            float getZoomScale()const;

        public:
            virtual bool init() override;
            virtual bool init(const std::string& backGround,
                              const std::string& backGroundSeleted,
                              const std::string& cross,
                              const std::string& backGroundDisabled,
                              const std::string& frontCrossDisabled,
                              TextureResType texType = TextureResType::LOCAL);

        protected:
            AbstractCheckButton();
            virtual ~AbstractCheckButton();

            virtual void initRenderer() override;
            virtual void onPressStateChangedToNormal() override;
            virtual void onPressStateChangedToPressed() override;
            virtual void onPressStateChangedToDisabled() override;

            void setupBackgroundTexture();
            void loadTextureBackGround(SpriteFrame* spriteFrame);
            void setupBackgroundSelectedTexture();
            void loadTextureBackGroundSelected(SpriteFrame* spriteFrame);
            void setupFrontCrossTexture();
            void loadTextureFrontCross(SpriteFrame* spriteframe);
            void setupBackgroundDisable();
            void loadTextureBackGroundDisabled(SpriteFrame* spriteframe);
            void setupFrontCrossDisableTexture();
            void loadTextureFrontCrossDisabled(SpriteFrame* spriteframe);

            virtual void dispatchSelectChangedEvent(bool selected) = 0;

            virtual void onSizeChanged() override;

            void backGroundTextureScaleChangedWithSize();
            void backGroundSelectedTextureScaleChangedWithSize();
            void frontCrossTextureScaleChangedWithSize();
            void backGroundDisabledTextureScaleChangedWithSize();
            void frontCrossDisabledTextureScaleChangedWithSize();

            virtual void copySpecialProperties(Widget* model) override;
            virtual void adaptRenderers() override;
        protected:
            Sprite* backGroundBoxRenderer_;
            Sprite* backGroundSelectedBoxRenderer_;
            Sprite* frontCrossRenderer_;
            Sprite* backGroundBoxDisabledRenderer_;
            Sprite* frontCrossDisabledRenderer_;
            bool isSelected_;

            bool isBackgroundSelectedTextureLoaded_;
            bool isBackgroundDisabledTextureLoaded_;
            bool isFrontCrossDisabledTextureLoaded_;
            TextureResType backGroundTexType_;
            TextureResType backGroundSelectedTexType_;
            TextureResType frontCrossTexType_;
            TextureResType backGroundDisabledTexType_;
            TextureResType frontCrossDisabledTexType_;

            float zoomScale_;
            float backgroundTextureScaleX_;
            float backgroundTextureScaleY_;

            bool backGroundBoxRendererAdaptDirty_;
            bool backGroundSelectedBoxRendererAdaptDirty_;
            bool frontCrossRendererAdaptDirty_;
            bool backGroundBoxDisabledRendererAdaptDirty_;
            bool frontCrossDisabledRendererAdaptDirty_;
        };

        typedef enum
        {
            CHECKBOX_STATE_EVENT_SELECTED,
            CHECKBOX_STATE_EVENT_UNSELECTED
        } CheckBoxEventType;

        typedef void (HObject::*SEL_SelectedStateEvent)(HObject*,CheckBoxEventType);
        #define checkboxselectedeventselector(_SELECTOR) (SEL_SelectedStateEvent)(&_SELECTOR)

        class CheckBox : public AbstractCheckButton
        {

            DECLARE_CLASS_GUI_INFO

        public:
            enum class EventType
            {
                SELECTED,
                UNSELECTED
            };

            typedef std::function<void(HObject*,CheckBox::EventType)> CheckBoxCallback;

            CheckBox();
            virtual ~CheckBox();

            static CheckBox* create();
            static CheckBox* create(const std::string& backGround,
                                    const std::string& backGroundSelected,
                                    const std::string& cross,
                                    const std::string& backGroundDisabled,
                                    const std::string& frontCrossDisabled,
                                    TextureResType texType = TextureResType::LOCAL);
            static CheckBox* create(const std::string& backGround,
                                    const std::string& cross,
                                    TextureResType texType = TextureResType::LOCAL);

            void addEventListener(const CheckBoxCallback& callback);

        protected:
            virtual void releaseUpEvent() override;

            virtual void dispatchSelectChangedEvent(bool selected) override;

            virtual Widget* createCloneInstance() override;
            virtual void copySpecialProperties(Widget* model) override;

        protected:
            HObject*       checkBoxEventListener_;
            SEL_SelectedStateEvent    checkBoxEventSelector_;
            CheckBoxCallback checkBoxEventCallback_;
        };
    }
}

#endif // UICHECKBOX_H
