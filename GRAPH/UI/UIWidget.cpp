#include "GRAPH/UI/UIWidget.h"
#include "GRAPH/Director.h"
#include "GRAPH/EventListener.h"
#include "GRAPH/EventDispatcher.h"
#include "GRAPH/EventDispatcher.h"
#include "GRAPH/UNITY3D/ShaderState.h"
#include "GRAPH/UI/UILayout.h"

namespace GRAPH
{
    namespace UI
    {
        ObjectFactory::TInfo::TInfo(void)
            :_class("")
            , _fun(nullptr)
            , _func(nullptr)
        {
        }

        ObjectFactory::TInfo::TInfo(const std::string& type, Instance ins)
            :_class(type)
            , _fun(ins)
            , _func(nullptr)
        {
            ObjectFactory::getInstance()->registerType(*this);
        }

        ObjectFactory::TInfo::TInfo(const std::string& type, InstanceFunc ins)
            :_class(type)
            , _fun(nullptr)
            , _func(ins)
        {
            ObjectFactory::getInstance()->registerType(*this);
        }

        ObjectFactory::TInfo::TInfo(const TInfo &t)
        {
            _class = t._class;
            _fun = t._fun;
            _func = t._func;
        }

        ObjectFactory::TInfo::~TInfo(void)
        {
            _class = "";
            _fun = nullptr;
            _func = nullptr;
        }

        ObjectFactory::TInfo& ObjectFactory::TInfo::operator= (const TInfo &t)
        {
            _class = t._class;
            _fun = t._fun;
            _func = t._func;
            return *this;
        }


        ObjectFactory* ObjectFactory::_sharedFactory = nullptr;

        ObjectFactory::ObjectFactory(void)
        {

        }

        ObjectFactory::~ObjectFactory(void)
        {
            _typeMap.clear();
        }

        ObjectFactory* ObjectFactory::getInstance()
        {
            if (nullptr == _sharedFactory)
            {
                _sharedFactory = new (std::nothrow) ObjectFactory();
            }
            return _sharedFactory;
        }

        void ObjectFactory::destroyInstance()
        {
            SAFE_DELETE(_sharedFactory);
        }

        HObject* ObjectFactory::createObject(const std::string &name)
        {
            HObject *o = nullptr;
            do
            {
                const TInfo t = _typeMap[name];
                if (t._fun != nullptr)
                {
                    o = t._fun();
                }
                else if (t._func != nullptr)
                {
                    o = t._func();
                }
            } while (0);

            return o;
        }

        void ObjectFactory::registerType(const TInfo &t)
        {
            _typeMap.insert(std::make_pair(t._class, t));
        }

        class Widget::FocusNavigationController
        {
            void enableFocusNavigation(bool flag);

            FocusNavigationController():
            _keyboardListener(nullptr),
            _firstFocusedWidget(nullptr),
            _enableFocusNavigation(false),
            _keyboardEventPriority(1)
            {
                //no-op
            }
            ~FocusNavigationController();
        protected:
            void setFirstFocsuedWidget(Widget* widget);

            void onKeypadKeyPressed(EventKeyboard::KeyCode, Event*);

            void addKeyboardEventListener();
            void removeKeyboardEventListener();

            friend class Widget;
        private:
            EventListenerKeyboard* _keyboardListener ;
            Widget* _firstFocusedWidget ;
            bool _enableFocusNavigation ;
            const int _keyboardEventPriority;
        };

        Widget::FocusNavigationController::~FocusNavigationController()
        {
            this->removeKeyboardEventListener();
        }

        void Widget::FocusNavigationController::onKeypadKeyPressed(EventKeyboard::KeyCode  keyCode, Event *)
        {
            if (_enableFocusNavigation && _firstFocusedWidget)
            {
                if (keyCode == EventKeyboard::KeyCode::KEY_DPAD_DOWN)
                {
                    _firstFocusedWidget = _firstFocusedWidget->findNextFocusedWidget(Widget::FocusDirection::DOWN, _firstFocusedWidget);
                }
                if (keyCode == EventKeyboard::KeyCode::KEY_DPAD_UP)
                {
                    _firstFocusedWidget = _firstFocusedWidget->findNextFocusedWidget(Widget::FocusDirection::UP, _firstFocusedWidget);
                }
                if (keyCode == EventKeyboard::KeyCode::KEY_DPAD_LEFT)
                {
                    _firstFocusedWidget = _firstFocusedWidget->findNextFocusedWidget(Widget::FocusDirection::LEFT, _firstFocusedWidget);
                }
                if (keyCode == EventKeyboard::KeyCode::KEY_DPAD_RIGHT)
                {
                    _firstFocusedWidget = _firstFocusedWidget->findNextFocusedWidget(Widget::FocusDirection::RIGHT, _firstFocusedWidget);
                }
            }
        }

