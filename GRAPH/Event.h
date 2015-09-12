#ifndef EVENT_H
#define EVENT_H

#include <vector>
#include "BASE/HObject.h"
#include "MATH/Vector.h"

namespace GRAPH
{
    class Event : public HObject
    {
    public:
        enum class Type
        {
            TOUCH,
            KEYBOARD,
            ACCELERATION,
            MOUSE,
            FOCUS,
            GAME_CONTROLLER,
            CUSTOM
        };

    public:
        Event(Type type);
        virtual ~Event();

        inline Type getType() const { return type_; }

        inline void stopPropagation() { isStopped_ = true; }

        inline bool isStopped() const { return isStopped_; }

        inline HObject* getCurrentTarget() { return currentTarget_; }

    protected:
        inline void setCurrentTarget(HObject* target) { currentTarget_ = target; }

    private:
        Type type_;
        bool isStopped_;
        HObject* currentTarget_;  ///< Current target

        friend class EventDispatcher;
    };

    class EventCustom : public Event
    {
    public:
        EventCustom(const std::string& eventName);

        inline void setUserData(void* data) { userData_ = data; }
        inline void* getUserData() const { return userData_; }

        inline const std::string& getEventName() const { return eventName_; }
    protected:
        void* userData_;       ///< User data
        std::string eventName_;
    };

    class Acceleration : public HObject
    {
    public:
        double x;
        double y;
        double z;

        double timestamp;

        Acceleration(): x(0), y(0), z(0), timestamp(0) {}
    };

    class EventAcceleration : public Event
    {
    public:
        EventAcceleration(const Acceleration& acc);

    private:
        Acceleration aaceleration_;

        friend class EventListenerAcceleration;
    };

    class EventKeyboard : public Event
    {
    public:
        enum class KeyCode
        {
            KEY_NONE,
            KEY_PAUSE,
            KEY_SCROLL_LOCK,
            KEY_PRINT,
            KEY_SYSREQ,
            KEY_BREAK,
            KEY_ESCAPE,
            KEY_BACK = KEY_ESCAPE,
            KEY_BACKSPACE,
            KEY_TAB,
            KEY_BACK_TAB,
            KEY_RETURN,
            KEY_CAPS_LOCK,
            KEY_SHIFT,
            KEY_LEFT_SHIFT = KEY_SHIFT,
            KEY_RIGHT_SHIFT,
            KEY_CTRL,
            KEY_LEFT_CTRL = KEY_CTRL,
            KEY_RIGHT_CTRL,
            KEY_ALT,
            KEY_LEFT_ALT = KEY_ALT,
            KEY_RIGHT_ALT,
            KEY_MENU,
            KEY_HYPER,
            KEY_INSERT,
            KEY_HOME,
            KEY_PG_UP,
            KEY_DELETE,
            KEY_END,
            KEY_PG_DOWN,
            KEY_LEFT_ARROW,
            KEY_RIGHT_ARROW,
            KEY_UP_ARROW,
            KEY_DOWN_ARROW,
            KEY_NUM_LOCK,
            KEY_KP_PLUS,
            KEY_KP_MINUS,
            KEY_KP_MULTIPLY,
            KEY_KP_DIVIDE,
            KEY_KP_ENTER,
            KEY_KP_HOME,
            KEY_KP_UP,
            KEY_KP_PG_UP,
            KEY_KP_LEFT,
            KEY_KP_FIVE,
            KEY_KP_RIGHT,
            KEY_KP_END,
            KEY_KP_DOWN,
            KEY_KP_PG_DOWN,
            KEY_KP_INSERT,
            KEY_KP_DELETE,
            KEY_F1,
            KEY_F2,
            KEY_F3,
            KEY_F4,
            KEY_F5,
            KEY_F6,
            KEY_F7,
            KEY_F8,
            KEY_F9,
            KEY_F10,
            KEY_F11,
            KEY_F12,
            KEY_SPACE,
            KEY_EXCLAM,
            KEY_QUOTE,
            KEY_NUMBER,
            KEY_DOLLAR,
            KEY_PERCENT,
            KEY_CIRCUMFLEX,
            KEY_AMPERSAND,
            KEY_APOSTROPHE,
            KEY_LEFT_PARENTHESIS,
            KEY_RIGHT_PARENTHESIS,
            KEY_ASTERISK,
            KEY_PLUS,
            KEY_COMMA,
            KEY_MINUS,
            KEY_PERIOD,
            KEY_SLASH,
            KEY_0,
            KEY_1,
            KEY_2,
            KEY_3,
            KEY_4,
            KEY_5,
            KEY_6,
            KEY_7,
            KEY_8,
            KEY_9,
            KEY_COLON,
            KEY_SEMICOLON,
            KEY_LESS_THAN,
            KEY_EQUAL,
            KEY_GREATER_THAN,
            KEY_QUESTION,
            KEY_AT,
            KEY_CAPITAL_A,
            KEY_CAPITAL_B,
            KEY_CAPITAL_C,
            KEY_CAPITAL_D,
            KEY_CAPITAL_E,
            KEY_CAPITAL_F,
            KEY_CAPITAL_G,
            KEY_CAPITAL_H,
            KEY_CAPITAL_I,
            KEY_CAPITAL_J,
            KEY_CAPITAL_K,
            KEY_CAPITAL_L,
            KEY_CAPITAL_M,
            KEY_CAPITAL_N,
            KEY_CAPITAL_O,
            KEY_CAPITAL_P,
            KEY_CAPITAL_Q,
            KEY_CAPITAL_R,
            KEY_CAPITAL_S,
            KEY_CAPITAL_T,
            KEY_CAPITAL_U,
            KEY_CAPITAL_V,
            KEY_CAPITAL_W,
            KEY_CAPITALxPosition_,
            KEY_CAPITAL_Y,
            KEY_CAPITAL_Z,
            KEY_LEFT_BRACKET,
            KEY_BACK_SLASH,
            KEY_RIGHT_BRACKET,
            KEY_UNDERSCORE,
            KEY_GRAVE,
            KEY_A,
            KEY_B,
            KEY_C,
            KEY_D,
            KEY_E,
            KEY_F,
            KEY_G,
            KEY_H,
            KEY_I,
            KEY_J,
            KEY_K,
            KEY_L,
            KEY_M,
            KEY_N,
            KEY_O,
            KEY_P,
            KEY_Q,
            KEY_R,
            KEY_S,
            KEY_T,
            KEY_U,
            KEY_V,
            KEY_W,
            KEY_X,
            KEY_Y,
            KEY_Z,
            KEY_LEFT_BRACE,
            KEY_BAR,
            KEY_RIGHT_BRACE,
            KEY_TILDE,
            KEY_EURO,
            KEY_POUND,
            KEY_YEN,
            KEY_MIDDLE_DOT,
            KEY_SEARCH,
            KEY_DPAD_LEFT,
            KEY_DPAD_RIGHT,
            KEY_DPAD_UP,
            KEY_DPAD_DOWN,
            KEY_DPAD_CENTER,
            KEY_ENTER,
            KEY_PLAY
        };

