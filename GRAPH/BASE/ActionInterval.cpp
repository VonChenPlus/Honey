#include "GRAPH/BASE/ActionInterval.h"
#include "GRAPH/BASE/Sprite.h"
#include "GRAPH/BASE/Director.h"
#include "GRAPH/BASE/Event.h"
#include "GRAPH/BASE/EventDispatcher.h"
#include <stdarg.h>

namespace GRAPH
{
    // Extra action for making a Sequence or Spawn when only adding one action to it.
    class ExtraAction : public FiniteTimeAction
    {
    public:
        static ExtraAction* create();
        virtual ExtraAction* clone() const;
        virtual ExtraAction* reverse(void) const;
        virtual void update(float time);
        virtual void step(float dt);
    };

    ExtraAction* ExtraAction::create()
    {
        ExtraAction* ret = new (std::nothrow) ExtraAction();
        if (ret)
        {
            ret->autorelease();
        }
        return ret;
    }
    ExtraAction* ExtraAction::clone() const
    {
        // no copy constructor
        auto a = new (std::nothrow) ExtraAction();
        a->autorelease();
        return a;
    }

    ExtraAction* ExtraAction::reverse() const
    {
        return ExtraAction::create();
    }

    void ExtraAction::update(float)
    {
    }

    void ExtraAction::step(float)
    {
    }

    //
    // IntervalAction
    //

    bool ActionInterval::initWithDuration(float d)
    {
        _duration = d;

        // prevent division by 0
        // This comparison could be in step:, but it might decrease the performance
        // by 3% in heavy based action games.
        if (_duration == 0)
        {
            _duration = FLT_EPSILON;
        }

        _elapsed = 0;
        _firstTick = true;

        return true;
    }

    bool ActionInterval::isDone() const
    {
        return _elapsed >= _duration;
    }

    void ActionInterval::step(float dt)
    {
        if (_firstTick)
        {
            _firstTick = false;
            _elapsed = 0;
        }
        else
        {
            _elapsed += dt;
        }

        this->update(MATH::MATH_MAX (0.0f,                                  // needed for rewind. elapsed could be negative
                          MATH::MATH_MIN(1.0f, _elapsed /
                              MATH::MATH_MAX(_duration, MATH::MATH_FLOAT_MAX())   // division by 0
                              )
                          )
                     );
    }

    void ActionInterval::setAmplitudeRate(float amp)
    {
    }

    float ActionInterval::getAmplitudeRate()
    {
        return 0;
    }

    void ActionInterval::startWithTarget(Node *target)
    {
        FiniteTimeAction::startWithTarget(target);
        _elapsed = 0.0f;
        _firstTick = true;
    }

    //
    // Sequence
    //

    Sequence* Sequence::createWithTwoActions(FiniteTimeAction *actionOne, FiniteTimeAction *actionTwo)
    {
        Sequence *sequence = new (std::nothrow) Sequence();
        sequence->initWithTwoActions(actionOne, actionTwo);
        sequence->autorelease();

        return sequence;
    }

    Sequence* Sequence::create(FiniteTimeAction *action1, ...)
    {
        va_list params;
        va_start(params, action1);

        Sequence *ret = Sequence::createWithVariableList(action1, params);

        va_end(params);

        return ret;
    }

    Sequence* Sequence::createWithVariableList(FiniteTimeAction *action1, va_list args)
    {
        FiniteTimeAction *now;
        FiniteTimeAction *prev = action1;
        bool bOneAction = true;

        while (action1)
        {
            now = va_arg(args, FiniteTimeAction*);
            if (now)
            {
                prev = createWithTwoActions(prev, now);
                bOneAction = false;
            }
            else
            {
                // If only one action is added to Sequence, make up a Sequence by adding a simplest finite time action.
                if (bOneAction)
                {
                    prev = createWithTwoActions(prev, ExtraAction::create());
                }
                break;
            }
        }

        return ((Sequence*)prev);
    }

    Sequence* Sequence::create(const HObjectVector<FiniteTimeAction*>& arrayOfActions)
    {
        Sequence* ret = nullptr;
        do
        {
            auto count = arrayOfActions.size();
            if (count == 0) break;

            auto prev = arrayOfActions.at(0);

            if (count > 1)
            {
                for (int i = 1; i < count; ++i)
                {
                    prev = createWithTwoActions(prev, arrayOfActions.at(i));
                }
            }
            else
            {
                // If only one action is added to Sequence, make up a Sequence by adding a simplest finite time action.
                prev = createWithTwoActions(prev, ExtraAction::create());
            }
            ret = static_cast<Sequence*>(prev);
        }while (0);
        return ret;
    }

    bool Sequence::initWithTwoActions(FiniteTimeAction *actionOne, FiniteTimeAction *actionTwo)
    {
        float d = actionOne->getDuration() + actionTwo->getDuration();
        ActionInterval::initWithDuration(d);

        _actions[0] = actionOne;
        actionOne->retain();

        _actions[1] = actionTwo;
        actionTwo->retain();

        return true;
    }