        void Widget::FocusNavigationController::enableFocusNavigation(bool flag)
        {
            if (_enableFocusNavigation == flag)
                return;

            _enableFocusNavigation = flag;

            if (flag)
                this->addKeyboardEventListener();
            else
                this->removeKeyboardEventListener();
        }

        void Widget::FocusNavigationController::setFirstFocsuedWidget(Widget* widget)
        {
            _firstFocusedWidget = widget;
        }

        void Widget::FocusNavigationController::addKeyboardEventListener()
        {
            if (nullptr == _keyboardListener)
            {
                _keyboardListener = EventListenerKeyboard::create();
                _keyboardListener->onKeyReleased = std::bind(&Widget::FocusNavigationController::onKeypadKeyPressed, this, std::placeholders::_1, std::placeholders::_2);
                EventDispatcher* dispatcher = Director::getInstance().getEventDispatcher();
                dispatcher->addEventListenerWithFixedPriority(_keyboardListener, _keyboardEventPriority);
            }
        }

        void Widget::FocusNavigationController::removeKeyboardEventListener()
        {
            if (nullptr != _keyboardListener)
            {
                EventDispatcher* dispatcher = Director::getInstance().getEventDispatcher();
                dispatcher->removeEventListener(_keyboardListener);
                _keyboardListener = nullptr;
            }
        }

        Widget* Widget::focusedWidget_ = nullptr;
        Widget::FocusNavigationController* Widget::focusNavigationController_ = nullptr;

        Widget::Widget():
        usingLayoutComponent_(false),
        unifySize_(false),
        enabled_(true),
        bright_(true),
        touchEnabled_(false),
        highlight_(false),
        affectByClipping_(false),
        ignoreSize_(false),
        propagateTouchEvents_(true),
        brightStyle_(BrightStyle::NONE),
        sizeType_(SizeType::ST_ABSOLUTE),
        positionType_(PositionType::PT_ABSOLUTE),
        actionTag_(0),
        customSize_(MATH::SizefZERO),
        hitted_(false),
        hittedByCamera_(nullptr),
        touchListener_(nullptr),
        flippedX_(false),
        flippedY_(false),
        layoutParameterType_(LayoutParameter::Type::TNONE),
        focused_(false),
        focusEnabled_(true),
        touchEventListener_(nullptr),
        touchEventSelector_(nullptr),
        EventCallback_(nullptr),
        callbackType_(""),
        callbackName_("")
        {

        }

        Widget::~Widget()
        {
            this->cleanupWidget();
        }

        void Widget::cleanupWidget()
        {
            //clean up _touchListener
            eventDispatcher_->removeEventListener(touchListener_);
            SAFE_RELEASE_NULL(touchListener_);

            //cleanup focused widget and focus navigation controller
            if (focusedWidget_ == this)
            {
                //delete
                SAFE_DELETE(focusNavigationController_);
                focusedWidget_ = nullptr;
            }

        }

        Widget* Widget::create()
        {
            Widget* widget = new (std::nothrow) Widget();
            if (widget && widget->init())
            {
                widget->autorelease();
                return widget;
            }
            SAFE_DELETE(widget);
            return nullptr;
        }

        bool Widget::init()
        {
            if (ProtectedNode::init())
            {
                initRenderer();
                setBright(true);
                onFocusChanged = std::bind(&Widget::onFocusChange,this, std::placeholders::_1, std::placeholders::_2);
                onNextFocusedWidget = nullptr;
                this->setAnchorPoint(MATH::Vector2f(0.5f, 0.5f));

                ignoreContentAdaptWithSize(true);

                return true;
            }
            return false;
        }

        void Widget::onEnter()
        {
            if (!usingLayoutComponent_)
                updateSizeAndPosition();
            ProtectedNode::onEnter();
        }

        void Widget::onExit()
        {
            unscheduleUpdate();
            ProtectedNode::onExit();
        }

        void Widget::visit(Renderer *renderer, const MATH::Matrix4 &parentTransform, uint32_t parentFlags)
        {
            if (visible_)
            {
                adaptRenderers();
                ProtectedNode::visit(renderer, parentTransform, parentFlags);
            }
        }

        Widget* Widget::getWidgetParent()
        {
            return dynamic_cast<Widget*>(getParent());
        }

        void Widget::setEnabled(bool enabled)
        {
            enabled_ = enabled;
        }

        void Widget::initRenderer()
        {
        }

        void Widget::setContentSize(const MATH::Sizef &contentSize)
        {
            ProtectedNode::setContentSize(contentSize);

            customSize_ = contentSize;
            if (unifySize_)
            {
                //unify Size logic
            }
            else if (ignoreSize_)
            {
                contentSize_ = getVirtualRendererSize();
            }
            if (!usingLayoutComponent_ && running_)
            {
                Widget* widgetParent = getWidgetParent();
                MATH::Sizef pSize;
                if (widgetParent)
                {
                    pSize = widgetParent->getContentSize();
                }
                else
                {
                    pSize = parent_->getContentSize();
                }
                float spx = 0.0f;
                float spy = 0.0f;
                if (pSize.width > 0.0f)
                {
                    spx = customSize_.width / pSize.width;
                }
                if (pSize.height > 0.0f)
                {
                    spy = customSize_.height / pSize.height;
                }
                sizePercent_.set(spx, spy);
            }
            onSizeChanged();
        }

