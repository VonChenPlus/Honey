#ifndef UIWIDGET_H
#define UIWIDGET_H

#include "GRAPH/BASE/Protocols.h"
#include "GRAPH/BASE/Node.h"
#include "GRAPH/UI/LAYOUTS/UILayoutParameter.h"
#include "GRAPH/BASE/Event.h"

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

            /**
             * Widget position type for layout.
             */
            enum class PositionType
            {
                PT_ABSOLUTE,
                PT_PERCENT
            };

            /**
             * Widget size type for layout.
             */
            enum class SizeType
            {
                ST_ABSOLUTE,
                ST_PERCENT
            };

            /**
             * Touch event type.
             */
            enum class TouchEventType
            {
                BEGAN,
                MOVED,
                ENDED,
                CANCELED
            };

            /**
             * Texture resource type.
             * - LOCAL:  It means the texture is loaded from image.
             * - PLIST: It means the texture is loaded from texture atlas.
             */
            enum class TextureResType
            {
                LOCAL = 0,
                PLIST = 1
            };

            /**
             * Widget bright style.
             */
            enum class BrightStyle
            {
                NONE = -1,
                NORMAL,
                HIGHLIGHT
            };

            typedef std::function<void(HObject*,Widget::TouchEventType)> ccWidgetTouchCallback;
            /**
             * Widget click event callback.
             */
            typedef std::function<void(HObject*)> ccWidgetClickCallback;
            /**
             * Widget custom event callback.
             * It is mainly used together with Cocos Studio.
             */
            typedef std::function<void(HObject*, int)> ccWidgetEventCallback;
            /**
             * Default constructor
             * @js ctor
             * @lua new
             */
            Widget(void);

            /**
             * Default destructor
             * @js NA
             * @lua NA
             */
            virtual ~Widget();
            /**
             * Create and return a empty Widget instance pointer.
             */
            static Widget* create();

            /**
             * Sets whether the widget is enabled
             *
             * true if the widget is enabled, widget may be touched , false if the widget is disabled, widget cannot be touched.
             *
             * Note: If you want to change the widget's appearance  to disabled state, you should also call  `setBright(false)`.
             *
             * The default value is true, a widget is default to enable touch.
             *
             * @param enabled Set to true to enable touch, false otherwise.
             */
            virtual void setEnabled(bool enabled);

            /**
             * Determines if the widget is enabled or not.
             *
             * @return true if the widget is enabled, false if the widget is disabled.
             */
            bool isEnabled() const;

            /**
             * Sets whether the widget is bright
             *
             * The default value is true, a widget is default to bright
             *
             * @param bright   true if the widget is bright, false if the widget is dark.
             */
            void setBright(bool bright);

            /**
             * Determines if the widget is bright
             *
             * @return true if the widget is bright, false if the widget is dark.
             */
            bool isBright() const;

            /**
             * Sets whether the widget is touch enabled.
             *
             * The default value is false, a widget is default to touch disabled.
             *
             * @param enabled   True if the widget is touch enabled, false if the widget is touch disabled.
             */
            virtual void setTouchEnabled(bool enabled);

            /**
             * To set the bright style of widget.
             *
             * @see BrightStyle
             *
             * @param style   BrightStyle::NORMAL means the widget is in normal state, BrightStyle::HIGHLIGHT means the widget is in highlight state.
             */
            void setBrightStyle(BrightStyle style);

            /**
             * Determines if the widget is touch enabled
             *
             * @return true if the widget is touch enabled, false if the widget is touch disabled.
             */
            bool isTouchEnabled() const;

            /**
             * Determines if the widget is highlighted
             *
             * @return true if the widget is highlighted, false if the widget is not hignlighted .
             */
            bool isHighlighted() const;

            /**
             * Sets whether the widget is hilighted
             *
             * The default value is false, a widget is default to not hilighted
             *
             * @param hilight   true if the widget is hilighted, false if the widget is not hilighted.
             */
            void setHighlighted(bool hilight);

            /**
             * Gets the left boundary position of this widget in parent's coordination system.
             * @return The left boundary position of this widget.
             */
            float getLeftBoundary() const;

            /**
             * Gets the bottom boundary position of this widget in parent's coordination system.
             * @return The bottom boundary position of this widget.
             */
            float getBottomBoundary() const;

            /**
             * Gets the right boundary position of this widget in parent's coordination system.
             * @return The right boundary position of this widget.
             */
            float getRightBoundary() const;

            /**
             * Gets the top boundary position of this widget in parent's coordination system.
             * @return The top boundary position of this widget.
             */
            float getTopBoundary() const;

            /**
             * @js NA
             */
            virtual void visit(Renderer *renderer, const MATH::Matrix4 &parentTransform, uint32_t parentFlags) override;

            /**
             * Set a callback to touch vent listener.
             *@param callback  The callback in `ccWidgetEventCallback.`
             */
            void addTouchEventListener(const ccWidgetTouchCallback& callback);

            /**
             * Set a click event handler to the widget.
             * @param callback The callback in `ccWidgetClickCallback`.
             */
            void addClickEventListener(const ccWidgetClickCallback& callback);
            /**
             * Set a event handler to the widget in order to use cocostudio editor and framework
             * @param callback The callback in `ccWidgetEventCallback`.
             * @lua NA
             */
            virtual void addCCSEventListener(const ccWidgetEventCallback& callback);
            /**/

            /**
             * Changes the position (x,y) of the widget in OpenGL coordinates
             *
             * Usually we use p(x,y) to compose a MATH::Vector2f object.
             * The original point (0,0) is at the left-bottom corner of screen.
             *
             * @param pos  The position (x,y) of the widget in OpenGL coordinates
             */
            virtual void setPosition(const MATH::Vector2f &pos) override;

            /**
             * Set the percent(x,y) of the widget in OpenGL coordinates
             *
             * @param percent  The percent (x,y) of the widget in OpenGL coordinates
             */
            void setPositionPercent(const MATH::Vector2f &percent);

            /**
             * Gets the percent (x,y) of the widget in OpenGL coordinates
             *
             * @see setPosition(const MATH::Vector2f&)
             *
             * @return The percent (x,y) of the widget in OpenGL coordinates
             */
            const MATH::Vector2f& getPositionPercent();

            /**
             * Changes the position type of the widget
             *
             * @see `PositionType`
             *
             * @param type  the position type of widget
             */
            void setPositionType(PositionType type);

            /**
             * Gets the position type of the widget
             *
             * @see `PositionType`
             *
             * @return type  the position type of widget
             */
            PositionType getPositionType() const;

            /**
             * Sets whether the widget should be flipped horizontally or not.
             *
             * @param flippedX true if the widget should be flipped horizaontally, false otherwise.
             */
            virtual void setFlippedX(bool flippedX);

            /**
             * Returns the flag which indicates whether the widget is flipped horizontally or not.
             *
             * It not only flips the texture of the widget, but also the texture of the widget's children.
             * Also, flipping relies on widget's anchor point.
             * Internally, it just use setScaleX(-1) to flip the widget.
             *
             * @return true if the widget is flipped horizaontally, false otherwise.
             */
            virtual bool isFlippedX()const{return _flippedX;};

            /**
             * Sets whether the widget should be flipped vertically or not.
             *
             * @param flippedY true if the widget should be flipped vertically, flase otherwise.
             */
            virtual void setFlippedY(bool flippedY);

            /**
             * Return the flag which indicates whether the widget is flipped vertically or not.
             *
             * It not only flips the texture of the widget, but also the texture of the widget's children.
             * Also, flipping relies on widget's anchor point.
             * Internally, it just use setScaleY(-1) to flip the widget.
             *
             * @return true if the widget is flipped vertically, flase otherwise.
             */
            virtual bool isFlippedY()const{return _flippedY;}

            //override the setScale function of Node
            virtual void setScaleX(float scaleX) override;
            virtual void setScaleY(float scaleY) override;
            virtual void setScale(float scale) override;
            virtual void setScale(float scalex, float scaley) override;
            using Node::setScaleZ;
            virtual float getScaleX() const override;
            virtual float getScaleY() const override;
            virtual float getScale() const override;
            using Node::getScaleZ;

            /**
             * Checks a point if in parent's area.
             *
             * @param pt A point in `MATH::Vector2f`.
             * @return true if the point is in parent's area, flase otherwise.
             */
            bool isClippingParentContainsPoint(const MATH::Vector2f& pt);

            /**
             * Gets the touch began point of widget when widget is selected.
             * @return the touch began point.
             */
            const MATH::Vector2f& getTouchBeganPosition()const;

            /*
             * Gets the touch move point of widget when widget is selected.
             * @return the touch move point.
             */
            const MATH::Vector2f& getTouchMovePosition()const;

            /*
             * Gets the touch end point of widget when widget is selected.
             * @return the touch end point.
             */
            const MATH::Vector2f& getTouchEndPosition()const;

            /**
             * Changes the size that is widget's size
             * @param contentSize A content size in `Size`.
             */
            virtual void setContentSize(const MATH::Sizef& contentSize) override;

            /**
             * Changes the percent that is widget's percent size
             *
             * @param percent that is widget's percent size
             */
            virtual void setSizePercent(const MATH::Vector2f &percent);

            /**
             * Changes the size type of widget.
             *
             * @see `SizeType`
             *
             * @param type that is widget's size type
             */
            void setSizeType(SizeType type);

            /**
             * Gets the size type of widget.
             *
             * @see `SizeType`
             */
            SizeType getSizeType() const;

            /**
             * Get the user defined widget size.
             *@return User defined size.
             */
            const MATH::Sizef& getCustomSize() const;

            /**
             * Get the content size of widget.
             * @warning This API exists mainly for keeping back compatibility.
             * @return
             */
            virtual const MATH::Sizef& getLayoutSize() {return _contentSize;}

            /**
             * Get size percent of widget.
             *
             * @return Percent size.
             */
            const MATH::Vector2f& getSizePercent();

            /**
             * Checks a point is in widget's content space.
             * This function is used for determining touch area of widget.
             *
             * @param pt        The point in `MATH::Vector2f`.
             * @param camera    The camera look at widget, used to convert GL screen point to near/far plane.
             * @param p         Point to a MATH::Vector3f for store the intersect point, if don't need them set to nullptr.
             * @return true if the point is in widget's content space, flase otherwise.
             */
            virtual bool hitTest(const MATH::Vector2f &pt, const Camera* camera, MATH::Vector3f *p) const;

            /**
             * A callback which will be called when touch began event is issued.
             *@param touch The touch info.
             *@param unusedEvent The touch event info.
             *@return True if user want to handle touches, false otherwise.
             */
            virtual bool onTouchBegan(Touch *touch, Event *unusedEvent);

            /**
             * A callback which will be called when touch moved event is issued.
             *@param touch The touch info.
             *@param unusedEvent The touch event info.
             */
            virtual void onTouchMoved(Touch *touch, Event *unusedEvent);

            /**
             * A callback which will be called when touch ended event is issued.
             *@param touch The touch info.
             *@param unusedEvent The touch event info.
             */
            virtual void onTouchEnded(Touch *touch, Event *unusedEvent);

            /**
             * A callback which will be called when touch cancelled event is issued.
             *@param touch The touch info.
             *@param unusedEvent The touch event info.
             */
            virtual void onTouchCancelled(Touch *touch, Event *unusedEvent);

            /**
             * Sets a LayoutParameter to widget.
             *
             * @see LayoutParameter
             * @param parameter LayoutParameter pointer
             */
            void setLayoutParameter(LayoutParameter* parameter);

            /**
             * Gets LayoutParameter of widget.
             *
             * @see LayoutParameter
             * @return LayoutParameter
             */
            LayoutParameter* getLayoutParameter()const override;

            /**
             * Toggle whether ignore user defined content size for widget.
             * Set true will ignore user defined content size which means
             * the widget size is always equal to the return value of `getVirtualRendererSize`.
             *
             * @param ignore set member variabl _ignoreSize to ignore
             */
            virtual void ignoreContentAdaptWithSize(bool ignore);

            /**
             * Query whether the widget ignores user deinfed content size or not
             *
             * @return True means ignore user defined content size, false otherwise.
             */
            bool isIgnoreContentAdaptWithSize() const;

            /**
             * Gets position of widget in world space.
             *
             * @return Position of widget in world space.
             */
            MATH::Vector2f getWorldPosition()const;

            /**
             * Gets the inner Renderer node of widget.
             *
             * For example, a button's Virtual Renderer is it's texture renderer.
             *
             * @return Node pointer.
             */
            virtual Node* getVirtualRenderer();


            /**
             *  Get the virtual renderer's size
             *@return Widget virtual renderer size.
             */
            virtual MATH::Sizef getVirtualRendererSize() const;


            /**
             * Returns the string representation of widget class name
             * @return get the class description.
             */
            virtual std::string getDescription() const override;

            /**
             * Create a new widget copy of the original one.
             * @return A cloned widget copy of original.
             */
            Widget* clone();
            /**
             * @lua NA
             */
            virtual void onEnter() override;

            /**
             * @lua NA
             */
            virtual void onExit() override;

            /**
             * Update all children's contents size and position recursively.
             * @see `updateSizeAndPosition(const Size&)`
             */
            void updateSizeAndPosition();

            /**
             * Update all children's contents size and position recursively.
             */
            void updateSizeAndPosition(const MATH::Sizef& parentSize);

            /**
             * Set the tag of action.
             *@param tag  A integer tag value.
             */
            void setActionTag(int tag);

            /**
             * Get the action tag.
             *@return Action tag.
             */
            int getActionTag()const;

            /**
             * @brief Allow widget touch events to propagate to its parents. Set false will disable propagation
             * @param isPropagate  True to allow propagation, false otherwise.
             * @since v3.3
             */
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

            void setCallbackName(const std::string& callbackName) { _callbackName = callbackName; }
            const std::string& getCallbackName() const{ return _callbackName; }

            void setCallbackType(const std::string& callbackType) { _callbackType = callbackType; }
            const std::string& getCallbackType() const{ return _callbackType; }

        public:
            virtual bool init() override;
            virtual void interceptTouchEvent(TouchEventType event, Widget* sender, Touch *touch);

            void propagateTouchEvent(TouchEventType event, Widget* sender, Touch *touch);

            void onFocusChange(Widget* widgetLostFocus, Widget* widgetGetFocus);

            void  dispatchFocusEvent(Widget* widgetLoseFocus, Widget* widgetGetFocus);

        protected:
            GLProgramState* getNormalGLProgramState()const;
            GLProgramState* getGrayGLProgramState()const;


            //call back function called when size changed.
            virtual void onSizeChanged();

            //initializes renderer of widget.
            virtual void initRenderer();

            //call back function called widget's state changed to normal.
            virtual void onPressStateChangedToNormal();
            //call back function called widget's state changed to selected.
            virtual void onPressStateChangedToPressed();
            //call back function called widget's state changed to dark.
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

        protected:
            bool _unifySize;
            bool _enabled;
            bool _bright;
            bool _touchEnabled;
            bool _highlight;
            bool _affectByClipping;
            bool _ignoreSize;
            bool _propagateTouchEvents;

            BrightStyle _brightStyle;
            SizeType _sizeType;
            PositionType _positionType;

            //used for search widget by action tag in UIHelper class
            int _actionTag;

            MATH::Sizef _customSize;

            MATH::Vector2f _sizePercent;
            MATH::Vector2f _positionPercent;

            bool _hitted;
            // weak reference of the camera which made the widget passed the hit test when response touch begin event
            // it's useful in the next touch move/end events
            const Camera *_hittedByCamera;
            EventListenerTouchOneByOne* _touchListener;
            MATH::Vector2f _touchBeganPosition;
            MATH::Vector2f _touchMovePosition;
            MATH::Vector2f _touchEndPosition;

            bool _flippedX;
            bool _flippedY;

            //use map to enble switch back and forth for user layout parameters
            HObjectMap<int,LayoutParameter*> _layoutParameterDictionary;
            LayoutParameter::Type _layoutParameterType;

            bool _focused;
            bool _focusEnabled;
            /**
             * store the only one focued widget
             */
            static Widget *_focusedWidget;  //both layout & widget will be stored in this variable

            HObject*       _touchEventListener;
            SEL_TouchEvent    _touchEventSelector;
            ccWidgetTouchCallback _touchEventCallback;
            ccWidgetClickCallback _clickEventListener;
            ccWidgetEventCallback _ccEventCallback;

            std::string _callbackType;
            std::string _callbackName;
        private:
            class FocusNavigationController;
            static FocusNavigationController* _focusNavigationController;
        };
    }
}

#endif /* defined(__Widget__) */
