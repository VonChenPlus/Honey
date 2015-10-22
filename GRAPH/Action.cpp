#include "GRAPH/Action.h"
#include "GRAPH/Director.h"
#include "MATH/Size.h"
#include "GRAPH/Node.h"

namespace GRAPH
{
    Action::Action()
        :originalTarget_(nullptr)
        ,target_(nullptr)
        ,tag_(Action::INVALID_TAG) {

    }

    Action::~Action() {

    }

    void Action::startWithTarget(Node *aTarget) {
        originalTarget_ = target_ = aTarget;
    }

    void Action::stop() {
        target_ = nullptr;
    }

    bool Action::isDone() const {
        return true;
    }

    void Action::step(float) {
    }

    void Action::update(float) {
    }

    bool ActionInterval::initWithDuration(float d)
    {
        duration_ = d;

        // prevent division by 0
        // This comparison could be in step:, but it might decrease the performance
        // by 3% in heavy based action games.
        if (duration_ == 0)
        {
            duration_ = MATH::MATH_FLOAT_EPSILON();
        }

        elapsed_ = 0;
        firstTick_ = true;

        return true;
    }

    bool ActionInterval::isDone() const
    {
        return elapsed_ >= duration_;
    }

    void ActionInterval::step(float dt)
    {
        if (firstTick_)
        {
            firstTick_ = false;
            elapsed_ = 0;
        }
        else
        {
            elapsed_ += dt;
        }

        this->update(MATH::MATH_MAX (0.0f,                                  // needed for rewind. elapsed could be negative
                          MATH::MATH_MIN(1.0f, elapsed_ /
                              MATH::MATH_MAX(duration_, MATH::MATH_FLOAT_MAX())   // division by 0
                              )
                          )
                     );
    }

    void ActionInterval::setAmplitudeRate(float)
    {
    }

    float ActionInterval::getAmplitudeRate()
    {
        return 0;
    }

    void ActionInterval::startWithTarget(Node *target)
    {
        FiniteTimeAction::startWithTarget(target);
        elapsed_ = 0.0f;
        firstTick_ = true;
    }

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
            endScaleX_ = s;
            endScaleY_ = s;
            endScaleZ_ = s;