    void Sequence::startWithTarget(Node *target)
    {
        ActionInterval::startWithTarget(target);
        _split = _actions[0]->getDuration() / _duration;
        _last = -1;
    }

    void Sequence::stop(void)
    {
        // Issue #1305
        if( _last != - 1)
        {
            _actions[_last]->stop();
        }

        ActionInterval::stop();
    }

    void Sequence::update(float t)
    {
        int found = 0;
        float new_t = 0.0f;

        if( t < _split ) {
            // action[0]
            found = 0;
            if( _split != 0 )
                new_t = t / _split;
            else
                new_t = 1;

        } else {
            // action[1]
            found = 1;
            if ( _split == 1 )
                new_t = 1;
            else
                new_t = (t-_split) / (1 - _split );
        }

        if ( found==1 ) {

            if( _last == -1 ) {
                // action[0] was skipped, execute it.
                _actions[0]->startWithTarget(_target);
                _actions[0]->update(1.0f);
                _actions[0]->stop();
            }
            else if( _last == 0 )
            {
                // switching to action 1. stop action 0.
                _actions[0]->update(1.0f);
                _actions[0]->stop();
            }
        }
        else if(found==0 && _last==1 )
        {
            // Reverse mode ?
            // FIXME: Bug. this case doesn't contemplate when _last==-1, found=0 and in "reverse mode"
            // since it will require a hack to know if an action is on reverse mode or not.
            // "step" should be overriden, and the "reverseMode" value propagated to inner Sequences.
            _actions[1]->update(0);
            _actions[1]->stop();
        }
        // Last action found and it is done.
        if( found == _last && _actions[found]->isDone() )
        {
            return;
        }

        // Last action found and it is done
        if( found != _last )
        {
            _actions[found]->startWithTarget(_target);
        }

        _actions[found]->update(new_t);
        _last = found;
    }

    //
    // Repeat
    //

    Repeat* Repeat::create(FiniteTimeAction *action, unsigned int times)
    {
        Repeat* repeat = new (std::nothrow) Repeat();
        repeat->initWithAction(action, times);
        repeat->autorelease();

        return repeat;
    }

    bool Repeat::initWithAction(FiniteTimeAction *action, unsigned int times)
    {
        float d = action->getDuration() * times;

        if (ActionInterval::initWithDuration(d))
        {
            _times = times;
            _innerAction = action;
            action->retain();

            _actionInstant = dynamic_cast<ActionInstant*>(action) ? true : false;
            //an instant action needs to be executed one time less in the update method since it uses startWithTarget to execute the action
            if (_actionInstant)
            {
                _times -=1;
            }
            _total = 0;

            return true;
        }

        return false;
    }

    Repeat::~Repeat(void)
    {
        SAFE_RELEASE(_innerAction);
    }

    void Repeat::startWithTarget(Node *target)
    {
        _total = 0;
        _nextDt = _innerAction->getDuration()/_duration;
        ActionInterval::startWithTarget(target);
        _innerAction->startWithTarget(target);
    }

    void Repeat::stop(void)
    {
        _innerAction->stop();
        ActionInterval::stop();
    }

    // issue #80. Instead of hooking step:, hook update: since it can be called by any
    // container action like Repeat, Sequence, Ease, etc..
    void Repeat::update(float dt)
    {
        if (dt >= _nextDt)
        {
            while (dt > _nextDt && _total < _times)
            {

                _innerAction->update(1.0f);
                _total++;

                _innerAction->stop();
                _innerAction->startWithTarget(_target);
                _nextDt = _innerAction->getDuration()/_duration * (_total+1);
            }

            // fix for issue #1288, incorrect end value of repeat
            if(dt >= 1.0f && _total < _times)
            {
                _total++;
            }

            // don't set an instant action back or update it, it has no use because it has no duration
            if (!_actionInstant)
            {
                if (_total == _times)
                {
                    _innerAction->update(1);
                    _innerAction->stop();
                }
                else
                {
                    // issue #390 prevent jerk, use right update
                    _innerAction->update(dt - (_nextDt - _innerAction->getDuration()/_duration));
                }
            }
        }
        else
        {
            _innerAction->update(fmodf(dt * _times,1.0f));
        }
    }

    bool Repeat::isDone(void) const
    {
        return _total == _times;
    }

    //
    // RepeatForever
    //
    RepeatForever::~RepeatForever()
    {
        SAFE_RELEASE(_innerAction);
    }

    RepeatForever *RepeatForever::create(ActionInterval *action)
    {
        RepeatForever *ret = new (std::nothrow) RepeatForever();
        if (ret && ret->initWithAction(action))
        {
            ret->autorelease();
            return ret;
        }
        SAFE_DELETE(ret);
        return nullptr;
    }

    bool RepeatForever::initWithAction(ActionInterval *action)
    {
        action->retain();
        _innerAction = action;
        return true;
    }

