#ifndef EVENTLISTENER_H
#define EVENTLISTENER_H

#include <functional>
#include <string>
#include <memory>
#include <vector>
#include "BASE/HObject.h"
#include "GRAPH/BASE/Event.h"
#include "MATH/Vector.h"

namespace GRAPH
{
    class Event;
    class Node;

    class EventListener : public HObject
    {
    public:
        enum class Type
        {
            UNKNOWN,
            TOUCH_ONE_BY_ONE,
            TOUCH_ALL_AT_ONCE,
            KEYBOARD,
            MOUSE,
            ACCELERATION,
            FOCUS,
            GAME_CONTROLLER,
            CUSTOM
        };

        typedef std::string ListenerID;

    public:
        EventListener();
        bool init(Type t, const ListenerID& listenerID, const std::function<void(Event*)>& callback);

    public:
        virtual ~EventListener();

        virtual bool checkAvailable() = 0;

        virtual EventListener* clone() = 0;

        inline void setEnabled(bool enabled) { _isEnabled = enabled; }
        inline bool isEnabled() const { return _isEnabled; }

    protected:
        inline void setPaused(bool paused) { _paused = paused; }

        inline bool isPaused() const { return _paused; }

        inline void setRegistered(bool registered) { _isRegistered = registered; }
        inline bool isRegistered() const { return _isRegistered; }

        inline Type getType() const { return _type; }
        inline const ListenerID& getListenerID() const { return _listenerID; }


        inline void setFixedPriority(int fixedPriority) { _fixedPriority = fixedPriority; }
        inline int getFixedPriority() const { return _fixedPriority; }

        inline void setAssociatedNode(Node* node) { _node = node; }
        inline Node* getAssociatedNode() const { return _node; }

        std::function<void(Event*)> _onEvent;   /// Event callback function

        Type _type;                             /// Event listener type
        ListenerID _listenerID;                 /// Event listener ID
        bool _isRegistered;                     /// Whether the listener has been added to dispatcher.

        int   _fixedPriority;   // The higher the number, the higher the priority, 0 is for scene graph base priority.
        Node* _node;            // scene graph based priority
        bool _paused;           // Whether the listener is paused
        bool _isEnabled;        // Whether the listener is enabled
        friend class EventDispatcher;
    };

    class EventListenerCustom : public EventListener
    {
    public:
        static EventListenerCustom* create(const std::string& eventName, const std::function<void(EventCustom*)>& callback);

        virtual bool checkAvailable() override;
        virtual EventListenerCustom* clone() override;

    public:
        EventListenerCustom();
        bool init(const ListenerID& listenerId, const std::function<void(EventCustom*)>& callback);

    protected:
        std::function<void(EventCustom*)> _onCustomEvent;
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
        Acceleration _acc;
        friend class EventListenerAcceleration;
    };

    class EventListenerAcceleration : public EventListener
    {
    public:
        static const std::string LISTENER_ID;

        static EventListenerAcceleration* create(const std::function<void(Acceleration*, Event*)>& callback);

        virtual ~EventListenerAcceleration();

        virtual EventListenerAcceleration* clone() override;
        virtual bool checkAvailable() override;

    public:
        EventListenerAcceleration();
        bool init(const std::function<void(Acceleration*, Event* event)>& callback);

    private:
        std::function<void(Acceleration*, Event*)> onAccelerationEvent;
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
            KEY_CAPITAL_X,
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
        KeyCode _keyCode;
        bool _isPressed;

        friend class EventListenerKeyboard;
    };

    class EventListenerKeyboard : public EventListener
    {
    public:
        static const std::string LISTENER_ID;

        /** Create a keyboard event listener.
         *
         * @return An autoreleased EventListenerKeyboard object.
         */
        static EventListenerKeyboard* create();

        /// Overrides
        virtual EventListenerKeyboard* clone() override;
        virtual bool checkAvailable() override;

        std::function<void(EventKeyboard::KeyCode, Event*)> onKeyPressed;
        std::function<void(EventKeyboard::KeyCode, Event*)> onKeyReleased;
    public:
        EventListenerKeyboard();
        bool init();
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

        inline void setScrollData(float scrollX, float scrollY) { _scrollX = scrollX; _scrollY = scrollY; }
        inline float getScrollX() const { return _scrollX; }
        inline float getScrollY() const { return _scrollY; }
        inline void setCursorPosition(float x, float y) {
            _x = x;
            _y = y;
            _prevPoint = _point;
            _point.x = x;
            _point.y = y;
            if (!_startPointCaptured)
            {
                _startPoint = _point;
                _startPointCaptured = true;
            }
        }

