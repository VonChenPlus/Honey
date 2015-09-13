#include "GRAPH/Action.h"
#include "GRAPH/Director.h"
#include "MATH/Size.h"

namespace GRAPH
{
    Action::Action()
        :_originalTarget(nullptr)
        ,_target(nullptr)
        ,_tag(Action::INVALID_TAG) {
    }

    Action::~Action()
    {
    }

    void Action::startWithTarget(HObject *aTarget)
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

    Follow::Follow()
        : _followedHObject(nullptr)
        , _boundarySet(false)
        , _boundaryFullyCovered(false)
        , _leftBoundary(0.0)
        , _rightBoundary(0.0)
        , _topBoundary(0.0)
        , _bottomBoundary(0.0)
        , _worldRect(MATH::RectfZERO) {

    }

    Follow::~Follow()
    {
        SAFE_RELEASE(_followedHObject);
    }

    Follow* Follow::create(HObject *followedHObject, const MATH::Rectf& rect)
    {
        Follow *follow = new (std::nothrow) Follow();
        if (follow && follow->initWithTarget(followedHObject, rect))
        {
            follow->autorelease();
            return follow;
        }
        SAFE_DELETE(follow);
        return nullptr;
    }

    bool Follow::initWithTarget(HObject *followedHObject, const MATH::Rectf& rect)
    {
        followedHObject->retain();
        _followedHObject = followedHObject;
        _worldRect = rect;
        _boundarySet = !rect.equals(MATH::RectfZERO);
        _boundaryFullyCovered = false;

        MATH::Sizef winSize = Director::getInstance().getWinSize();
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

            MATH::Vector2f tempPos = _halfScreenSize - _followedHObject->getPosition();

            _target->setPosition(MATH_CLAMP(tempPos.x, _leftBoundary, _rightBoundary),
                                       MATH_CLAMP(tempPos.y, _bottomBoundary, _topBoundary));
        }
        else
        {
            _target->setPosition(_halfScreenSize - _followedHObject->getPosition());
        }
    }

    bool Follow::isDone() const
    {
        return ( !_followedHObject->isRunning() );
    }

    void Follow::stop()
    {
        _target = nullptr;
        Action::stop();
    }

    ActionManager::ActionManager()
    : _currentTarget(nullptr),
      _currentTargetSalvaged(false) {

    }

    ActionManager::~ActionManager() {
        removeAllActions();
    }

    void ActionManager::deleteHashElement(ActionEntry *element) {
        SAFE_DELETE(element->actions);
        _targets.erase(element->target);
        element->target->release();
        delete element;
    }

    void ActionManager::actionAllocWithHashElement(ActionEntry *element) {
        // 4 actions per HObject by default
        if (element->actions == nullptr) {
            element->actions = new HObjectArray(4);
        }
        else if (element->actions->number() == element->actions->maximun()) {
            element->actions->doubleCapacity();
        }
    }

    void ActionManager::removeActionAtIndex(ssize_t index, ActionEntry *element) {
        Action *action = (Action*)(*element->actions)[index];
        if (action == element->currentAction && (! element->currentActionSalvaged)) {
            element->currentAction->retain();
            element->currentActionSalvaged = true;
        }

        element->actions->removeObjectAtIndex(index, true);

        // update actionIndex in case we are in tick. looping over the actions
        if (element->actionIndex >= index) {
            element->actionIndex--;
        }

        if (element->actions->number() == 0) {
            if (_currentTarget == element) {
                _currentTargetSalvaged = true;
            }
            else {
                deleteHashElement(element);
            }
        }
    }

    void ActionManager::pauseTarget(HObject *target) {
        ActionEntry *element = nullptr;
        for (auto iter : _targets) {
            if (iter.first == target) {
                element = iter.second;
            }
        }

        if (element) {
            element->paused = true;
        }
    }

    void ActionManager::resumeTarget(HObject *target) {
        ActionEntry *element = nullptr;
        for (auto iter : _targets) {
            if (iter.first == target) {
                element = iter.second;
            }
        }

        if (element) {
            element->paused = false;
        }
    }

    std::vector<HObject*> ActionManager::pauseAllRunningActions() {
        std::vector<HObject*> idsWithActions;

        for (auto iter : _targets) {
            if (!iter.second->paused) {
                iter.second->paused = true;
                idsWithActions.push_back(iter.first);
            }
        }

        return idsWithActions;
    }

    void ActionManager::resumeTargets(const std::vector<HObject*>& targetsToResume) {
        for(const auto &HObject : targetsToResume) {
            this->resumeTarget(HObject);
        }
    }

    void ActionManager::addAction(Action *action, HObject *target, bool paused) {
        ActionEntry *element = nullptr;
        for (auto iter : _targets) {
            if (iter.first == target) {
                element = iter.second;
            }
        }

        if (! element) {
            element = new ActionEntry;
            element->paused = paused;
            target->retain();
            element->target = target;
            _targets.insert(std::unordered_map<HObject *, ActionEntry *>::value_type(target, element));
        }

         actionAllocWithHashElement(element);

         element->actions->appendObject(action);
         action->startWithTarget(target);
    }

    void ActionManager::removeAllActions() {
        for (auto iter : _targets) {
            removeAllActionsFromTarget(iter.first);
        }
    }

    void ActionManager::removeAllActionsFromTarget(HObject *target) {
        if (target == nullptr) {
            return;
        }

        ActionEntry *element = nullptr;
        for (auto iter : _targets) {
            if (iter.first == target) {
                element = iter.second;
            }
        }

        if (element) {
            if (element->actions->containsObject(element->currentAction) && (! element->currentActionSalvaged)) {
                element->currentAction->retain();
                element->currentActionSalvaged = true;
            }

            element->actions->removeAllObjects();
            if (_currentTarget == element) {
                _currentTargetSalvaged = true;
            }
            else {
                deleteHashElement(element);
            }
        }
    }

    void ActionManager::removeAction(Action *action) {
        if (action == nullptr) {
            return;
        }

        ActionEntry *element = nullptr;
        HObject *target = action->getOriginalTarget();
        for (auto iter : _targets) {
            if (iter.first == target) {
                element = iter.second;
            }
        }

        if (element) {
            auto i = element->actions->getIndexOfObject(action);
            if (i != -1) {
                removeActionAtIndex(i, element);
            }
        }
    }

    void ActionManager::removeActionByTag(int tag, HObject *target) {
        ActionEntry *element = nullptr;
        for (auto iter : _targets) {
            if (iter.first == target) {
                element = iter.second;
            }
        }

        if (element) {
            auto limit = element->actions->number();
            for (int i = 0; i < limit; ++i) {
                Action *action = (Action*)(*element->actions)[i];

                if (action->getTag() == (int)tag && action->getOriginalTarget() == target) {
                    removeActionAtIndex(i, element);
                    break;
                }
            }
        }
    }

    void ActionManager::removeAllActionsByTag(int tag, HObject *target)
    {
        ActionEntry *element = nullptr;
        for (auto iter : _targets) {
            if (iter.first == target) {
                element = iter.second;
            }
        }

        if (element) {
            auto limit = element->actions->number();
            for (int i = 0; i < limit;) {
                Action *action = (Action*)(*element->actions)[i];

                if (action->getTag() == (int)tag && action->getOriginalTarget() == target) {
                    removeActionAtIndex(i, element);
                    --limit;
                }
                else {
                    ++i;
                }
            }
        }
    }

    Action* ActionManager::getActionByTag(int tag, const HObject *target) const
    {
        ActionEntry *element = nullptr;
        for (auto iter : _targets) {
            if (iter.first == target) {
                element = iter.second;
            }
        }

        if (element) {
            if (element->actions != nullptr) {
                auto limit = element->actions->number();
                for (int i = 0; i < limit; ++i) {
                    Action *action = (Action*)(*element->actions)[i];

                    if (action->getTag() == (int)tag) {
                        return action;
                    }
                }
            }
        }

        return nullptr;
    }

    ssize_t ActionManager::getNumberOfRunningActionsInTarget(const HObject *target) const
    {
        ActionEntry *element = nullptr;
        for (auto iter : _targets) {
            if (iter.first == target) {
                element = iter.second;
            }
        }

        if (element) {
            return element->actions ? element->actions->number() : 0;
        }

        return 0;
    }

    void ActionManager::update(float dt) {
        for (auto iter : _targets) {
            _currentTarget = iter.second;
            _currentTargetSalvaged = false;

            if (! _currentTarget->paused) {
                for (_currentTarget->actionIndex = 0; _currentTarget->actionIndex < _currentTarget->actions->number();
                    _currentTarget->actionIndex++) {
                    _currentTarget->currentAction = (Action*)(*_currentTarget->actions)[_currentTarget->actionIndex];
                    if (_currentTarget->currentAction == nullptr) {
                        continue;
                    }

                    _currentTarget->currentActionSalvaged = false;

                    _currentTarget->currentAction->step(dt);

                    if (_currentTarget->currentActionSalvaged)
                    {
                        // The currentAction told the HObject to remove it. To prevent the action from
                        // accidentally deallocating itself before finishing its step, we retained
                        // it. Now that step is done, it's safe to release it.
                        _currentTarget->currentAction->release();
                    }
                    else if (_currentTarget->currentAction->isDone()) {
                        _currentTarget->currentAction->stop();

                        Action *action = _currentTarget->currentAction;
                        _currentTarget->currentAction = nullptr;
                        removeAction(action);
                    }

                    _currentTarget->currentAction = nullptr;
                }
            }

            // only delete currentTarget if no actions were scheduled during the cycle (issue #481)
            if (_currentTargetSalvaged && _currentTarget->actions->number() == 0) {
                deleteHashElement(_currentTarget);
            }
        }

        _currentTarget = nullptr;
    }
}