        LayoutComponent* Widget::getOrCreateLayoutComponent()
        {
            auto layoutComponent = this->getComponent(__LAYOUT_COMPONENT_NAME);
            if (nullptr == layoutComponent)
            {
                LayoutComponent *component = LayoutComponent::create();
                this->addComponent(component);
                layoutComponent = component;
            }

            return (LayoutComponent*)layoutComponent;
        }

        void Widget::setSizePercent(const MATH::Vector2f &percent)
        {
            if (usingLayoutComponent_)
            {
                auto component = this->getOrCreateLayoutComponent();
                component->setUsingPercentContentSize(true);
                component->setPercentContentSize(percent);
                component->refreshLayout();
            }
            else
            {
                sizePercent_ = percent;
                MATH::Sizef cSize = customSize_;
                if (running_)
                {
                    Widget* widgetParent = getWidgetParent();
                    if (widgetParent)
                    {
                        cSize = MATH::Sizef(widgetParent->getContentSize().width * percent.x, widgetParent->getContentSize().height * percent.y);
                    }
                    else
                    {
                        cSize = MATH::Sizef(parent_->getContentSize().width * percent.x, parent_->getContentSize().height * percent.y);
                    }
                }
                if (ignoreSize_)
                {
                    this->setContentSize(getVirtualRendererSize());
                }
                else
                {
                    this->setContentSize(cSize);
                }
                customSize_ = cSize;
            }
        }

        void Widget::setSizeType(SizeType type)
        {
            sizeType_ = type;

            if (usingLayoutComponent_)
            {
                auto component = this->getOrCreateLayoutComponent();

                if (sizeType_ == Widget::SizeType::ST_PERCENT)
                {
                    component->setUsingPercentContentSize(true);
                }
                else
                {
                    component->setUsingPercentContentSize(false);
                }
            }
        }
        Widget::SizeType Widget::getSizeType() const
        {
            return sizeType_;
        }

        void Widget::updateSizeAndPosition()
        {
            MATH::Sizef pSize = parent_->getContentSize();

            updateSizeAndPosition(pSize);
        }

        void Widget::updateSizeAndPosition(const MATH::Sizef &parentSize)
        {
            switch (sizeType_)
            {
                case SizeType::ST_ABSOLUTE:
                {
                    if (ignoreSize_)
                    {
                        this->setContentSize(getVirtualRendererSize());
                    }
                    else
                    {
                        this->setContentSize(customSize_);
                    }
                    float spx = 0.0f;
                    float spy = 0.0f;
                    if (parentSize.width > 0.0f)
                    {
                        spx = customSize_.width / parentSize.width;
                    }
                    if (parentSize.height > 0.0f)
                    {
                        spy = customSize_.height / parentSize.height;
                    }
                    sizePercent_.set(spx, spy);
                    break;
                }
                case SizeType::ST_PERCENT:
                {
                    MATH::Sizef cSize = MATH::Sizef(parentSize.width * sizePercent_.x , parentSize.height * sizePercent_.y);
                    if (ignoreSize_)
                    {
                        this->setContentSize(getVirtualRendererSize());
                    }
                    else
                    {
                        this->setContentSize(cSize);
                    }
                    customSize_ = cSize;
                    break;
                }
                default:
                    break;
            }

            //update position & position percent
            MATH::Vector2f absPos = getPosition();
            switch (positionType_)
            {
                case PositionType::PT_ABSOLUTE:
                {
                    if (parentSize.width <= 0.0f || parentSize.height <= 0.0f)
                    {
                        positionPercent_.setZero();
                    }
                    else
                    {
                        positionPercent_.set(absPos.x / parentSize.width, absPos.y / parentSize.height);
                    }
                    break;
                }
                case PositionType::PT_PERCENT:
                {
                    absPos.set(parentSize.width * positionPercent_.x, parentSize.height * positionPercent_.y);
                    break;
                }
                default:
                    break;
            }
            setPosition(absPos);
        }

        void Widget::ignoreContentAdaptWithSize(bool ignore)
        {
            if (unifySize_)
            {
                this->setContentSize(customSize_);
                return;
            }
            if (ignoreSize_ == ignore)
            {
                return;
            }
            ignoreSize_ = ignore;
            if (ignoreSize_)
            {
                MATH::Sizef s = getVirtualRendererSize();
                this->setContentSize(s);
            }
            else
            {
                this->setContentSize(customSize_);
            }
        }