    void RepeatForever::startWithTarget(Node* target)
    {
        ActionInterval::startWithTarget(target);
        _innerAction->startWithTarget(target);
    }

    void RepeatForever::step(float dt)
    {
        _innerAction->step(dt);
        if (_innerAction->isDone())
        {
            float diff = _innerAction->getElapsed() - _innerAction->getDuration();
            if (diff > _innerAction->getDuration())
                diff = fmodf(diff, _innerAction->getDuration());
            _innerAction->startWithTarget(_target);
            // to prevent jerk. issue #390, 1247
            _innerAction->step(0.0f);
            _innerAction->step(diff);
        }
    }

    bool RepeatForever::isDone() const
    {
        return false;
    }

    Spawn* Spawn::create(FiniteTimeAction *action1, ...)
    {
        va_list params;
        va_start(params, action1);

        Spawn *ret = Spawn::createWithVariableList(action1, params);

        va_end(params);

        return ret;
    }

    Spawn* Spawn::createWithVariableList(FiniteTimeAction *action1, va_list args)
    {
        FiniteTimeAction *now;
        FiniteTimeAction *prev = action1;
        bool oneAction = true;

        while (action1)
        {
            now = va_arg(args, FiniteTimeAction*);
            if (now)
            {
                prev = createWithTwoActions(prev, now);
                oneAction = false;
            }
            else
            {
                // If only one action is added to Spawn, make up a Spawn by adding a simplest finite time action.
                if (oneAction)
                {
                    prev = createWithTwoActions(prev, ExtraAction::create());
                }
                break;
            }
        }

        return ((Spawn*)prev);
    }

    Spawn* Spawn::create(const HObjectVector<FiniteTimeAction*>& arrayOfActions)
    {
        Spawn* ret = nullptr;
        do
        {
            auto count = arrayOfActions.size();
            if(count == 0) break;
            auto prev = arrayOfActions.at(0);
            if (count > 1)
            {
                for (int i = 1; i < arrayOfActions.size(); ++i)
                {
                    prev = createWithTwoActions(prev, arrayOfActions.at(i));
                }
            }
            else
            {
                // If only one action is added to Spawn, make up a Spawn by adding a simplest finite time action.
                prev = createWithTwoActions(prev, ExtraAction::create());
            }
            ret = static_cast<Spawn*>(prev);
        }while (0);

        return ret;
    }

    Spawn* Spawn::createWithTwoActions(FiniteTimeAction *action1, FiniteTimeAction *action2)
    {
        Spawn *spawn = new (std::nothrow) Spawn();
        spawn->initWithTwoActions(action1, action2);
        spawn->autorelease();

        return spawn;
    }

    bool Spawn::initWithTwoActions(FiniteTimeAction *action1, FiniteTimeAction *action2)
    {
        bool ret = false;

        float d1 = action1->getDuration();
        float d2 = action2->getDuration();

        if (ActionInterval::initWithDuration(MATH::MATH_MAX(d1, d2)))
        {
            _one = action1;
            _two = action2;

            if (d1 > d2)
            {
                _two = Sequence::createWithTwoActions(action2, DelayTime::create(d1 - d2));
            }
            else if (d1 < d2)
            {
                _one = Sequence::createWithTwoActions(action1, DelayTime::create(d2 - d1));
            }

            _one->retain();
            _two->retain();

            ret = true;
        }

        return ret;
    }

    Spawn::~Spawn(void)
    {
        SAFE_RELEASE(_one);
        SAFE_RELEASE(_two);
    }

    void Spawn::startWithTarget(Node *target)
    {
        ActionInterval::startWithTarget(target);
        _one->startWithTarget(target);
        _two->startWithTarget(target);
    }

    void Spawn::stop(void)
    {
        _one->stop();
        _two->stop();
        ActionInterval::stop();
    }

    void Spawn::update(float time)
    {
        if (_one)
        {
            _one->update(time);
        }
        if (_two)
        {
            _two->update(time);
        }
    }

    //
    // RotateTo
    //

    RotateTo* RotateTo::create(float duration, float dstAngle)
    {
        RotateTo* rotateTo = new (std::nothrow) RotateTo();
        rotateTo->initWithDuration(duration, dstAngle, dstAngle);
        rotateTo->autorelease();

        return rotateTo;
    }

    RotateTo* RotateTo::create(float duration, float dstAngleX, float dstAngleY)
    {
        RotateTo* rotateTo = new (std::nothrow) RotateTo();
        rotateTo->initWithDuration(duration, dstAngleX, dstAngleY);
        rotateTo->autorelease();

        return rotateTo;
    }

    RotateTo::RotateTo()
    : _is3D(false)
    {
    }

    bool RotateTo::initWithDuration(float duration, float dstAngleX, float dstAngleY)
    {
        if (ActionInterval::initWithDuration(duration))
        {
            _dstAngle.x = dstAngleX;
            _dstAngle.y = dstAngleY;

            return true;
        }

        return false;
    }

