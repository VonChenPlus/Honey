#ifndef UIWIDGET_H
#define UIWIDGET_H

#include "GRAPH/Protocols.h"
#include "GRAPH/Node.h"
#include "GRAPH/UI/UILayoutParameter.h"
#include "GRAPH/Event.h"

namespace GRAPH
{
    class EventListenerTouchOneByOne;
    class Camera;

    namespace UI
    {
        class ObjectFactory
        {
        public:
            typedef HObject* (*Instance)(void);
            typedef std::function<HObject* (void)> InstanceFunc;
            struct TInfo
            {
                TInfo(void);
                TInfo(const std::string& type, Instance ins = nullptr);
                TInfo(const std::string& type, InstanceFunc ins = nullptr);
                TInfo(const TInfo &t);
                ~TInfo(void);
                TInfo& operator= (const TInfo &t);
                std::string _class;
                Instance _fun;
                InstanceFunc _func;
            };
            typedef std::unordered_map<std::string, TInfo>  FactoryMap;

            static ObjectFactory* getInstance();
            static void destroyInstance();
            HObject* createObject(const std::string &name);

            void registerType(const TInfo &t);
            void removeAll();

        protected:
            ObjectFactory(void);
            virtual ~ObjectFactory(void);
        private:
            static ObjectFactory *_sharedFactory;
            FactoryMap _typeMap;
        };

        #define DECLARE_CLASS_GUI_INFO \
            public: \
            static ObjectFactory::TInfo __Type; \
            static HObject* createInstance(void); \

