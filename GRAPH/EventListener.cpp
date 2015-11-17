#include "GRAPH/EventListener.h"

namespace GRAPH
{
    EventListener::EventListener() {

    }

    EventListener::~EventListener() {
    }

    bool EventListener::init(Type t, const ListenerID& listenerID, const std::function<void(Event*)>& callback) {
        _onEvent = callback;
        type_ = t;
        listenerID_ = listenerID;
        registered_ = false;
        paused_ = true;
        enabled_ = true;

        return true;
    }

    bool EventListener::checkAvailable() {
        return (_onEvent != nullptr);
    }

    EventListenerCustom::EventListenerCustom()
        : onCustomEvent(nullptr)
    {
    }

    EventListenerCustom* EventListenerCustom::create(const std::string& eventName, const std::function<void(EventCustom*)>& callback) {
        EventListenerCustom* ret = new (std::nothrow) EventListenerCustom();
        if (ret && ret->init(eventName, callback)) {
            ret->autorelease();
        }
        else {
            SAFE_DELETE(ret);
        }
        return ret;
    }

    bool EventListenerCustom::init(const ListenerID& listenerId, const std::function<void(EventCustom*)>& callback) {
        bool ret = false;

        onCustomEvent = callback;

        auto listener = [this](Event* event){
            if (onCustomEvent != nullptr) {
                onCustomEvent(static_cast<EventCustom*>(event));
            }
        };

        if (EventListener::init(EventListener::Type::CUSTOM, listenerId, listener)) {
            ret = true;
        }
        return ret;
    }

    EventListenerCustom* EventListenerCustom::clone() {
        EventListenerCustom* ret = new (std::nothrow) EventListenerCustom();
        if (ret && ret->init(listenerID_, onCustomEvent)) {
            ret->autorelease();
        }
        else {
            SAFE_DELETE(ret);
        }
        return ret;
    }

    bool EventListenerCustom::checkAvailable() {
        bool ret = false;
        if (EventListener::checkAvailable() && onCustomEvent != nullptr) {
            ret = true;
        }
        return ret;
    }

    const std::string EventListenerAcceleration::LISTENER_ID = "_aaceleration_eleration";

    EventListenerAcceleration::EventListenerAcceleration() {

    }

    EventListenerAcceleration::~EventListenerAcceleration() {
    }

    EventListenerAcceleration* EventListenerAcceleration::create(const std::function<void(Acceleration*, Event*)>& callback) {
        EventListenerAcceleration* ret = new (std::nothrow) EventListenerAcceleration();
        if (ret && ret->init(callback)) {
            ret->autorelease();
        }
        else {
            SAFE_DELETE(ret);
        }

        return ret;
    }

    bool EventListenerAcceleration::init(const std::function<void(Acceleration*, Event* event)>& callback) {
        auto listener = [this](Event* event){
            auto accEvent = static_cast<EventAcceleration*>(event);
            this->onAccelerationEvent(&accEvent->aaceleration_, event);
        };

        if (EventListener::init(Type::ACCELERATION, LISTENER_ID, listener)) {
            onAccelerationEvent = callback;
            return true;
        }

        return false;
    }

    EventListenerAcceleration* EventListenerAcceleration::clone() {
        auto ret = new (std::nothrow) EventListenerAcceleration();

        if (ret && ret->init(onAccelerationEvent)) {
            ret->autorelease();
        }
        else {
            SAFE_DELETE(ret);
        }

        return ret;
    }

    bool EventListenerAcceleration::checkAvailable() {
        return true;
    }

    const std::string EventListenerKeyboard::LISTENER_ID = "__keyboard";

    bool EventListenerKeyboard::checkAvailable() {
        if (onKeyPressed == nullptr && onKeyReleased == nullptr) {
            return false;
        }

        return true;
    }

    EventListenerKeyboard* EventListenerKeyboard::create() {
        auto ret = new (std::nothrow) EventListenerKeyboard();
        if (ret && ret->init()) {
            ret->autorelease();
        }
        else {
            SAFE_DELETE(ret);
        }
        return ret;
    }