    void RotateTo::calculateAngles(float &startAngle, float &diffAngle, float dstAngle)
    {
        if (startAngle > 0)
        {
            startAngle = fmodf(startAngle, 360.0f);
        }
        else
        {
            startAngle = fmodf(startAngle, -360.0f);
        }

        diffAngle = dstAngle - startAngle;
        if (diffAngle > 180)
        {
            diffAngle -= 360;
        }
        if (diffAngle < -180)
        {
            diffAngle += 360;
        }
    }

    void RotateTo::startWithTarget(Node *target)
    {
        ActionInterval::startWithTarget(target);

        if (_is3D)
        {
            _startAngle = _target->getRotation3D();
        }
        else
        {
            _startAngle.x = _target->getRotationSkewX();
            _startAngle.y = _target->getRotationSkewY();
        }

        calculateAngles(_startAngle.x, _diffAngle.x, _dstAngle.x);
        calculateAngles(_startAngle.y, _diffAngle.y, _dstAngle.y);
        calculateAngles(_startAngle.z, _diffAngle.z, _dstAngle.z);
    }

    void RotateTo::update(float time)
    {
        if (_target)
        {
            _target->setRotationSkewX(_startAngle.x + _diffAngle.x * time);
            _target->setRotationSkewY(_startAngle.y + _diffAngle.y * time);
        }
    }

    //
    // RotateBy
    //

    RotateBy* RotateBy::create(float duration, float deltaAngle)
    {
        RotateBy *rotateBy = new (std::nothrow) RotateBy();
        rotateBy->initWithDuration(duration, deltaAngle);
        rotateBy->autorelease();

        return rotateBy;
    }

    RotateBy::RotateBy()
    {
    }

    bool RotateBy::initWithDuration(float duration, float deltaAngle)
    {
        if (ActionInterval::initWithDuration(duration))
        {
            _deltaAngle.x = _deltaAngle.y = deltaAngle;
            return true;
        }

        return false;
    }

    bool RotateBy::initWithDuration(float duration, float deltaAngleX, float deltaAngleY)
    {
        if (ActionInterval::initWithDuration(duration))
        {
            _deltaAngle.x = deltaAngleX;
            _deltaAngle.y = deltaAngleY;
            return true;
        }

        return false;
    }

    void RotateBy::startWithTarget(Node *target)
    {
        ActionInterval::startWithTarget(target);
        _startAngle.x = target->getRotationSkewX();
        _startAngle.y = target->getRotationSkewY();

    }

    void RotateBy::update(float time)
    {
        // FIXME: shall I add % 360
        if (_target)
        {
            _target->setRotationSkewX(_startAngle.x + _deltaAngle.x * time);
            _target->setRotationSkewY(_startAngle.y + _deltaAngle.y * time);
        }
    }
    //
    // MoveBy
    //

    MoveBy* MoveBy::create(float duration, const MATH::Vector2f& deltaPosition)
    {
        return MoveBy::create(duration, MATH::Vector3f(deltaPosition.x, deltaPosition.y, 0));
    }

    MoveBy* MoveBy::create(float duration, const MATH::Vector3f &deltaPosition)
    {
        MoveBy *ret = new (std::nothrow) MoveBy();

        if (ret)
        {
            if (ret->initWithDuration(duration, deltaPosition))
            {
                ret->autorelease();
            }
            else
            {
                delete ret;
                ret = nullptr;
            }
        }

        return ret;
    }

    bool MoveBy::initWithDuration(float duration, const MATH::Vector2f& deltaPosition)
    {
        return MoveBy::initWithDuration(duration, MATH::Vector3f(deltaPosition.x, deltaPosition.y, 0));
    }

    bool MoveBy::initWithDuration(float duration, const MATH::Vector3f& deltaPosition)
    {
        bool ret = false;

        if (ActionInterval::initWithDuration(duration))
        {
            _positionDelta = deltaPosition;
            ret = true;
        }

        return ret;
    }

    void MoveBy::startWithTarget(Node *target)
    {
        ActionInterval::startWithTarget(target);
        _previousPosition = _startPosition = target->getPosition3D();
    }

    void MoveBy::update(float t)
    {
        if (_target)
        {
            MATH::Vector3f currentPos = _target->getPosition3D();
            MATH::Vector3f diff = currentPos - _previousPosition;
            _startPosition = _startPosition + diff;
            MATH::Vector3f newPos =  _startPosition + (_positionDelta * t);
            _target->setPosition3D(newPos);
            _previousPosition = newPos;
        }
    }

    //
    // MoveTo
    //

    MoveTo* MoveTo::create(float duration, const MATH::Vector2f& position)
    {
        return MoveTo::create(duration, MATH::Vector3f(position.x, position.y, 0));
    }

    MoveTo* MoveTo::create(float duration, const MATH::Vector3f& position)
    {
        MoveTo *ret = new (std::nothrow) MoveTo();

        if (ret)
        {
            if (ret->initWithDuration(duration, position))
            {
                ret->autorelease();
            }
            else
            {
                delete ret;
                ret = nullptr;
            }
        }

        return ret;
    }

