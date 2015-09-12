#ifndef EVENTLISTENER_H
#define EVENTLISTENER_H

#include <functional>
#include <string>
#include <memory>
#include <vector>
#include "BASE/HObject.h"
#include "GRAPH/Event.h"
#include "MATH/Vector.h"

namespace GRAPH
{
    class Event;

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

        inline void setEnabled(bool enabled) { enabled_ = enabled; }
        inline bool isEnabled() const { return enabled_; }

    protected:
        inline void setPaused(bool paused) { paused_ = paused; }

        inline bool isPaused() const { return paused_; }

        inline void setRegistered(bool registered) { registered_ = registered; }
        inline bool isRegistered() const { return registered_; }

        inline Type getType() const { return type_; }
        inline const ListenerID& getListenerID() const { return listenerID_; }


        inline void setFixedPriority(int fixedPriority) { fixedPriority_ = fixedPriority; }
        inline int getFixedPriority() const { return fixedPriority_; }

        inline void setAssociatedNode(HObject* node) { node_ = node; }
        inline HObject* getAssociatedNode() const { return node_; }

        std::function<void(Event*)> _onEvent;

        Type type_;
        ListenerID listenerID_;
        bool registered_;

        int   fixedPriority_;   // The higher the number, the higher the priority, 0 is for scene graph base priority.
        HObject* node_;            // scene graph based priority
        bool paused_;
        bool enabled_;

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
        std::function<void(EventCustom*)> onCustomEvent;
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

        static EventListenerKeyboard* create();

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
        std::function<void(HObject*, HObject*)> onFocusChanged;

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
        typedef std::function<bool(Touch*, Event*)> TouchBeganCallback;
        typedef std::function<void(Touch*, Event*)> TouchCallback;

        TouchBeganCallback onTouchBegan;
        TouchCallback onTouchMoved;
        TouchCallback onTouchEnded;
        TouchCallback onTouchCancelled;

    public:
        EventListenerTouchOneByOne();
        bool init();

    private:
        std::vector<Touch*> claimedTouches_;
        bool needSwallow_;

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

    public:

        typedef std::function<void(const std::vector<Touch*>&, Event*)> TouchesCallback;

        TouchesCallback onTouchesBegan;
        TouchesCallback onTouchesMoved;
        TouchesCallback onTouchesEnded;
        TouchesCallback onTouchesCancelled;

    public:
        EventListenerTouchAllAtOnce();
        bool init();
    private:

        friend class EventDispatcher;
    };
}

#endif // EVENTLISTENER_H
