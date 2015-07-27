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
}