    bool MoveTo::initWithDuration(float duration, const MATH::Vector2f& position)
    {
        return initWithDuration(duration, MATH::Vector3f(position.x, position.y, 0));
    }

    bool MoveTo::initWithDuration(float duration, const MATH::Vector3f& position)
    {
        bool ret = false;

        if (ActionInterval::initWithDuration(duration))
        {
            _endPosition = position;
            ret = true;
        }

        return ret;
    }

    void MoveTo::startWithTarget(Node *target)
    {
        MoveBy::startWithTarget(target);
        _positionDelta = _endPosition - target->getPosition3D();
    }

    //
    // SkewTo
    //
    SkewTo* SkewTo::create(float t, float sx, float sy)
    {
        SkewTo *skewTo = new (std::nothrow) SkewTo();
        if (skewTo)
        {
            if (skewTo->initWithDuration(t, sx, sy))
            {
                skewTo->autorelease();
            }
            else
            {
                SAFE_DELETE(skewTo);
            }
        }

        return skewTo;
    }

    bool SkewTo::initWithDuration(float t, float sx, float sy)
    {
        bool bRet = false;

        if (ActionInterval::initWithDuration(t))
        {
            _endSkewX = sx;
            _endSkewY = sy;

            bRet = true;
        }

        return bRet;
    }

    void SkewTo::startWithTarget(Node *target)
    {
        ActionInterval::startWithTarget(target);

        _startSkewX = target->getSkewX();

        if (_startSkewX > 0)
        {
            _startSkewX = fmodf(_startSkewX, 180.f);
        }
        else
        {
            _startSkewX = fmodf(_startSkewX, -180.f);
        }

        _deltaX = _endSkewX - _startSkewX;

        if (_deltaX > 180)
        {
            _deltaX -= 360;
        }
        if (_deltaX < -180)
        {
            _deltaX += 360;
        }

        _startSkewY = target->getSkewY();

        if (_startSkewY > 0)
        {
            _startSkewY = fmodf(_startSkewY, 360.f);
        }
        else
        {
            _startSkewY = fmodf(_startSkewY, -360.f);
        }

        _deltaY = _endSkewY - _startSkewY;

        if (_deltaY > 180)
        {
            _deltaY -= 360;
        }
        if (_deltaY < -180)
        {
            _deltaY += 360;
        }
    }

    void SkewTo::update(float t)
    {
        _target->setSkewX(_startSkewX + _deltaX * t);
        _target->setSkewY(_startSkewY + _deltaY * t);
    }

    SkewTo::SkewTo()
    : _skewX(0.0)
    , _skewY(0.0)
    , _startSkewX(0.0)
    , _startSkewY(0.0)
    , _endSkewX(0.0)
    , _endSkewY(0.0)
    , _deltaX(0.0)
    , _deltaY(0.0)
    {
    }

    //
    // SkewBy
    //
    SkewBy* SkewBy::create(float t, float sx, float sy)
    {
        SkewBy *skewBy = new (std::nothrow) SkewBy();
        if (skewBy)
        {
            if (skewBy->initWithDuration(t, sx, sy))
            {
                skewBy->autorelease();
            }
            else
            {
                SAFE_DELETE(skewBy);
            }
        }

        return skewBy;
    }

    bool SkewBy::initWithDuration(float t, float deltaSkewX, float deltaSkewY)
    {
        bool ret = false;

        if (SkewTo::initWithDuration(t, deltaSkewX, deltaSkewY))
        {
            _skewX = deltaSkewX;
            _skewY = deltaSkewY;

            ret = true;
        }

        return ret;
    }

    void SkewBy::startWithTarget(Node *target)
    {
        SkewTo::startWithTarget(target);
        _deltaX = _skewX;
        _deltaY = _skewY;
        _endSkewX = _startSkewX + _deltaX;
        _endSkewY = _startSkewY + _deltaY;
    }

    //
    // JumpBy
    //

    JumpBy* JumpBy::create(float duration, const MATH::Vector2f& position, float height, int jumps)
    {
        JumpBy *jumpBy = new (std::nothrow) JumpBy();
        jumpBy->initWithDuration(duration, position, height, jumps);
        jumpBy->autorelease();

        return jumpBy;
    }

    bool JumpBy::initWithDuration(float duration, const MATH::Vector2f& position, float height, int jumps)
    {
        if (ActionInterval::initWithDuration(duration) && jumps>=0)
        {
            _delta = position;
            _height = height;
            _jumps = jumps;

            return true;
        }

        return false;
    }

    void JumpBy::startWithTarget(Node *target)
    {
        ActionInterval::startWithTarget(target);
        _previousPos = _startPosition = target->getPosition();
    }