        bool Widget::isIgnoreContentAdaptWithSize() const
        {
            return ignoreSize_;
        }

        const MATH::Sizef& Widget::getCustomSize() const
        {
            return customSize_;
        }

        const MATH::Vector2f& Widget::getSizePercent()
        {
            if (usingLayoutComponent_)
            {
                auto component = this->getOrCreateLayoutComponent();
                sizePercent_ = component->getPercentContentSize();
            }

            return sizePercent_;
        }

        MATH::Vector2f Widget::getWorldPosition()const
        {
            return convertToWorldSpace(MATH::Vector2f(anchorPoint_.x * contentSize_.width, anchorPoint_.y * contentSize_.height));
        }

        Node* Widget::getVirtualRenderer()
        {
            return this;
        }

        void Widget::onSizeChanged()
        {
            if (!usingLayoutComponent_)
            {
                for (auto& child : getChildren())
                {
                    Widget* widgetChild = dynamic_cast<Widget*>(child);
                    if (widgetChild)
                    {
                        widgetChild->updateSizeAndPosition();
                    }
                }
            }
        }

        MATH::Sizef Widget::getVirtualRendererSize() const
        {
            return contentSize_;
        }

        void Widget::updateContentSizeWithTextureSize(const MATH::Sizef &size)
        {
            if (unifySize_)
            {
                this->setContentSize(size);
                return;
            }
            if (ignoreSize_)
            {
                this->setContentSize(size);
            }
            else
            {
                this->setContentSize(customSize_);
            }
        }

        void Widget::setTouchEnabled(bool enable)
        {
            if (enable == touchEnabled_)
            {
                return;
            }
            touchEnabled_ = enable;
            if (touchEnabled_)
            {
                touchListener_ = EventListenerTouchOneByOne::create();
                SAFE_RETAIN(touchListener_);
                touchListener_->setSwallowTouches(true);
                touchListener_->onTouchBegan = std::bind(&Widget::onTouchBegan, this, std::placeholders::_1, std::placeholders::_2);
                touchListener_->onTouchMoved = std::bind(&Widget::onTouchMoved, this, std::placeholders::_1, std::placeholders::_2);
                touchListener_->onTouchEnded = std::bind(&Widget::onTouchEnded, this, std::placeholders::_1, std::placeholders::_2);
                touchListener_->onTouchCancelled = std::bind(&Widget::onTouchCancelled, this, std::placeholders::_1, std::placeholders::_2);
                eventDispatcher_->addEventListenerWithSceneGraphPriority(touchListener_, this);
            }
            else
            {
                eventDispatcher_->removeEventListener(touchListener_);
                SAFE_RELEASE_NULL(touchListener_);
            }
        }

        bool Widget::isTouchEnabled() const
        {
            return touchEnabled_;
        }

        bool Widget::isHighlighted() const
        {
            return highlight_;
        }

        void Widget::setHighlighted(bool hilight)
        {
            if (hilight == highlight_)
            {
                return;
            }
            highlight_ = hilight;
            if (bright_)
            {
                if (highlight_)
                {
                    setBrightStyle(BrightStyle::HIGHLIGHT);
                }
                else
                {
                    setBrightStyle(BrightStyle::NORMAL);
                }
            }
            else
            {
                onPressStateChangedToDisabled();
            }
        }

        void Widget::setBright(bool bright)
        {
            bright_ = bright;
            if (bright_)
            {
                brightStyle_ = BrightStyle::NONE;
                setBrightStyle(BrightStyle::NORMAL);
            }
            else
            {
                onPressStateChangedToDisabled();
            }
        }

        void Widget::setBrightStyle(BrightStyle style)
        {
            if (brightStyle_ == style)
            {
                return;
            }
            brightStyle_ = style;
            switch (brightStyle_)
            {
                case BrightStyle::NORMAL:
                    onPressStateChangedToNormal();
                    break;
                case BrightStyle::HIGHLIGHT:
                    onPressStateChangedToPressed();
                    break;
                default:
                    break;
            }
        }

        void Widget::onPressStateChangedToNormal()
        {

        }

        void Widget::onPressStateChangedToPressed()
        {

        }

        void Widget::onPressStateChangedToDisabled()
        {

        }

        void Widget::updateChildrenDisplayedRGBA()
        {
            this->setColor(this->getColor());
            this->setOpacity(this->getOpacity());
        }


        Widget* Widget::getAncensterWidget(Node* node)
        {
            if (nullptr == node)
            {
                return nullptr;
            }

            Node* parent = node->getParent();
            if (nullptr == parent)
            {
                return nullptr;
            }
            Widget* parentWidget = dynamic_cast<Widget*>(parent);
            if (parentWidget)
            {
                return parentWidget;
            }
            else
            {
                return this->getAncensterWidget(parent->getParent());
            }
        }

