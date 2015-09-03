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
        typedef std::function<bool(Touch*, Event*)> ccTouchBeganCallback;
        typedef std::function<void(Touch*, Event*)> ccTouchCallback;

        ccTouchBeganCallback onTouchBegan;
        ccTouchCallback onTouchMoved;
        ccTouchCallback onTouchEnded;
        ccTouchCallback onTouchCancelled;

    public:
        EventListenerTouchOneByOne();
        bool init();

    private:
        std::vector<Touch*> _claimedTouches;
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

        typedef std::function<void(const std::vector<Touch*>&, Event*)> ccTouchesCallback;

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
