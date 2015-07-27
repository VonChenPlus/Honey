#ifndef TouchEvent_H
#define TouchEvent_H

#include "GRAPH/BASE/Event.h"
#include <vector>

namespace GRAPH
{
    class Touch;

    #define TOUCH_PERF_DEBUG 1

    /** @class EventTouch
     * @brief Touch event.
     */
    class EventTouch : public Event
    {
    public:
        static const int MAX_TOUCHES = 15;

        /** EventCode Touch event code.*/
        enum class EventCode
        {
            BEGAN,
            MOVED,
            ENDED,
            CANCELLED
        };

        /**
         * Constructor.
         * @js NA
         */
        EventTouch();

        /** Get event code.
         *
         * @return The code of the event.
         */
        inline EventCode getEventCode() const { return _eventCode; }

        /** Get the touches.
         *
         * @return The touches of the event.
         */
        inline const std::vector<Touch*>& getTouches() const { return _touches; }

    #if TOUCH_PERF_DEBUG
        /** Set the event code.
         *
         * @param eventCode A given EventCode.
         */
        void setEventCode(EventCode eventCode) { _eventCode = eventCode; }
        /** Set the touches
         *
         * @param touches A given touches vector.
         */
        void setTouches(const std::vector<Touch*>& touches) { _touches = touches; }
    #endif

    private:
        EventCode _eventCode;
        std::vector<Touch*> _touches;

        friend class GLView;
    };
}

#endif // TouchEvent_H
