#ifndef EVENT_H
#define EVENT_H

#include "BASE/HObject.h"

namespace GRAPH
{
    #define EVENT_COME_TO_FOREGROUND    "event_come_to_foreground"
    #define EVENT_RENDERER_RECREATED    "event_renderer_recreated"
    #define EVENT_COME_TO_BACKGROUND    "event_come_to_background"

    class Node;

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

        inline Type getType() const { return _type; }

        inline void stopPropagation() { _isStopped = true; }

        inline bool isStopped() const { return _isStopped; }

        inline Node* getCurrentTarget() { return _currentTarget; }

    protected:
        inline void setCurrentTarget(Node* target) { _currentTarget = target; }

        Type _type;     ///< Event type

        bool _isStopped;       ///< whether the event has been stopped.
        Node* _currentTarget;  ///< Current target

        friend class EventDispatcher;
    };

    class EventCustom : public Event
    {
    public:
        EventCustom(const std::string& eventName);

        inline void setUserData(void* data) { _userData = data; }
        inline void* getUserData() const { return _userData; }

        inline const std::string& getEventName() const { return _eventName; }
    protected:
        void* _userData;       ///< User data
        std::string _eventName;
    };
}

#endif // EVENT_H
