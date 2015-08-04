#include "GRAPH/BASE/EventListener.h"

namespace GRAPH
{
    EventListener::EventListener() {

    }

    EventListener::~EventListener() {
    }

    bool EventListener::init(Type t, const ListenerID& listenerID, const std::function<void(Event*)>& callback) {
        _onEvent = callback;
        _type = t;
        _listenerID = listenerID;
        _isRegistered = false;
        _paused = true;
        _isEnabled = true;

        return true;
    }

    bool EventListener::checkAvailable() {
        return (_onEvent != nullptr);
    }
}