        EventKeyboard(KeyCode keyCode, bool isPressed);

    private:
        KeyCode keyCode_;
        bool isPressed_;

        friend class EventListenerKeyboard;
    };

    class EventMouse : public Event
    {
    public:
        enum class MouseEventType
        {
            MOUSE_NONE,
            MOUSE_DOWN,
            MOUSE_UP,
            MOUSE_MOVE,
            MOUSE_SCROLL,
        };

        EventMouse(MouseEventType mouseEventCode);

        inline void setScrollData(float scrollX, float scrollY) { scrollX_ = scrollX; scrollY_ = scrollY; }
        inline float getScrollX() const { return scrollX_; }
        inline float getScrollY() const { return scrollY_; }
        inline void setCursorPosition(float x, float y) {
            xPosition_ = x;
            yPosition_ = y;
            prevPoint_ = point_;
            point_.x = x;
            point_.y = y;
            if (!startPointCaptured_)
            {
                startPoint_ = point_;
                startPointCaptured_ = true;
            }
        }

        inline void setMouseButton(int button) { mouseButton_ = button; }
        inline int getMouseButton() const { return mouseButton_; }
        inline float getCursorX() const { return xPosition_; }
        inline float getCursorY() const { return yPosition_; }

        MATH::Vector2f getLocation() const;
        MATH::Vector2f getPreviousLocation() const;
        MATH::Vector2f getStartLocation() const;
        MATH::Vector2f getDelta() const;
        MATH::Vector2f getLocationInView() const;
        MATH::Vector2f getPreviousLocationInView() const;
        MATH::Vector2f getStartLocationInView() const;

    private:
        MouseEventType mouseEventType_;
        int mouseButton_;
        float xPosition_;
        float yPosition_;
        float scrollX_;
        float scrollY_;

        bool startPointCaptured_;
        MATH::Vector2f startPoint_;
        MATH::Vector2f point_;
        MATH::Vector2f prevPoint_;

        friend class EventListenerMouse;
    };

    class EventFocus : public Event
    {
    public:
        EventFocus(HObject* widgetLoseFocus, HObject* widgetGetFocus);

    private:
        HObject *widgetGetFocus_;
        HObject *widgetLoseFocus_;

        friend class EventListenerFocus;
    };

    class Touch : public HObject
    {
    public:
        enum class DispatchMode {
            ALL_AT_ONCE, /** All at once. */
            ONE_BY_ONE,  /** One by one. */
        };

        Touch()
            : touchID_(0)
            , startPointCaptured_(false)
        {}

        MATH::Vector2f getLocation() const;
        MATH::Vector2f getPreviousLocation() const;
        MATH::Vector2f getStartLocation() const;
        MATH::Vector2f getDelta() const;
        MATH::Vector2f getLocationInView() const;
        MATH::Vector2f getPreviousLocationInView() const;
        MATH::Vector2f getStartLocationInView() const;

        void setTouchInfo(int id, float x, float y)
        {
            touchID_ = id;
            prevPoint_ = point_;
            point_.x   = x;
            point_.y   = y;
            if (!startPointCaptured_)
            {
                startPoint_ = point_;
                startPointCaptured_ = true;
                prevPoint_ = point_;
            }
        }

        int getID() const
        {
            return touchID_;
        }

    private:
        int touchID_;
        bool startPointCaptured_;
        MATH::Vector2f startPoint_;
        MATH::Vector2f point_;
        MATH::Vector2f prevPoint_;
    };

    class EventTouch : public Event
    {
    public:
        static const int MAXtouches_ = 15;

        enum class EventCode
        {
            BEGAN,
            MOVED,
            ENDED,
            CANCELLED
        };

        EventTouch();

        inline EventCode getEventCode() const { return eventCode_; }
        inline const std::vector<Touch*>& getTouches() const { return touches_; }

    private:
        EventCode eventCode_;
        std::vector<Touch*> touches_;
    };
}

#endif // EVENT_H
