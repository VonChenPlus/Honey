#ifndef EVENT_H
#define EVENT_H

#include <functional>
#include <vector>

#include "BASE/Native.h"

namespace UI
{
    class View;
    // Should cover all bases.
    struct EventParams
    {
        View *v;
        uint32_t a, b, x, y;
        float f;
        std::string s;
    };

    // I hope I can find a way to simplify this one day.
    enum EventReturn
    {
        EVENT_DONE,  // Return this when no other view may process this event, for example if you changed the view hierarchy
        EVENT_SKIPPED,  // Return this if you ignored an event
        EVENT_CONTINUE,  // Return this if it's safe to send this event to further listeners. This should normally be the default choice but often EVENT_DONE is necessary.
    };

    struct HandlerRegistration {
        std::function<EventReturn(EventParams&)> func;
    };
    class Event {
    public:
        Event() {}
        ~Event() {
            handlers_.clear();
        }
        // Call this from input thread or whatever, it doesn't matter
        void trigger(EventParams &e);
        // Call this from UI thread
        EventReturn dispatch(EventParams &e);

        // This is suggested for use in most cases. Autobinds, allowing for neat syntax.
        template<class T>
        T *handle(T *thiz, EventReturn (T::* theCallback)(EventParams &e)) {
            add(std::bind(theCallback, thiz, std::placeholders::_1));
            return thiz;
        }

        // Sometimes you have an already-bound function<>, just use this then.
        void add(std::function<EventReturn(EventParams&)> func);

    private:
        std::vector<HandlerRegistration> handlers_;
        DISALLOW_COPY_AND_ASSIGN(Event)
    };
}

#endif // EVENT_H
