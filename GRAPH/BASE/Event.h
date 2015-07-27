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
}

#endif // EVENT_H