    void JumpBy::update(float t)
    {
        // parabolic jump (since v0.8.2)
        if (_target)
        {
            float frac = fmodf( t * _jumps, 1.0f );
            float y = _height * 4 * frac * (1 - frac);
            y += _delta.y * t;

            float x = _delta.x * t;
            MATH::Vector2f currentPos = _target->getPosition();

            MATH::Vector2f diff = currentPos - _previousPos;
            _startPosition = diff + _startPosition;

            MATH::Vector2f newPos = _startPosition + MATH::Vector2f(x,y);
            _target->setPosition(newPos);

            _previousPos = newPos;
        }
    }

    //
    // JumpTo
    //

    JumpTo* JumpTo::create(float duration, const MATH::Vector2f& position, float height, int jumps)
    {
        JumpTo *jumpTo = new (std::nothrow) JumpTo();
        jumpTo->initWithDuration(duration, position, height, jumps);
        jumpTo->autorelease();

        return jumpTo;
    }

    bool JumpTo::initWithDuration(float duration, const MATH::Vector2f& position, float height, int jumps)
    {
        if (ActionInterval::initWithDuration(duration) && jumps>=0)
        {
            _endPosition = position;
            _height = height;
            _jumps = jumps;

            return true;
        }

        return false;
    }

    void JumpTo::startWithTarget(Node *target)
    {
        JumpBy::startWithTarget(target);
        _delta.set(_endPosition.x - _startPosition.x, _endPosition.y - _startPosition.y);
    }

    // Bezier cubic formula:
    //    ((1 - t) + t)3 = 1
    // Expands to ...
    //   (1 - t)3 + 3t(1-t)2 + 3t2(1 - t) + t3 = 1
    static inline float bezierat( float a, float b, float c, float d, float t )
    {
        return (powf(1-t,3) * a +
                3*t*(powf(1-t,2))*b +
                3*powf(t,2)*(1-t)*c +
                powf(t,3)*d );
    }

    //
    // BezierBy
    //

    BezierBy* BezierBy::create(float t, const ccBezierConfig& c)
    {
        BezierBy *bezierBy = new (std::nothrow) BezierBy();
        bezierBy->initWithDuration(t, c);
        bezierBy->autorelease();

        return bezierBy;
    }

    bool BezierBy::initWithDuration(float t, const ccBezierConfig& c)
    {
        if (ActionInterval::initWithDuration(t))
        {
            _config = c;
            return true;
        }

        return false;
    }

    void BezierBy::startWithTarget(Node *target)
    {
        ActionInterval::startWithTarget(target);
        _previousPosition = _startPosition = target->getPosition();
    }

    void BezierBy::update(float time)
    {
        if (_target)
        {
            float xa = 0;
            float xb = _config.controlPoint_1.x;
            float xc = _config.controlPoint_2.x;
            float xd = _config.endPosition.x;

            float ya = 0;
            float yb = _config.controlPoint_1.y;
            float yc = _config.controlPoint_2.y;
            float yd = _config.endPosition.y;

            float x = bezierat(xa, xb, xc, xd, time);
            float y = bezierat(ya, yb, yc, yd, time);

            MATH::Vector2f currentPos = _target->getPosition();
            MATH::Vector2f diff = currentPos - _previousPosition;
            _startPosition = _startPosition + diff;

            MATH::Vector2f newPos = _startPosition + MATH::Vector2f(x,y);
            _target->setPosition(newPos);

            _previousPosition = newPos;
        }
    }

    //
    // BezierTo
    //

    BezierTo* BezierTo::create(float t, const ccBezierConfig& c)
    {
        BezierTo *bezierTo = new (std::nothrow) BezierTo();
        bezierTo->initWithDuration(t, c);
        bezierTo->autorelease();

        return bezierTo;
    }

    bool BezierTo::initWithDuration(float t, const ccBezierConfig &c)
    {
        if (ActionInterval::initWithDuration(t))
        {
            _toConfig = c;
            return true;
        }

        return false;
    }

    void BezierTo::startWithTarget(Node *target)
    {
        BezierBy::startWithTarget(target);
        _config.controlPoint_1 = _toConfig.controlPoint_1 - _startPosition;
        _config.controlPoint_2 = _toConfig.controlPoint_2 - _startPosition;
        _config.endPosition = _toConfig.endPosition - _startPosition;
    }

    //
    // ScaleTo
    //
    ScaleTo* ScaleTo::create(float duration, float s)
    {
        ScaleTo *scaleTo = new (std::nothrow) ScaleTo();
        scaleTo->initWithDuration(duration, s);
        scaleTo->autorelease();

        return scaleTo;
    }

    ScaleTo* ScaleTo::create(float duration, float sx, float sy)
    {
        ScaleTo *scaleTo = new (std::nothrow) ScaleTo();
        scaleTo->initWithDuration(duration, sx, sy);
        scaleTo->autorelease();

        return scaleTo;
    }

    ScaleTo* ScaleTo::create(float duration, float sx, float sy, float sz)
    {
        ScaleTo *scaleTo = new (std::nothrow) ScaleTo();
        scaleTo->initWithDuration(duration, sx, sy, sz);
        scaleTo->autorelease();

        return scaleTo;
    }

