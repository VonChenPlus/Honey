#ifndef TOUCH_H
#define TOUCH_H

#include "BASE/HObject.h"
#include "MATH/Vector.h"

namespace GRAPH
{
    class Touch : public HObject
    {
    public:
        /**
         * Dispatch mode, how the touches are dispathced.
         * @js NA
         */
        enum class DispatchMode {
            ALL_AT_ONCE, /** All at once. */
            ONE_BY_ONE,  /** One by one. */
        };

        /** Constructor.
         * @js ctor
         */
        Touch()
            : _id(0),
            _startPointCaptured(false)
        {}

        /** Returns the current touch location in OpenGL coordinates.
         *
         * @return The current touch location in OpenGL coordinates.
         */
        MATH::Vector2f getLocation() const;
        /** Returns the previous touch location in OpenGL coordinates.
         *
         * @return The previous touch location in OpenGL coordinates.
         */
        MATH::Vector2f getPreviousLocation() const;
        /** Returns the start touch location in OpenGL coordinates.
         *
         * @return The start touch location in OpenGL coordinates.
         */
        MATH::Vector2f getStartLocation() const;
        /** Returns the delta of 2 current touches locations in screen coordinates.
         *
         * @return The delta of 2 current touches locations in screen coordinates.
         */
        MATH::Vector2f getDelta() const;
        /** Returns the current touch location in screen coordinates.
         *
         * @return The current touch location in screen coordinates.
         */
        MATH::Vector2f getLocationInView() const;
        /** Returns the previous touch location in screen coordinates.
         *
         * @return The previous touch location in screen coordinates.
         */
        MATH::Vector2f getPreviousLocationInView() const;
        /** Returns the start touch location in screen coordinates.
         *
         * @return The start touch location in screen coordinates.
         */
        MATH::Vector2f getStartLocationInView() const;

        /** Set the touch infomation. It always used to monitor touch event.
         *
         * @param id A given id
         * @param x A given x coordinate.
         * @param y A given y coordinate.
         */
        void setTouchInfo(int id, float x, float y)
        {
            _id = id;
            _prevPoint = _point;
            _point.x   = x;
            _point.y   = y;
            if (!_startPointCaptured)
            {
                _startPoint = _point;
                _startPointCaptured = true;
                _prevPoint = _point;
            }
        }
        /** Get touch id.
         * @js getId
         * @lua getId
         *
         * @return The id of touch.
         */
        int getID() const
        {
            return _id;
        }

    private:
        int _id;
        bool _startPointCaptured;
        MATH::Vector2f _startPoint;
        MATH::Vector2f _point;
        MATH::Vector2f _prevPoint;
    };
}

#endif  // TOUCH_H