        bool Widget::isAncestorsVisible(Node* node)
        {
            if (nullptr == node)
            {
                return true;
            }
            Node* parent = node->getParent();

            if (parent && !parent->isVisible())
            {
                return false;
            }
            return this->isAncestorsVisible(parent);
        }

        bool Widget::isAncestorsEnabled()
        {
            Widget* parentWidget = this->getAncensterWidget(this);
            if (parentWidget == nullptr)
            {
                return true;
            }
            if (parentWidget && !parentWidget->isEnabled())
            {
                return false;
            }

            return parentWidget->isAncestorsEnabled();
        }

        void Widget::setPropagateTouchEvents(bool isPropagate)
        {
            propagateTouchEvents_ = isPropagate;
        }

        bool Widget::isPropagateTouchEvents()const
        {
            return propagateTouchEvents_;
        }

        void Widget::setSwallowTouches(bool swallow)
        {
            if (touchListener_)
            {
                touchListener_->setSwallowTouches(swallow);
            }
        }

        bool Widget::isSwallowTouches()const
        {
            if (touchListener_)
            {
                return touchListener_->isSwallowTouches();
            }
            return false;
        }

        bool Widget::onTouchBegan(Touch *touch, Event *)
        {
            hitted_ = false;
            if (isVisible() && isEnabled() && isAncestorsEnabled() && isAncestorsVisible(this) )
            {
                touchBeganPosition_ = touch->getLocation();
                auto camera = Director::getInstance().getCamera();
                if(hitTest(touchBeganPosition_, camera, nullptr))
                {
                    hittedByCamera_ = camera;
                    if (isClippingParentContainsPoint(touchBeganPosition_)) {
                        hitted_ = true;
                    }
                }
            }
            if (!hitted_)
            {
                return false;
            }
            setHighlighted(true);

            /*
             * Propagate touch events to its parents
             */
            if (propagateTouchEvents_)
            {
                this->propagateTouchEvent(TouchEventType::BEGAN, this, touch);
            }

            pushDownEvent();
            return true;
        }

        void Widget::propagateTouchEvent(Widget::TouchEventType event, Widget *sender, Touch *touch)
        {
            Widget* widgetParent = getWidgetParent();
            if (widgetParent)
            {
                widgetParent->interceptTouchEvent(event, sender, touch);
            }
        }

        void Widget::onTouchMoved(Touch *touch, Event *)
        {
            touchMovePosition_ = touch->getLocation();

            setHighlighted(hitTest(touchMovePosition_, hittedByCamera_, nullptr));

            /*
             * Propagate touch events to its parents
             */
            if (propagateTouchEvents_)
            {
                this->propagateTouchEvent(TouchEventType::MOVED, this, touch);
            }

            moveEvent();
        }

        void Widget::onTouchEnded(Touch *touch, Event *)
        {
            touchEndPosition_ = touch->getLocation();

            /*
             * Propagate touch events to its parents
             */
            if (propagateTouchEvents_)
            {
                this->propagateTouchEvent(TouchEventType::ENDED, this, touch);
            }

            bool highlight = highlight_;
            setHighlighted(false);

            if (highlight)
            {
                releaseUpEvent();
            }
            else
            {
                cancelUpEvent();
            }
        }

        void Widget::onTouchCancelled(Touch *, Event *)
        {
            setHighlighted(false);
            cancelUpEvent();
        }

        void Widget::pushDownEvent()
        {
            this->retain();
            if (touchEventCallback_)
            {
                touchEventCallback_(this, TouchEventType::BEGAN);
            }

            if (touchEventListener_ && touchEventSelector_)
            {
                (touchEventListener_->*touchEventSelector_)(this,TOUCH_EVENT_BEGAN);
            }
            this->release();
        }

        void Widget::moveEvent()
        {
            this->retain();
            if (touchEventCallback_)
            {
                touchEventCallback_(this, TouchEventType::MOVED);
            }

            if (touchEventListener_ && touchEventSelector_)
            {
                (touchEventListener_->*touchEventSelector_)(this,TOUCH_EVENT_MOVED);
            }
            this->release();
        }

        void Widget::releaseUpEvent()
        {
            this->retain();
            if (touchEventCallback_)
            {
                touchEventCallback_(this, TouchEventType::ENDED);
            }

            if (touchEventListener_ && touchEventSelector_)
            {
                (touchEventListener_->*touchEventSelector_)(this,TOUCH_EVENT_ENDED);
            }

            if (clickEventListener_) {
                clickEventListener_(this);
            }
            this->release();
        }

        void Widget::cancelUpEvent()
        {
            this->retain();
            if (touchEventCallback_)
            {
                touchEventCallback_(this, TouchEventType::CANCELED);
            }

            if (touchEventListener_ && touchEventSelector_)
            {
                (touchEventListener_->*touchEventSelector_)(this,TOUCH_EVENT_CANCELED);
            }
            this->release();
        }