        #define IMPLEMENT_CLASS_GUI_INFO(className) \
            HObject* className::createInstance(void) \
            { \
                return className::create(); \
            } \
            ObjectFactory::TInfo className::__Type(#className, &className::createInstance); \

        #define CREATE_CLASS_GUI_INFO(className) \
            ObjectFactory::TInfo(#className, &className::createInstance) \

        class LayoutComponent;

        typedef enum
        {
            TOUCH_EVENT_BEGAN,
            TOUCH_EVENT_MOVED,
            TOUCH_EVENT_ENDED,
            TOUCH_EVENT_CANCELED
        }TouchEventType;

        typedef void (HObject::*SEL_TouchEvent)(HObject*,TouchEventType);
        #define toucheventselector(_SELECTOR) (SEL_TouchEvent)(&_SELECTOR)

        class Widget : public ProtectedNode, public LayoutParameterProtocol
        {
        public:
            enum class FocusDirection
            {
                LEFT,
                RIGHT,
                UP,
                DOWN
            };

            enum class PositionType
            {
                PT_ABSOLUTE,
                PT_PERCENT
            };

            enum class SizeType
            {
                ST_ABSOLUTE,
                ST_PERCENT
            };

            enum class TouchEventType
            {
                BEGAN,
                MOVED,
                ENDED,
                CANCELED
            };

            enum class TextureResType
            {
                LOCAL = 0,
                PLIST = 1
            };

            enum class BrightStyle
            {
                NONE = -1,
                NORMAL,
                HIGHLIGHT
            };

            typedef std::function<void(HObject*,Widget::TouchEventType)> WidgetTouchCallback;
            typedef std::function<void(HObject*)> WidgetClickCallback;
            typedef std::function<void(HObject*, int)> WidgetEventCallback;

            Widget(void);
            virtual ~Widget();
            static Widget* create();

            virtual void setEnabled(bool enabled);
            bool isEnabled() const;

            void setBright(bool bright);
            bool isBright() const;

            virtual void setTouchEnabled(bool enabled);

            void setBrightStyle(BrightStyle style);

            bool isTouchEnabled() const;

            bool isHighlighted() const;
            void setHighlighted(bool hilight);

            float getLeftBoundary() const;
            float getBottomBoundary() const;
            float getRightBoundary() const;
            float getTopBoundary() const;

            virtual void visit(Renderer *renderer, const MATH::Matrix4 &parentTransform, uint32_t parentFlags) override;

            void addTouchEventListener(const WidgetTouchCallback& callback);
            void addClickEventListener(const WidgetClickCallback& callback);

            virtual void addCCSEventListener(const WidgetEventCallback& callback);

            virtual void setPosition(const MATH::Vector2f &pos) override;
            void setPositionPercent(const MATH::Vector2f &percent);

            const MATH::Vector2f& getPositionPercent();

            void setPositionType(PositionType type);
            PositionType getPositionType() const;

            virtual void setFlippedX(bool flippedX);
            virtual bool isFlippedX()const{return flippedX_;}
            virtual void setFlippedY(bool flippedY);
            virtual bool isFlippedY()const{return flippedY_;}

            virtual void setScaleX(float scaleX) override;
            virtual void setScaleY(float scaleY) override;
            virtual void setScale(float scale) override;
            virtual void setScale(float scalex, float scaley) override;
            using Node::setScaleZ;
            virtual float getScaleX() const override;
            virtual float getScaleY() const override;
            virtual float getScale() const override;
            using Node::getScaleZ;

            bool isClippingParentContainsPoint(const MATH::Vector2f& pt);

            const MATH::Vector2f& getTouchBeganPosition()const;
            const MATH::Vector2f& getTouchMovePosition()const;
            const MATH::Vector2f& getTouchEndPosition()const;

            virtual void setContentSize(const MATH::Sizef& contentSize) override;

            virtual void setSizePercent(const MATH::Vector2f &percent);

            void setSizeType(SizeType type);
            SizeType getSizeType() const;

            const MATH::Sizef& getCustomSize() const;

            virtual const MATH::Sizef& getLayoutSize() {return contentSize_;}

            const MATH::Vector2f& getSizePercent();

            virtual bool hitTest(const MATH::Vector2f &pt, const Camera* camera, MATH::Vector3f *p) const;

            virtual bool onTouchBegan(Touch *touch, Event *unusedEvent);
            virtual void onTouchMoved(Touch *touch, Event *unusedEvent);
            virtual void onTouchEnded(Touch *touch, Event *unusedEvent);
            virtual void onTouchCancelled(Touch *touch, Event *unusedEvent);

            void setLayoutParameter(LayoutParameter* parameter);
            LayoutParameter* getLayoutParameter()const override;

            virtual void ignoreContentAdaptWithSize(bool ignore);
            bool isIgnoreContentAdaptWithSize() const;

            MATH::Vector2f getWorldPosition()const;

            virtual Node* getVirtualRenderer();

            virtual MATH::Sizef getVirtualRendererSize() const;

            Widget* clone();

            virtual void onEnter() override;
            virtual void onExit() override;

            void updateSizeAndPosition();
            void updateSizeAndPosition(const MATH::Sizef& parentSize);

            void setActionTag(int tag);
            int getActionTag()const;

            void setPropagateTouchEvents(bool isPropagate);

            bool isPropagateTouchEvents()const;

            void setSwallowTouches(bool swallow);
            bool isSwallowTouches()const;

            bool isFocused()const;
            void setFocused(bool focus);

            bool isFocusEnabled()const;
            void setFocusEnabled(bool enable);

            virtual Widget* findNextFocusedWidget(FocusDirection direction, Widget* current);

            void requestFocus();

            Widget* getCurrentFocusedWidget()const;

            static void enableDpadNavigation(bool enable);

            std::function<void(Widget*,Widget*)> onFocusChanged;
            std::function<Widget*(FocusDirection)> onNextFocusedWidget;

            void setUnifySizeEnabled(bool enable);
            bool isUnifySizeEnabled()const;

            void setCallbackName(const std::string& callbackName) { callbackName_ = callbackName; }
            const std::string& getCallbackName() const{ return callbackName_; }

            void setCallbackType(const std::string& callbackType) { callbackType_ = callbackType; }
            const std::string& getCallbackType() const{ return callbackType_; }

            void setLayoutComponentEnabled(bool enable);
            bool isLayoutComponentEnabled()const;

        public:
            virtual bool init() override;
            virtual void interceptTouchEvent(TouchEventType event, Widget* sender, Touch *touch);

            void propagateTouchEvent(TouchEventType event, Widget* sender, Touch *touch);

            void onFocusChange(Widget* widgetLostFocus, Widget* widgetGetFocus);

            void  dispatchFocusEvent(Widget* widgetLoseFocus, Widget* widgetGetFocus);

        protected:
            ShaderState* getNormalShaderState()const;
            ShaderState* getGrayShaderState()const;

            virtual void onSizeChanged();
            virtual void initRenderer();

            virtual void onPressStateChangedToNormal();
            virtual void onPressStateChangedToPressed();
            virtual void onPressStateChangedToDisabled();

            void pushDownEvent();
            void moveEvent();

            virtual void releaseUpEvent();
            virtual void cancelUpEvent();


            virtual void adaptRenderers(){}
            void updateChildrenDisplayedRGBA();

            void copyProperties(Widget* model);
            virtual Widget* createCloneInstance();
            virtual void copySpecialProperties(Widget* model);
            virtual void copyClonedWidgetChildren(Widget* model);

            Widget* getWidgetParent();
            void updateContentSizeWithTextureSize(const MATH::Sizef& size);

            bool isAncestorsEnabled();
            Widget* getAncensterWidget(Node* node);
            bool isAncestorsVisible(Node* node);

            void cleanupWidget();
            LayoutComponent* getOrCreateLayoutComponent();

        protected:
            bool usingLayoutComponent_;
            bool unifySize_;
            bool enabled_;
            bool bright_;
            bool touchEnabled_;
            bool highlight_;
            bool affectByClipping_;
            bool ignoreSize_;
            bool propagateTouchEvents_;

            BrightStyle brightStyle_;
            SizeType sizeType_;
            PositionType positionType_;

            int actionTag_;

            MATH::Sizef customSize_;

            MATH::Vector2f sizePercent_;
            MATH::Vector2f positionPercent_;

            bool hitted_;
            const Camera *hittedByCamera_;
            EventListenerTouchOneByOne* touchListener_;
            MATH::Vector2f touchBeganPosition_;
            MATH::Vector2f touchMovePosition_;
            MATH::Vector2f touchEndPosition_;

            bool flippedX_;
            bool flippedY_;

            HObjectMap<int,LayoutParameter*> layoutParameterDictionary_;
            LayoutParameter::Type layoutParameterType_;

            bool focused_;
            bool focusEnabled_;

            static Widget *focusedWidget_;

            HObject*       touchEventListener_;
            SEL_TouchEvent    touchEventSelector_;
            WidgetTouchCallback touchEventCallback_;
            WidgetClickCallback clickEventListener_;
            WidgetEventCallback EventCallback_;

            std::string callbackType_;
            std::string callbackName_;
        private:
            class FocusNavigationController;
            static FocusNavigationController* focusNavigationController_;
        };
    }
}

#endif // UIWIDGET_H