            return true;
        }

        return false;
    }

    bool ScaleTo::initWithDuration(float duration, float sx, float sy)
    {
        if (ActionInterval::initWithDuration(duration))
        {
            endScaleX_ = sx;
            endScaleY_ = sy;
            endScaleZ_ = 1.f;

            return true;
        }

        return false;
    }

    bool ScaleTo::initWithDuration(float duration, float sx, float sy, float sz)
    {
        if (ActionInterval::initWithDuration(duration))
        {
            endScaleX_ = sx;
            endScaleY_ = sy;
            endScaleZ_ = sz;

            return true;
        }

        return false;
    }

    void ScaleTo::startWithTarget(Node *target)
    {
        ActionInterval::startWithTarget(target);
        startScaleX_ = target->getScaleX();
        startScaleY_ = target->getScaleY();
        startScaleZ_ = target->getScaleZ();
        deltaX_ = endScaleX_ - startScaleX_;
        deltaY_ = endScaleY_ - startScaleY_;
        deltaZ_ = endScaleZ_ - startScaleZ_;
    }

    void ScaleTo::update(float time)
    {
        if (target_)
        {
            target_->setScaleX(startScaleX_ + deltaX_ * time);
            target_->setScaleY(startScaleY_ + deltaY_ * time);
            target_->setScaleZ(startScaleZ_ + deltaZ_ * time);
        }
    }

    Follow::Follow()
        : followedObject_(nullptr)
        , boundarySet_(false)
        , boundaryFullyCovered_(false)
        , leftBoundary_(0.0)
        , rightBoundary_(0.0)
        , topBoundary_(0.0)
        , bottomBoundary_(0.0)
        , worldRect_(MATH::RectfZERO) {

    }

    Follow::~Follow() {
        SAFE_RELEASE(followedObject_);
    }

    Follow* Follow::create(HObject *followedHObject, const MATH::Rectf& rect) {
        Follow *follow = new (std::nothrow) Follow();
        if (follow && follow->initWithTarget(followedHObject, rect)) {
            follow->autorelease();
            return follow;
        }
        SAFE_DELETE(follow);
        return nullptr;
    }

    bool Follow::initWithTarget(HObject *followedHObject, const MATH::Rectf& rect) {
        followedHObject->retain();
        followedObject_ = followedHObject;
        worldRect_ = rect;
        boundarySet_ = !rect.equals(MATH::RectfZERO);
        boundaryFullyCovered_ = false;

        MATH::Sizef winSize = Director::getInstance().getWinSize();
        fullScreenSize_.set(winSize.width, winSize.height);
        halfScreenSize_ = fullScreenSize_ * 0.5f;

        if (boundarySet_) {
            leftBoundary_ = -((rect.origin.x+rect.size.width) - fullScreenSize_.x);
            rightBoundary_ = -rect.origin.x ;
            topBoundary_ = -rect.origin.y;
            bottomBoundary_ = -((rect.origin.y+rect.size.height) - fullScreenSize_.y);

            if(rightBoundary_ < leftBoundary_) {
                // screen width is larger than world's boundary width
                //set both in the middle of the world
                rightBoundary_ = leftBoundary_ = (leftBoundary_ + rightBoundary_) / 2;
            }
            if(topBoundary_ < bottomBoundary_) {
                // screen width is larger than world's boundary width
                //set both in the middle of the world
                topBoundary_ = bottomBoundary_ = (topBoundary_ + bottomBoundary_) / 2;
            }

            if( (topBoundary_ == bottomBoundary_) && (leftBoundary_ == rightBoundary_) ) {
                boundaryFullyCovered_ = true;
            }
        }

        return true;
    }

    void Follow::step(float) {
        if(boundarySet_) {
            // whole map fits inside a single screen, no need to modify the position - unless map boundaries are increased
            if(boundaryFullyCovered_) {
                return;
            }

            MATH::Vector2f tempPos = halfScreenSize_ - dynamic_cast<Node *>(followedObject_)->getPosition();

            dynamic_cast<Node *>(target_)->setPosition(MATH_CLAMP(tempPos.x, leftBoundary_, rightBoundary_),
                                       MATH_CLAMP(tempPos.y, bottomBoundary_, topBoundary_));
        }
        else {
            dynamic_cast<Node *>(target_)->setPosition(halfScreenSize_ - dynamic_cast<Node *>(followedObject_)->getPosition());
        }
    }

    bool Follow::isDone() const {
        return ( !dynamic_cast<Node *>(followedObject_)->isRunning() );
    }

    void Follow::stop() {
        target_ = nullptr;
        Action::stop();
    }

    ActionManager::ActionManager()
    : currentTarget_(nullptr),
      currentTargetSalvaged_(false) {

    }

    ActionManager::~ActionManager() {
        removeAllActions();
    }

    void ActionManager::deleteHashElement(ActionEntry *element) {
        SAFE_DELETE(element->actions);
        targets_.erase(element->target);
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

    void ActionManager::removeActionAtIndex(uint64 index, ActionEntry *element) {
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
            if (currentTarget_ == element) {
                currentTargetSalvaged_ = true;
            }
            else {
                deleteHashElement(element);
            }
        }
    }

    void ActionManager::pauseTarget(Node *target) {
        ActionEntry *element = nullptr;
        for (auto iter : targets_) {
            if (iter.first == target) {
                element = iter.second;
            }
        }

        if (element) {
            element->paused = true;
        }
    }

    void ActionManager::resumeTarget(Node *target) {
        ActionEntry *element = nullptr;
        for (auto iter : targets_) {
            if (iter.first == target) {
                element = iter.second;
            }
        }

        if (element) {
            element->paused = false;
        }
    }

    std::vector<Node*> ActionManager::pauseAllRunningActions() {
        std::vector<Node*> idsWithActions;

        for (auto iter : targets_) {
            if (!iter.second->paused) {
                iter.second->paused = true;
                idsWithActions.push_back(iter.first);
            }
        }

        return idsWithActions;
    }

    void ActionManager::resumeTargets(const std::vector<Node*>& targetsToResume) {
        for(const auto &object : targetsToResume) {
            this->resumeTarget(object);
        }
    }

    void ActionManager::addAction(Action *action, Node *target, bool paused) {
        ActionEntry *element = nullptr;
        for (auto iter : targets_) {
            if (iter.first == target) {
                element = iter.second;
            }
        }

        if (! element) {
            element = new ActionEntry;
            element->paused = paused;
            target->retain();
            element->target = target;
            targets_.insert(std::unordered_map<Node *, ActionEntry *>::value_type(target, element));
        }

         actionAllocWithHashElement(element);

         element->actions->appendObject(action);
         action->startWithTarget(target);
    }

    void ActionManager::removeAllActions() {
        for (auto iter : targets_) {
            removeAllActionsFromTarget(iter.first);
        }
    }

    void ActionManager::removeAllActionsFromTarget(Node *target) {
        if (target == nullptr) {
            return;
        }

        ActionEntry *element = nullptr;
        for (auto iter : targets_) {
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
            if (currentTarget_ == element) {
                currentTargetSalvaged_ = true;
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
        for (auto iter : targets_) {
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

    void ActionManager::removeActionByTag(int tag, Node *target) {
        ActionEntry *element = nullptr;
        for (auto iter : targets_) {
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

    void ActionManager::removeAllActionsByTag(int tag, Node *target)
    {
        ActionEntry *element = nullptr;
        for (auto iter : targets_) {
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

    Action* ActionManager::getActionByTag(int tag, const Node *target) const
    {
        ActionEntry *element = nullptr;
        for (auto iter : targets_) {
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

    uint64 ActionManager::getNumberOfRunningActionsInTarget(const Node *target) const
    {
        ActionEntry *element = nullptr;
        for (auto iter : targets_) {
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
        for (auto iter : targets_) {
            currentTarget_ = iter.second;
            currentTargetSalvaged_ = false;

            if (! currentTarget_->paused) {
                for (currentTarget_->actionIndex = 0; currentTarget_->actionIndex < currentTarget_->actions->number();
                    currentTarget_->actionIndex++) {
                    currentTarget_->currentAction = (Action*)(*currentTarget_->actions)[currentTarget_->actionIndex];
                    if (currentTarget_->currentAction == nullptr) {
                        continue;
                    }

                    currentTarget_->currentActionSalvaged = false;

                    currentTarget_->currentAction->step(dt);

                    if (currentTarget_->currentActionSalvaged)
                    {
                        // The currentAction told the HObject to remove it. To prevent the action from
                        // accidentally deallocating itself before finishing its step, we retained
                        // it. Now that step is done, it's safe to release it.
                        currentTarget_->currentAction->release();
                    }
                    else if (currentTarget_->currentAction->isDone()) {
                        currentTarget_->currentAction->stop();

                        Action *action = currentTarget_->currentAction;
                        currentTarget_->currentAction = nullptr;
                        removeAction(action);
                    }

                    currentTarget_->currentAction = nullptr;
                }
            }

            // only delete currentTarget if no actions were scheduled during the cycle (issue #481)
            if (currentTargetSalvaged_ && currentTarget_->actions->number() == 0) {
                deleteHashElement(currentTarget_);
            }
        }

        currentTarget_ = nullptr;
    }
}