        void Widget::addTouchEventListener(const WidgetTouchCallback& callback)
        {
            this->touchEventCallback_ = callback;
        }

        void Widget::addClickEventListener(const WidgetClickCallback &callback)
        {
            this->clickEventListener_ = callback;
        }

        void Widget::addCCSEventListener(const WidgetEventCallback &callback)
        {
            this->EventCallback_ = callback;
        }

        bool Widget::hitTest(const MATH::Vector2f &pt, const Camera* camera, MATH::Vector3f *p) const
        {
            MATH::Rectf rect;
            rect.size = getContentSize();
            return Node::isScreenPointInRect(pt, camera, getWorldToNodeTransform(), rect, p);
        }

        bool Widget::isClippingParentContainsPoint(const MATH::Vector2f &pt)
        {
            affectByClipping_ = false;
            Widget* parent = getWidgetParent();
            Widget* clippingParent = nullptr;
            while (parent)
            {
                Layout* layoutParent = dynamic_cast<Layout*>(parent);
                if (layoutParent)
                {
                    if (layoutParent->isClippingEnabled())
                    {
                        affectByClipping_ = true;
                        clippingParent = layoutParent;
                        break;
                    }
                }
                parent = parent->getWidgetParent();
            }

            if (!affectByClipping_)
            {
                return true;
            }


            if (clippingParent)
            {
                bool bRet = false;
                if (clippingParent->hitTest(pt, hittedByCamera_, nullptr))
                {
                    bRet = true;
                }
                if (bRet)
                {
                    return clippingParent->isClippingParentContainsPoint(pt);
                }
                return false;
            }
            return true;
        }

        void Widget::interceptTouchEvent(Widget::TouchEventType event, Widget *sender, Touch *touch)
        {
            Widget* widgetParent = getWidgetParent();
            if (widgetParent)
            {
                widgetParent->interceptTouchEvent(event,sender,touch);
            }

        }

        void Widget::setPosition(const MATH::Vector2f &pos)
        {
            if (!usingLayoutComponent_ && running_)
            {
                Widget* widgetParent = getWidgetParent();
                if (widgetParent)
                {
                    MATH::Sizef pSize = widgetParent->getContentSize();
                    if (pSize.width <= 0.0f || pSize.height <= 0.0f)
                    {
                        positionPercent_.setZero();
                    }
                    else
                    {
                        positionPercent_.set(pos.x / pSize.width, pos.y / pSize.height);
                    }
                }
            }
            ProtectedNode::setPosition(pos);
        }

        void Widget::setPositionPercent(const MATH::Vector2f &percent)
        {
            if (usingLayoutComponent_)
            {
                auto component = this->getOrCreateLayoutComponent();
                component->setPositionPercentX(percent.x);
                component->setPositionPercentY(percent.y);
                component->refreshLayout();
            }
            else
            {
                positionPercent_ = percent;
                if (running_)
                {
                    Widget* widgetParent = getWidgetParent();
                    if (widgetParent)
                    {
                        MATH::Sizef parentSize = widgetParent->getContentSize();
                        MATH::Vector2f absPos(parentSize.width * positionPercent_.x, parentSize.height * positionPercent_.y);
                        setPosition(absPos);
                    }
                }
            }
        }

        const MATH::Vector2f& Widget::getPositionPercent(){
            if (usingLayoutComponent_)
            {
                auto component = this->getOrCreateLayoutComponent();
                float percentX = component->getPositionPercentX();
                float percentY = component->getPositionPercentY();

                positionPercent_.set(percentX, percentY);
            }
            return positionPercent_;
        }

        void Widget::setPositionType(PositionType type)
        {
            positionType_ = type;
            if (usingLayoutComponent_)
            {
                auto component = this->getOrCreateLayoutComponent();
                if (type == Widget::PositionType::PT_ABSOLUTE)
                {
                    component->setPositionPercentXEnabled(false);
                    component->setPositionPercentYEnabled(false);
                }
                else
                {
                    component->setPositionPercentXEnabled(true);
                    component->setPositionPercentYEnabled(true);
                }
            }
        }

        Widget::PositionType Widget::getPositionType() const
        {
            return positionType_;
        }

        bool Widget::isBright() const
        {
            return bright_;
        }

        bool Widget::isEnabled() const
        {
            return enabled_;
        }

        float Widget::getLeftBoundary() const
        {
            return getPosition().x - getAnchorPoint().x * contentSize_.width;
        }

        float Widget::getBottomBoundary() const
        {
            return getPosition().y - getAnchorPoint().y * contentSize_.height;
        }

        float Widget::getRightBoundary() const
        {
            return getLeftBoundary() + contentSize_.width;
        }

        float Widget::getTopBoundary() const
        {
            return getBottomBoundary() + contentSize_.height;
        }

        const MATH::Vector2f& Widget::getTouchBeganPosition()const
        {
            return touchBeganPosition_;
        }