    bool ScaleTo::initWithDuration(float duration, float s)
    {
        if (ActionInterval::initWithDuration(duration))
        {
            _endScaleX = s;
            _endScaleY = s;
            _endScaleZ = s;

            return true;
        }

        return false;
    }

    bool ScaleTo::initWithDuration(float duration, float sx, float sy)
    {
        if (ActionInterval::initWithDuration(duration))
        {
            _endScaleX = sx;
            _endScaleY = sy;
            _endScaleZ = 1.f;

            return true;
        }

        return false;
    }

    bool ScaleTo::initWithDuration(float duration, float sx, float sy, float sz)
    {
        if (ActionInterval::initWithDuration(duration))
        {
            _endScaleX = sx;
            _endScaleY = sy;
            _endScaleZ = sz;

            return true;
        }

        return false;
    }

    void ScaleTo::startWithTarget(Node *target)
    {
        ActionInterval::startWithTarget(target);
        _startScaleX = target->getScaleX();
        _startScaleY = target->getScaleY();
        _startScaleZ = target->getScaleZ();
        _deltaX = _endScaleX - _startScaleX;
        _deltaY = _endScaleY - _startScaleY;
        _deltaZ = _endScaleZ - _startScaleZ;
    }

    void ScaleTo::update(float time)
    {
        if (_target)
        {
            _target->setScaleX(_startScaleX + _deltaX * time);
            _target->setScaleY(_startScaleY + _deltaY * time);
            _target->setScaleZ(_startScaleZ + _deltaZ * time);
        }
    }

    //
    // ScaleBy
    //

    ScaleBy* ScaleBy::create(float duration, float s)
    {
        ScaleBy *scaleBy = new (std::nothrow) ScaleBy();
        scaleBy->initWithDuration(duration, s);
        scaleBy->autorelease();

        return scaleBy;
    }

    ScaleBy* ScaleBy::create(float duration, float sx, float sy)
    {
        ScaleBy *scaleBy = new (std::nothrow) ScaleBy();
        scaleBy->initWithDuration(duration, sx, sy, 1.f);
        scaleBy->autorelease();

        return scaleBy;
    }

    ScaleBy* ScaleBy::create(float duration, float sx, float sy, float sz)
    {
        ScaleBy *scaleBy = new (std::nothrow) ScaleBy();
        scaleBy->initWithDuration(duration, sx, sy, sz);
        scaleBy->autorelease();

        return scaleBy;
    }

    void ScaleBy::startWithTarget(Node *target)
    {
        ScaleTo::startWithTarget(target);
        _deltaX = _startScaleX * _endScaleX - _startScaleX;
        _deltaY = _startScaleY * _endScaleY - _startScaleY;
        _deltaZ = _startScaleZ * _endScaleZ - _startScaleZ;
    }

    //
    // Blink
    //

    Blink* Blink::create(float duration, int blinks)
    {
        Blink *blink = new (std::nothrow) Blink();
        blink->initWithDuration(duration, blinks);
        blink->autorelease();

        return blink;
    }

    bool Blink::initWithDuration(float duration, int blinks)
    {
        if (ActionInterval::initWithDuration(duration) && blinks>=0)
        {
            _times = blinks;
            return true;
        }

        return false;
    }

    void Blink::stop()
    {
        if(NULL != _target)
            _target->setVisible(_originalState);
        ActionInterval::stop();
    }

    void Blink::startWithTarget(Node *target)
    {
        ActionInterval::startWithTarget(target);
        _originalState = target->isVisible();
    }

    void Blink::update(float time)
    {
        if (_target && ! isDone())
        {
            float slice = 1.0f / _times;
            float m = fmodf(time, slice);
            _target->setVisible(m > slice / 2 ? true : false);
        }
    }

    //
    // FadeIn
    //

    FadeIn* FadeIn::create(float d)
    {
        FadeIn* action = new (std::nothrow) FadeIn();

        action->initWithDuration(d,255.0f);
        action->autorelease();

        return action;
    }

    void FadeIn::setReverseAction(FadeTo *ac)
    {
        _reverseAction = ac;
    }

    void FadeIn::startWithTarget(Node *target)
    {
        ActionInterval::startWithTarget(target);

        if (nullptr != _reverseAction) {
            this->_toOpacity = this->_reverseAction->_fromOpacity;
        }else{
            _toOpacity = 255.0f;
        }

        if (target) {
            _fromOpacity = target->getOpacity();
        }
    }



    //
    // FadeOut
    //

    FadeOut* FadeOut::create(float d)
    {
        FadeOut* action = new (std::nothrow) FadeOut();

        action->initWithDuration(d,0.0f);
        action->autorelease();

        return action;
    }

    void FadeOut::startWithTarget(Node *target)
    {
        ActionInterval::startWithTarget(target);

        if (nullptr != _reverseAction) {
            _toOpacity = _reverseAction->_fromOpacity;
        }else{
            _toOpacity = 0.0f;
        }

        if (target) {
            _fromOpacity = target->getOpacity();
        }
    }

