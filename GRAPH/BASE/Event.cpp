#include "GRAPH/BASE/Event.h"

namespace GRAPH
{
    Event::Event(Type type)
    : _type(type)
    , _isStopped(false)
    , _currentTarget(nullptr)
    {
    }

    Event::~Event()
    {
    }

    EventCustom::EventCustom(const std::string& eventName)
        : Event(Type::CUSTOM)
        , _userData(nullptr)
        , _eventName(eventName) {
    }
}