        const MATH::Vector2f& Widget::getTouchMovePosition()const
        {
            return touchMovePosition_;
        }

        const MATH::Vector2f& Widget::getTouchEndPosition()const
        {
            return touchEndPosition_;
        }

        void Widget::setLayoutParameter(LayoutParameter *parameter)
        {
            if (!parameter)
            {
                return;
            }
            layoutParameterDictionary_.insert((int)parameter->getLayoutType(), parameter);
            layoutParameterType_ = parameter->getLayoutType();
        }

        LayoutParameter* Widget::getLayoutParameter()const
        {
            return dynamic_cast<LayoutParameter*>(layoutParameterDictionary_.at((int)layoutParameterType_));
        }

        Widget* Widget::clone()
        {
            Widget* clonedWidget = createCloneInstance();
            clonedWidget->copyProperties(this);
            clonedWidget->copyClonedWidgetChildren(this);
            return clonedWidget;
        }

        Widget* Widget::createCloneInstance()
        {
            return Widget::create();
        }

        void Widget::copyClonedWidgetChildren(Widget* model)
        {
            auto& modelChildren = model->getChildren();

            for (auto& subWidget : modelChildren)
            {
                Widget* child = dynamic_cast<Widget*>(subWidget);
                if (child)
                {
                    addChild(child->clone());
                }
            }
        }

        ShaderState* Widget::getNormalShaderState()const
        {
            ShaderState *glState = nullptr;
            glState = ShaderState::getOrCreateWithShaderName(Unity3DShader::SHADER_NAME_POSITION_TEXTURE_COLOR_NO_MVP);
            return glState;
        }

        ShaderState* Widget::getGrayShaderState()const
        {
            ShaderState *glState = nullptr;
            glState = ShaderState::getOrCreateWithShaderName(Unity3DShader::SHADER_NAME_POSITION_GRAYSCALE);
            return glState;
        }

        void Widget::copySpecialProperties(Widget*)
        {

        }

        void Widget::copyProperties(Widget *widget)
        {
            setEnabled(widget->isEnabled());
            setVisible(widget->isVisible());
            setBright(widget->isBright());
            setTouchEnabled(widget->isTouchEnabled());
            setLocalZOrder(widget->getLocalZOrder());
            setTag(widget->getTag());
            setName(widget->getName());
            setActionTag(widget->getActionTag());
            ignoreSize_ = widget->ignoreSize_;
            this->setContentSize(widget->contentSize_);
            customSize_ = widget->customSize_;
            sizeType_ = widget->getSizeType();
            sizePercent_ = widget->sizePercent_;
            positionType_ = widget->positionType_;
            positionPercent_ = widget->positionPercent_;
            setPosition(widget->getPosition());
            setAnchorPoint(widget->getAnchorPoint());
            setScaleX(widget->getScaleX());
            setScaleY(widget->getScaleY());
            setRotation(widget->getRotation());
            setRotationSkewX(widget->getRotationSkewX());
            setRotationSkewY(widget->getRotationSkewY());
            setFlippedX(widget->isFlippedX());
            setFlippedY(widget->isFlippedY());
            setColor(widget->getColor());
            setOpacity(widget->getOpacity());
            touchEventCallback_ = widget->touchEventCallback_;
            touchEventListener_ = widget->touchEventListener_;
            touchEventSelector_ = widget->touchEventSelector_;
            clickEventListener_ = widget->clickEventListener_;
            focused_ = widget->focused_;
            focusEnabled_ = widget->focusEnabled_;
            propagateTouchEvents_ = widget->propagateTouchEvents_;

            copySpecialProperties(widget);

            HObjectMap<int, LayoutParameter*>& layoutParameterDic = widget->layoutParameterDictionary_;
            for (auto iter = layoutParameterDic.begin(); iter != layoutParameterDic.end(); ++iter)
            {
                setLayoutParameter(iter->second->clone());
            }
        }

            void Widget::setFlippedX(bool flippedX)
            {

                float realScale = this->getScaleX();
                flippedX_ = flippedX;
                this->setScaleX(realScale);
            }

            void Widget::setFlippedY(bool flippedY)
            {
                float realScale = this->getScaleY();
                flippedY_ = flippedY;
                this->setScaleY(realScale);
            }



            void Widget::setScaleX(float scaleX)
            {
                if (flippedX_) {
                    scaleX = scaleX * -1;
                }
                Node::setScaleX(scaleX);
            }

            void Widget::setScaleY(float scaleY)
            {
                if (flippedY_) {
                    scaleY = scaleY * -1;
                }
                Node::setScaleY(scaleY);
            }

            void Widget::setScale(float scale)
            {
                this->setScaleX(scale);
                this->setScaleY(scale);
                this->setScaleZ(scale);
            }