    EventListenerKeyboard* EventListenerKeyboard::clone() {
        auto ret = new (std::nothrow) EventListenerKeyboard();
        if (ret && ret->init()) {
            ret->autorelease();
            ret->onKeyPressed = onKeyPressed;
            ret->onKeyReleased = onKeyReleased;
        }
        else {
            SAFE_DELETE(ret);
        }
        return ret;
    }

    EventListenerKeyboard::EventListenerKeyboard()
    : onKeyPressed(nullptr)
    , onKeyReleased(nullptr) {
    }

    bool EventListenerKeyboard::init() {
        auto listener = [this](Event* event){
            auto keyboardEvent = static_cast<EventKeyboard*>(event);
            if (keyboardEvent->isPressed_) {
                if (onKeyPressed != nullptr)
                    onKeyPressed(keyboardEvent->keyCode_, event);
            }
            else {
                if (onKeyReleased != nullptr)
                    onKeyReleased(keyboardEvent->keyCode_, event);
            }
        };

        if (EventListener::init(Type::KEYBOARD, LISTENER_ID, listener)) {
            return true;
        }

        return false;
    }

    const std::string EventListenerMouse::LISTENER_ID = "__mouse";

    bool EventListenerMouse::checkAvailable() {
        return true;
    }

    EventListenerMouse* EventListenerMouse::create() {
        auto ret = new (std::nothrow) EventListenerMouse();
        if (ret && ret->init()) {
            ret->autorelease();
        }
        else {
            SAFE_DELETE(ret);
        }
        return ret;
    }

    EventListenerMouse* EventListenerMouse::clone() {
        auto ret = new (std::nothrow) EventListenerMouse();
        if (ret && ret->init()) {
            ret->autorelease();
            ret->onMouseUp = onMouseUp;
            ret->onMouseDown = onMouseDown;
            ret->onMouseMove = onMouseMove;
            ret->onMouseScroll = onMouseScroll;
        }
        else {
            SAFE_DELETE(ret);
        }
        return ret;
    }

    EventListenerMouse::EventListenerMouse()
        : onMouseDown(nullptr)
        , onMouseUp(nullptr)
        , onMouseMove(nullptr)
        , onMouseScroll(nullptr) {
    }

    bool EventListenerMouse::init() {
        auto listener = [this](Event* event){
            auto mouseEvent = static_cast<EventMouse*>(event);
            switch (mouseEvent->mouseEventType_) {
                case EventMouse::MouseEventType::MOUSE_DOWN:
                    if(onMouseDown != nullptr)
                        onMouseDown(event);
                    break;
                case EventMouse::MouseEventType::MOUSE_UP:
                    if(onMouseUp != nullptr)
                        onMouseUp(event);
                    break;
                case EventMouse::MouseEventType::MOUSE_MOVE:
                    if(onMouseMove != nullptr)
                        onMouseMove(event);
                    break;
                case EventMouse::MouseEventType::MOUSE_SCROLL:
                    if(onMouseScroll != nullptr)
                        onMouseScroll(event);
                    break;
                default:
                    break;
            }
        };

        if (EventListener::init(Type::MOUSE, LISTENER_ID, listener)) {
            return true;
        }

        return false;
    }

    const std::string EventListenerFocus::LISTENER_ID = "__focus_event";

    EventListenerFocus::EventListenerFocus()
        :onFocusChanged(nullptr) {

    }

    EventListenerFocus::~EventListenerFocus() {
    }

    EventListenerFocus* EventListenerFocus::create() {
        EventListenerFocus* ret = new (std::nothrow) EventListenerFocus;
        if (ret && ret->init()) {
            ret->autorelease();
            return ret;
        }
        SAFE_DELETE(ret);
        return nullptr;
    }

    EventListenerFocus* EventListenerFocus::clone() {
        EventListenerFocus* ret = new (std::nothrow) EventListenerFocus;
        if (ret && ret->init()) {
            ret->autorelease();

            ret->onFocusChanged = onFocusChanged;
        }
        else {
            SAFE_DELETE(ret);
        }
        return ret;
    }

    bool EventListenerFocus::init() {
        auto listener = [this](Event* event){
            auto focusEvent = static_cast<EventFocus*>(event);
            onFocusChanged(focusEvent->widgetLoseFocus_, focusEvent->widgetGetFocus_);
        };
        if (EventListener::init(Type::FOCUS, LISTENER_ID, listener)) {
            return true;
        }
        return false;
    }

