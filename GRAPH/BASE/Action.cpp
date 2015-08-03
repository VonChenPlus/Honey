#include "GRAPH/BASE/Action.h"
#include "UTILS/STRING/StringUtils.h"
#include "MATH/Size.h"
#include "GRAPH/BASE/Director.h"

namespace GRAPH
{
    Action::Action()
    :_originalTarget(nullptr)
    ,_target(nullptr)
    ,_tag(Action::INVALID_TAG)
    {
    }

    Action::~Action()
    {
    }

    std::string Action::description() const
    {
        return UTILS::STRING::StringFromFormat("<Action | Tag = %d", _tag);
    }

    void Action::startWithTarget(Node *aTarget)
    {
        _originalTarget = _target = aTarget;
    }

    void Action::stop()
    {
        _target = nullptr;
    }

    bool Action::isDone() const
    {
        return true;
    }

    void Action::step(float)
    {
    }

    void Action::update(float)
    {
    }

    //
    // Follow
    //
    Follow::~Follow()
    {
        SAFE_RELEASE(_followedNode);
    }

    Follow* Follow::create(Node *followedNode, const MATH::Rectf& rect)
    {
        Follow *follow = new (std::nothrow) Follow();
        if (follow && follow->initWithTarget(followedNode, rect))
        {
            follow->autorelease();
            return follow;
        }
        SAFE_DELETE(follow);
        return nullptr;
    }

    bool Follow::initWithTarget(Node *followedNode, const MATH::Rectf& rect)
    {
        followedNode->retain();
        _followedNode = followedNode;
        _worldRect = rect;
        _boundarySet = !rect.equals(MATH::RectfZERO);
        _boundaryFullyCovered = false;

        MATH::Sizef winSize = Director::getInstance()->getWinSize();
        _fullScreenSize.set(winSize.width, winSize.height);
        _halfScreenSize = _fullScreenSize * 0.5f;

        if (_boundarySet)
        {
            _leftBoundary = -((rect.origin.x+rect.size.width) - _fullScreenSize.x);
            _rightBoundary = -rect.origin.x ;
            _topBoundary = -rect.origin.y;
            _bottomBoundary = -((rect.origin.y+rect.size.height) - _fullScreenSize.y);

            if(_rightBoundary < _leftBoundary)
            {
                // screen width is larger than world's boundary width
                //set both in the middle of the world
                _rightBoundary = _leftBoundary = (_leftBoundary + _rightBoundary) / 2;
            }
            if(_topBoundary < _bottomBoundary)
            {
                // screen width is larger than world's boundary width
                //set both in the middle of the world
                _topBoundary = _bottomBoundary = (_topBoundary + _bottomBoundary) / 2;
            }

            if( (_topBoundary == _bottomBoundary) && (_leftBoundary == _rightBoundary) )
            {
                _boundaryFullyCovered = true;
            }
        }

        return true;
    }

    void Follow::step(float)
    {
        if(_boundarySet)
        {
            // whole map fits inside a single screen, no need to modify the position - unless map boundaries are increased
            if(_boundaryFullyCovered)
            {
                return;
            }

            MATH::Vector2f tempPos = _halfScreenSize - _followedNode->getPosition();

            _target->setPosition(MATH_CLAMP(tempPos.x, _leftBoundary, _rightBoundary),
                                       MATH_CLAMP(tempPos.y, _bottomBoundary, _topBoundary));
        }
        else
        {
            _target->setPosition(_halfScreenSize - _followedNode->getPosition());
        }
    }

    bool Follow::isDone() const
    {
        return ( !_followedNode->isRunning() );
    }

    void Follow::stop()
    {
        _target = nullptr;
        Action::stop();
    }
}
