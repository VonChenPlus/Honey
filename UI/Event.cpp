#include "Event.h"

namespace UI
{
    extern void EventTriggered(Event *e, EventParams params);

    void Event::add(std::function<EventReturn(EventParams&)> func) {
        HandlerRegistration reg;
        reg.func = func;
        handlers_.push_back(reg);
    }

    // Call this from input thread or whatever, it doesn't matter
    void Event::trigger(EventParams &e) {
        EventTriggered(this, e);
    }

    // Call this from UI thread
    EventReturn Event::dispatch(EventParams &e) {
        for (auto iter = handlers_.begin(); iter != handlers_.end(); ++iter) {
            if ((iter->func)(e) == EVENT_DONE) {
                // Event is handled, stop looping immediately. This event might even have gotten deleted.
                return EVENT_DONE;
            }
        }
        return EVENT_SKIPPED;
    }
}

