#ifndef EVENT_H
#define EVENT_H

#include "BASE/HObject.h"

class Node;

namespace GRAPH
{
    class Event : public HObject
    {
    public:
        /** Type Event type.*/
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
        /** Constructor */
        Event(Type type);

        /** Destructor.
         */
        virtual ~Event();

        /** Gets the event type.
         *
         * @return The event type.
         */
        inline Type getType() const { return _type; }

        /** Stops propagation for current event.
         */
        inline void stopPropagation() { _isStopped = true; }

        /** Checks whether the event has been stopped.
         *
         * @return True if the event has been stopped.
         */
        inline bool isStopped() const { return _isStopped; }

        /** Gets current target of the event.
         * @return The target with which the event associates.
         * @note It onlys be available when the event listener is associated with node.
         *        It returns 0 when the listener is associated with fixed priority.
         */
        inline Node* getCurrentTarget() { return _currentTarget; }

    protected:
        /** Sets current target */
        inline void setCurrentTarget(Node* target) { _currentTarget = target; }

        Type _type;     ///< Event type

        bool _isStopped;       ///< whether the event has been stopped.
        Node* _currentTarget;  ///< Current target

        friend class EventDispatcher;
    };

    class EventCustom : public Event
    {
    public:
        /** Constructor.
         *
         * @param eventName A given name of the custom event.
         * @js ctor
         */
        EventCustom(const std::string& eventName);

        /** Sets user data.
         *
         * @param data The user data pointer, it's a void*.
         */
        inline void setUserData(void* data) { _userData = data; }

        /** Gets user data.
         *
         * @return The user data pointer, it's a void*.
         */
        inline void* getUserData() const { return _userData; }

        /** Gets event name.
         *
         * @return The name of the event.
         */
        inline const std::string& getEventName() const { return _eventName; }
    protected:
        void* _userData;       ///< User data
        std::string _eventName;
    };
}

#endif // EVENT_H