            void Widget::setScale(float scaleX, float scaleY)
            {
                this->setScaleX(scaleX);
                this->setScaleY(scaleY);
            }

            float Widget::getScaleX()const
            {
                float originalScale = Node::getScaleX();
                if (flippedX_)
                {
                    originalScale = originalScale * -1.0;
                }
                return originalScale;
            }

            float Widget::getScaleY()const
            {
                float originalScale = Node::getScaleY();
                if (flippedY_)
                {
                    originalScale = originalScale * -1.0;
                }
                return originalScale;
            }

            float Widget::getScale()const
            {
                return this->getScaleX();
            }


        /*temp action*/
        void Widget::setActionTag(int tag)
        {
            actionTag_ = tag;
        }

        int Widget::getActionTag()const
        {
            return actionTag_;
        }

        void Widget::setFocused(bool focus)
        {
            focused_ = focus;

            //make sure there is only one focusedWidget
            if (focus) {
                focusedWidget_ = this;
                if (focusNavigationController_) {
                    focusNavigationController_->setFirstFocsuedWidget(this);
                }
            }

        }

        bool Widget::isFocused()const
        {
            return focused_;
        }

        void Widget::setFocusEnabled(bool enable)
        {
            focusEnabled_ = enable;
        }

        bool Widget::isFocusEnabled()const
        {
            return focusEnabled_;
        }

        Widget* Widget::findNextFocusedWidget(FocusDirection direction,  Widget* current)
        {
            if (nullptr == onNextFocusedWidget || nullptr == onNextFocusedWidget(direction) ) {
                if (this->isFocused() || dynamic_cast<Layout*>(current))
                {
                    Node* parent = this->getParent();

                    Layout* layout = dynamic_cast<Layout*>(parent);
                    if (nullptr == layout)
                    {
                        //the outer layout's default behaviour is : loop focus
                        if (dynamic_cast<Layout*>(current))
                        {
                            return current->findNextFocusedWidget(direction, current);
                        }
                        return current;
                    }
                    else
                    {
                        Widget *nextWidget = layout->findNextFocusedWidget(direction, current);
                        return nextWidget;
                    }
                }
                else
                {
                    return current;
                }
            }
            else
            {
                Widget *getFocusWidget = onNextFocusedWidget(direction);
                this->dispatchFocusEvent(this, getFocusWidget);
                return getFocusWidget;
            }
        }

        void Widget::dispatchFocusEvent(Widget *widgetLoseFocus, Widget *widgetGetFocus)
        {
            //if the widgetLoseFocus doesn't get focus, it will use the previous focused widget instead
            if (widgetLoseFocus && !widgetLoseFocus->isFocused())
            {
                widgetLoseFocus = focusedWidget_;
            }

            if (widgetGetFocus != widgetLoseFocus)
            {

                if (widgetGetFocus)
                {
                    widgetGetFocus->onFocusChanged(widgetLoseFocus, widgetGetFocus);
                }

                if (widgetLoseFocus)
                {
                    widgetLoseFocus->onFocusChanged(widgetLoseFocus, widgetGetFocus);
                }

                EventFocus event(widgetLoseFocus, widgetGetFocus);
                auto dispatcher = Director::getInstance().getEventDispatcher();
                dispatcher->dispatchEvent(&event);
            }

        }

        void Widget::requestFocus()
        {
            if (this == focusedWidget_)
            {
                return;
            }

            this->dispatchFocusEvent(focusedWidget_, this);
        }

        void Widget::onFocusChange(Widget* widgetLostFocus, Widget* widgetGetFocus)
        {
            //only change focus when there is indeed a get&lose happens
            if (widgetLostFocus)
            {
                widgetLostFocus->setFocused(false);
            }

            if (widgetGetFocus)
            {
                widgetGetFocus->setFocused(true);
            }
        }

        Widget* Widget::getCurrentFocusedWidget()const
        {
            return focusedWidget_;
        }

        void Widget::enableDpadNavigation(bool enable)
        {
            if (enable)
            {
                if (nullptr == focusNavigationController_)
                {
                    focusNavigationController_ = new (std::nothrow) FocusNavigationController;
                    if (focusedWidget_)
                    {
                        focusNavigationController_->setFirstFocsuedWidget(focusedWidget_);
                    }
                }
            }
            else
            {
                SAFE_DELETE(focusNavigationController_);
            }

            if (nullptr != focusNavigationController_)
            {
                focusNavigationController_->enableFocusNavigation(enable);
            }
        }


        bool Widget::isUnifySizeEnabled()const
        {
            return unifySize_;
        }

        void Widget::setUnifySizeEnabled(bool enable)
        {
            unifySize_ = enable;
        }

        void Widget::setLayoutComponentEnabled(bool enable)
        {
            usingLayoutComponent_ = enable;
        }

        bool Widget::isLayoutComponentEnabled()const
        {
            return usingLayoutComponent_;
        }
    }
}