        inline void setMouseButton(int button) { _mouseButton = button; }
        inline int getMouseButton() const { return _mouseButton; }
        inline float getCursorX() const { return _x; }
        inline float getCursorY() const { return _y; }

        MATH::Vector2f getLocation() const;
        MATH::Vector2f getPreviousLocation() const;
        MATH::Vector2f getStartLocation() const;
        MATH::Vector2f getDelta() const;
        MATH::Vector2f getLocationInView() const;
        MATH::Vector2f getPreviousLocationInView() const;
        MATH::Vector2f getStartLocationInView() const;

    private:
        MouseEventType _mouseEventType;
        int _mouseButton;
        float _x;
        float _y;
        float _scrollX;
        float _scrollY;

        bool _startPointCaptured;
        MATH::Vector2f _startPoint;
        MATH::Vector2f _point;
        MATH::Vector2f _prevPoint;

        friend class EventListenerMouse;
    };

    class EventListenerMouse : public EventListener
    {
    public:
        static const std::string LISTENER_ID;
        static EventListenerMouse* create();

        virtual EventListenerMouse* clone() override;
        virtual bool checkAvailable() override;

        std::function<void(Event* event)> onMouseDown;
        std::function<void(Event* event)> onMouseUp;
        std::function<void(Event* event)> onMouseMove;
        std::function<void(Event* event)> onMouseScroll;

    public:
        EventListenerMouse();
        bool init();
    };

    class EventFocus : public Event
    {
    public:
        EventFocus(Node* widgetLoseFocus, Node* widgetGetFocus);

    private:
        Node *_widgetGetFocus;
        Node *_widgetLoseFocus;

        friend class EventListenerFocus;
    };

    class EventListenerFocus : public EventListener
    {
    public:
        static const std::string LISTENER_ID;
        static EventListenerFocus* create();

        virtual ~EventListenerFocus();

        virtual EventListenerFocus* clone() override;
        virtual bool checkAvailable() override;

    public:
        std::function<void(Node*, Node*)> onFocusChanged;

    public:
        EventListenerFocus();
        bool init();

        friend class EventDispatcher;
    };

    class EventTouch : public Event
    {
    public:
        enum class DispatchMode {
            ALL_AT_ONCE, /** All at once. */
            ONE_BY_ONE,  /** One by one. */
        };

        EventTouch()
            : _id(0),
            _startPointCaptured(false)
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
            _id = id;
            _prevPoint = _point;
            _point.x   = x;
            _point.y   = y;
            if (!_startPointCaptured)
            {
                _startPoint = _point;
                _startPointCaptured = true;
                _prevPoint = _point;
            }
        }

        int getID() const
        {
            return _id;
        }

    private:
        int _id;
        bool _startPointCaptured;
        MATH::Vector2f _startPoint;
        MATH::Vector2f _point;
        MATH::Vector2f _prevPoint;
    };

    class EventListenerTouchOneByOne : public EventListener
    {
    public:
        static const std::string LISTENER_ID;

        static EventListenerTouchOneByOne* create();
        virtual ~EventListenerTouchOneByOne();

        void setSwallowTouches(bool needSwallow);
        bool isSwallowTouches();

        virtual EventListenerTouchOneByOne* clone() override;
        virtual bool checkAvailable() override;

    public:
        typedef std::function<bool(EventTouch*, Event*)> ccTouchBeganCallback;
        typedef std::function<void(EventTouch*, Event*)> ccTouchCallback;

        ccTouchBeganCallback onTouchBegan;
        ccTouchCallback onTouchMoved;
        ccTouchCallback onTouchEnded;
        ccTouchCallback onTouchCancelled;

    public:
        EventListenerTouchOneByOne();
        bool init();

    private:
        std::vector<EventTouch*> _claimedTouches;
        bool _needSwallow;

        friend class EventDispatcher;
    };

    class EventListenerTouchAllAtOnce : public EventListener
    {
    public:
        static const std::string LISTENER_ID;

        static EventListenerTouchAllAtOnce* create();
        virtual ~EventListenerTouchAllAtOnce();

        virtual EventListenerTouchAllAtOnce* clone() override;
        virtual bool checkAvailable() override;
        //
    public:

        typedef std::function<void(const std::vector<EventTouch*>&, Event*)> ccTouchesCallback;

        ccTouchesCallback onTouchesBegan;
        ccTouchesCallback onTouchesMoved;
        ccTouchesCallback onTouchesEnded;
        ccTouchesCallback onTouchesCancelled;

    public:
        EventListenerTouchAllAtOnce();
        bool init();
    private:

        friend class EventDispatcher;
    };
}

#endif // EVENTLISTENER_H
