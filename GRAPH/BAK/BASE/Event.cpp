#include "GRAPH/BASE/Event.h"
#include "GRAPH/BASE/Director.h"

namespace GRAPH
{
    Event::Event(Type type)
    : _type(type)
    , _isStopped(false)
    , _currentTarget(nullptr)
    {
    }

    Event::~Event()
    {
    }

    EventCustom::EventCustom(const std::string& eventName)
        : Event(Type::CUSTOM)
        , _userData(nullptr)
        , _eventName(eventName) {
    }

    EventAcceleration::EventAcceleration(const Acceleration& acc)
        : Event(Type::ACCELERATION)
        , _acc(acc) {
    }

    EventKeyboard::EventKeyboard(KeyCode keyCode, bool isPressed)
        : Event(Type::KEYBOARD)
        , _keyCode(keyCode)
        , _isPressed(isPressed) {

    }

    EventMouse::EventMouse(MouseEventType mouseEventCode)
        : Event(Type::MOUSE)
        , _mouseEventType(mouseEventCode)
        , _mouseButton(-1)
        , _x(0.0f)
        , _y(0.0f)
        , _scrollX(0.0f)
        , _scrollY(0.0f)
        , _startPointCaptured(false) {
    }

    MATH::Vector2f EventMouse::getLocationInView() const {
        return _point;
    }

    MATH::Vector2f EventMouse::getPreviousLocationInView() const {
        return _prevPoint;
    }

    MATH::Vector2f EventMouse::getStartLocationInView() const {
        return _startPoint;
    }

    MATH::Vector2f EventMouse::getLocation() const {
        return Director::getInstance()->convertToGL(_point);
    }

    MATH::Vector2f EventMouse::getPreviousLocation() const {
        return Director::getInstance()->convertToGL(_prevPoint);
    }

    MATH::Vector2f EventMouse::getStartLocation() const {
        return Director::getInstance()->convertToGL(_startPoint);
    }

    MATH::Vector2f EventMouse::getDelta() const {
        return getLocation() - getPreviousLocation();
    }

    EventFocus::EventFocus(Node *widgetLoseFocus, Node* widgetGetFocus)
        :Event(Type::FOCUS),
        _widgetGetFocus(widgetGetFocus),
        _widgetLoseFocus(widgetLoseFocus) {

    }

    MATH::Vector2f Touch::getLocationInView() const {
        return _point;
    }

    MATH::Vector2f Touch::getPreviousLocationInView() const {
        return _prevPoint;
    }

    MATH::Vector2f Touch::getStartLocationInView() const {
        return _startPoint;
    }

    MATH::Vector2f Touch::getLocation() const {
        return Director::getInstance()->convertToGL(_point);
    }

    MATH::Vector2f Touch::getPreviousLocation() const {
        return Director::getInstance()->convertToGL(_prevPoint);
    }

    MATH::Vector2f Touch::getStartLocation() const {
        return Director::getInstance()->convertToGL(_startPoint);
    }

    MATH::Vector2f Touch::getDelta() const {
        return getLocation() - getPreviousLocation();
    }

    EventTouch::EventTouch()
        : Event(Type::TOUCH) {
        _touches.reserve(MAX_TOUCHES);
    }
}