    void FadeOut::setReverseAction(FadeTo *ac)
    {
        _reverseAction = ac;
    }

    //
    // FadeTo
    //

    FadeTo* FadeTo::create(float duration, GLubyte opacity)
    {
        FadeTo *fadeTo = new (std::nothrow) FadeTo();
        fadeTo->initWithDuration(duration, opacity);
        fadeTo->autorelease();

        return fadeTo;
    }

    bool FadeTo::initWithDuration(float duration, GLubyte opacity)
    {
        if (ActionInterval::initWithDuration(duration))
        {
            _toOpacity = opacity;
            return true;
        }

        return false;
    }

    void FadeTo::startWithTarget(Node *target)
    {
        ActionInterval::startWithTarget(target);

        if (target)
        {
            _fromOpacity = target->getOpacity();
        }
        /*_fromOpacity = target->getOpacity();*/
    }

    void FadeTo::update(float time)
    {
        if (_target)
        {
            _target->setOpacity((GLubyte)(_fromOpacity + (_toOpacity - _fromOpacity) * time));
        }
        /*_target->setOpacity((GLubyte)(_fromOpacity + (_toOpacity - _fromOpacity) * time));*/
    }

    //
    // TintTo
    //
    TintTo* TintTo::create(float duration, GLubyte red, GLubyte green, GLubyte blue)
    {
        TintTo *tintTo = new (std::nothrow) TintTo();
        tintTo->initWithDuration(duration, red, green, blue);
        tintTo->autorelease();

        return tintTo;
    }

    TintTo* TintTo::create(float duration, const Color3B& color)
    {
        return create(duration, color.red, color.green, color.blue);
    }

    bool TintTo::initWithDuration(float duration, GLubyte red, GLubyte green, GLubyte blue)
    {
        if (ActionInterval::initWithDuration(duration))
        {
            _to = Color3B(red, green, blue);
            return true;
        }

        return false;
    }

    void TintTo::startWithTarget(Node *target)
    {
        ActionInterval::startWithTarget(target);
        if (_target)
        {
            _from = _target->getColor();
        }
        /*_from = target->getColor();*/
    }

    void TintTo::update(float time)
    {
        if (_target)
        {
            _target->setColor(Color3B(GLubyte(_from.red + (_to.red - _from.red) * time),
                (GLubyte)(_from.green + (_to.green - _from.green) * time),
                (GLubyte)(_from.blue + (_to.blue - _from.blue) * time)));
        }
    }

    //
    // TintBy
    //

    TintBy* TintBy::create(float duration, GLshort deltaRed, GLshort deltaGreen, GLshort deltaBlue)
    {
        TintBy *tintBy = new (std::nothrow) TintBy();
        tintBy->initWithDuration(duration, deltaRed, deltaGreen, deltaBlue);
        tintBy->autorelease();

        return tintBy;
    }

    bool TintBy::initWithDuration(float duration, GLshort deltaRed, GLshort deltaGreen, GLshort deltaBlue)
    {
        if (ActionInterval::initWithDuration(duration))
        {
            _deltaR = deltaRed;
            _deltaG = deltaGreen;
            _deltaB = deltaBlue;

            return true;
        }

        return false;
    }

    void TintBy::startWithTarget(Node *target)
    {
        ActionInterval::startWithTarget(target);

        if (target)
        {
            Color3B color = target->getColor();
            _fromR = color.red;
            _fromG = color.green;
            _fromB = color.blue;
        }
    }

    void TintBy::update(float time)
    {
        if (_target)
        {
            _target->setColor(Color3B((GLubyte)(_fromR + _deltaR * time),
                (GLubyte)(_fromG + _deltaG * time),
                (GLubyte)(_fromB + _deltaB * time)));
        }
    }

    //
    // DelayTime
    //
    DelayTime* DelayTime::create(float d)
    {
        DelayTime* action = new (std::nothrow) DelayTime();

        action->initWithDuration(d);
        action->autorelease();

        return action;
    }

    void DelayTime::update(float time)
    {
        return;
    }

    // ActionFloat

    ActionFloat* ActionFloat::create(float duration, float from, float to, ActionFloatCallback callback)
    {
        auto ref = new (std::nothrow) ActionFloat();
        if (ref && ref->initWithDuration(duration, from, to, callback))
        {
            ref->autorelease();
            return ref;
        }
        SAFE_DELETE(ref);
        return ref;
    }

    bool ActionFloat::initWithDuration(float duration, float from, float to, ActionFloatCallback callback)
    {
        if (ActionInterval::initWithDuration(duration))
        {
            _from = from;
            _to = to;
            _callback = callback;
            return true;
        }
        return false;
    }

    void ActionFloat::startWithTarget(Node *target)
    {
        ActionInterval::startWithTarget(target);
        _delta = _to - _from;
    }

    void ActionFloat::update(float delta)
    {
        float value = _to - _delta * (1 - delta);

        if (_callback)
        {
            // report back value to caller
            _callback(value);
        }
    }
}
