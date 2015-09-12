#include "GRAPH/Event.h"
#include "GRAPH/Director.h"

namespace GRAPH
{
    Event::Event(Type type)
        : type_(type)
        , isStopped_(false)
        , currentTarget_(nullptr) {
    }

    Event::~Event() {
    }

    EventCustom::EventCustom(const std::string& eventName)
        : Event(Type::CUSTOM)
        , userData_(nullptr)
        , eventName_(eventName) {
    }

    EventAcceleration::EventAcceleration(const Acceleration& acc)
        : Event(Type::ACCELERATION)
        , aaceleration_(acc) {
    }

    EventKeyboard::EventKeyboard(KeyCode keyCode, bool isPressed)
        : Event(Type::KEYBOARD)
        , keyCode_(keyCode)
        , isPressed_(isPressed) {

    }

    EventMouse::EventMouse(MouseEventType mouseEventCode)
        : Event(Type::MOUSE)
        , mouseEventType_(mouseEventCode)
        , mouseButton_(-1)
        , xPosition_(0.0f)
        , yPosition_(0.0f)
        , scrollX_(0.0f)
        , scrollY_(0.0f)
        , startPointCaptured_(false) {

    }

    MATH::Vector2f EventMouse::getLocationInView() const {
        return point_;
    }

    MATH::Vector2f EventMouse::getPreviousLocationInView() const {
        return prevPoint_;
    }

    MATH::Vector2f EventMouse::getStartLocationInView() const {
        return startPoint_;
    }

    MATH::Vector2f EventMouse::getLocation() const {
        return Director::getInstance().convertToGL(point_);
    }

    MATH::Vector2f EventMouse::getPreviousLocation() const {
        return Director::getInstance().convertToGL(prevPoint_);
    }

    MATH::Vector2f EventMouse::getStartLocation() const {
        return Director::getInstance().convertToGL(startPoint_);
    }

    MATH::Vector2f EventMouse::getDelta() const {
        return getLocation() - getPreviousLocation();
    }

    EventFocus::EventFocus(HObject *widgetLoseFocus, HObject* widgetGetFocus)
        :Event(Type::FOCUS),
        widgetGetFocus_(widgetGetFocus),
        widgetLoseFocus_(widgetLoseFocus) {

    }

    MATH::Vector2f Touch::getLocationInView() const {
        return point_;
    }

    MATH::Vector2f Touch::getPreviousLocationInView() const {
        return prevPoint_;
    }

    MATH::Vector2f Touch::getStartLocationInView() const {
        return startPoint_;
    }

    MATH::Vector2f Touch::getLocation() const {
        return Director::getInstance().convertToGL(point_);
    }

    MATH::Vector2f Touch::getPreviousLocation() const {
        return Director::getInstance().convertToGL(prevPoint_);
    }

    MATH::Vector2f Touch::getStartLocation() const {
        return Director::getInstance().convertToGL(startPoint_);
    }

    MATH::Vector2f Touch::getDelta() const {
        return getLocation() - getPreviousLocation();
    }

    EventTouch::EventTouch()
        : Event(Type::TOUCH) {
        touches_.reserve(MAXtouches_);
    }
}
