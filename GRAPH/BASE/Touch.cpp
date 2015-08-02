#include "GRAPH/BASE/Touch.h"
#include "GRAPH/BASE/Director.h"

namespace GRAPH
{
    // returns the current touch location in screen coordinates
    MATH::Vector2f Touch::getLocationInView() const
    {
        return _point;
    }

    // returns the previous touch location in screen coordinates
    MATH::Vector2f Touch::getPreviousLocationInView() const
    {
        return _prevPoint;
    }

    // returns the start touch location in screen coordinates
    MATH::Vector2f Touch::getStartLocationInView() const
    {
        return _startPoint;
    }

    // returns the current touch location in OpenGL coordinates
    MATH::Vector2f Touch::getLocation() const
    {
        return Director::getInstance()->convertToGL(_point);
    }

    // returns the previous touch location in OpenGL coordinates
    MATH::Vector2f Touch::getPreviousLocation() const
    {
        return Director::getInstance()->convertToGL(_prevPoint);
    }

    // returns the start touch location in OpenGL coordinates
    MATH::Vector2f Touch::getStartLocation() const
    {
        return Director::getInstance()->convertToGL(_startPoint);
    }

    // returns the delta position between the current location and the previous location in OpenGL coordinates
    MATH::Vector2f Touch::getDelta() const
    {
        return getLocation() - getPreviousLocation();
    }
}