    bool EventListenerFocus::checkAvailable() {
        if (onFocusChanged == nullptr) {
            return false;
        }

        return true;
    }

    const std::string EventListenerTouchOneByOne::LISTENER_ID = "__touch_one_by_one";

    EventListenerTouchOneByOne::EventListenerTouchOneByOne()
        : onTouchBegan(nullptr)
        , onTouchMoved(nullptr)
        , onTouchEnded(nullptr)
        , onTouchCancelled(nullptr)
        , needSwallow_(false) {
    }

    EventListenerTouchOneByOne::~EventListenerTouchOneByOne() {
    }

    bool EventListenerTouchOneByOne::init() {
        if (EventListener::init(Type::TOUCH_ONE_BY_ONE, LISTENER_ID, nullptr)) {
            return true;
        }

        return false;
    }

    void EventListenerTouchOneByOne::setSwallowTouches(bool needSwallow) {
        needSwallow_ = needSwallow;
    }

    bool EventListenerTouchOneByOne::isSwallowTouches() {
        return needSwallow_;
    }

    EventListenerTouchOneByOne* EventListenerTouchOneByOne::create() {
        auto ret = new (std::nothrow) EventListenerTouchOneByOne();
        if (ret && ret->init()) {
            ret->autorelease();
        }
        else {
            SAFE_DELETE(ret);
        }
        return ret;
    }

    bool EventListenerTouchOneByOne::checkAvailable() {
        // EventDispatcher will use the return value of 'onTouchBegan' to determine whether to pass following 'move', 'end'
        // message to 'EventListenerTouchOneByOne' or not. So 'onTouchBegan' needs to be set.
        if (onTouchBegan == nullptr) {
            return false;
        }

        return true;
    }

    EventListenerTouchOneByOne* EventListenerTouchOneByOne::clone() {
        auto ret = new (std::nothrow) EventListenerTouchOneByOne();
        if (ret && ret->init()) {
            ret->autorelease();

            ret->onTouchBegan = onTouchBegan;
            ret->onTouchMoved = onTouchMoved;
            ret->onTouchEnded = onTouchEnded;
            ret->onTouchCancelled = onTouchCancelled;

            ret->claimedTouches_ = claimedTouches_;
            ret->needSwallow_ = needSwallow_;
        }
        else {
            SAFE_DELETE(ret);
        }
        return ret;
    }

    const std::string EventListenerTouchAllAtOnce::LISTENER_ID = "__touch_all_at_once";

    EventListenerTouchAllAtOnce::EventListenerTouchAllAtOnce()
        : onTouchesBegan(nullptr)
        , onTouchesMoved(nullptr)
        , onTouchesEnded(nullptr)
        , onTouchesCancelled(nullptr) {
    }

    EventListenerTouchAllAtOnce::~EventListenerTouchAllAtOnce() {
    }

    bool EventListenerTouchAllAtOnce::init() {
        if (EventListener::init(Type::TOUCH_ALL_AT_ONCE, LISTENER_ID, nullptr)) {
            return true;
        }

        return false;
    }

    EventListenerTouchAllAtOnce* EventListenerTouchAllAtOnce::create() {
        auto ret = new (std::nothrow) EventListenerTouchAllAtOnce();
        if (ret && ret->init()) {
            ret->autorelease();
        }
        else {
            SAFE_DELETE(ret);
        }
        return ret;
    }

    bool EventListenerTouchAllAtOnce::checkAvailable() {
        if (onTouchesBegan == nullptr && onTouchesMoved == nullptr
            && onTouchesEnded == nullptr && onTouchesCancelled == nullptr) {
            return false;
        }

        return true;
    }

    EventListenerTouchAllAtOnce* EventListenerTouchAllAtOnce::clone() {
        auto ret = new (std::nothrow) EventListenerTouchAllAtOnce();
        if (ret && ret->init()) {
            ret->autorelease();

            ret->onTouchesBegan = onTouchesBegan;
            ret->onTouchesMoved = onTouchesMoved;
            ret->onTouchesEnded = onTouchesEnded;
            ret->onTouchesCancelled = onTouchesCancelled;
        }
        else {
            SAFE_DELETE(ret);
        }
        return ret;
    }
}
