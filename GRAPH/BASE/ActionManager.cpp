#include "GRAPH/BASE/ActionManager.h"
#include "UTILS/HASH/uthash.h"
#include "BASE/HObject.h"
#include "GRAPH/BASE/Action.h"

namespace GRAPH
{
    //
    // singleton stuff
    //
    typedef struct _hashElement
    {
        HObjectArray     *actions;
        Node                *target;
        int                 actionIndex;
        Action              *currentAction;
        bool                currentActionSalvaged;
        bool                paused;
        UT_hash_handle      hh;
    } tHashElement;

    ActionManager::ActionManager()
    : _targets(nullptr),
      _currentTarget(nullptr),
      _currentTargetSalvaged(false)
    {

    }

    ActionManager::~ActionManager()
    {
        removeAllActions();
    }

    // private

    void ActionManager::deleteHashElement(tHashElement *element)
    {
        delete element->actions;
        HASH_DEL(_targets, element);
        element->target->release();
        free(element);
    }

    void ActionManager::actionAllocWithHashElement(tHashElement *element)
    {
        // 4 actions per Node by default
        if (element->actions == nullptr)
        {
            element->actions = new HObjectArray(4);
        }
        else if (element->actions->number() == element->actions->maximun())
        {
            element->actions->doubleCapacity();
        }

    }

    void ActionManager::removeActionAtIndex(ssize_t index, tHashElement *element)
    {
        Action *action = (Action*)(*element->actions)[index];

        if (action == element->currentAction && (! element->currentActionSalvaged))
        {
            element->currentAction->retain();
            element->currentActionSalvaged = true;
        }

        element->actions->removeObjectAtIndex(index, true);

        // update actionIndex in case we are in tick. looping over the actions
        if (element->actionIndex >= index)
        {
            element->actionIndex--;
        }

        if (element->actions->number() == 0)
        {
            if (_currentTarget == element)
            {
                _currentTargetSalvaged = true;
            }
            else
            {
                deleteHashElement(element);
            }
        }
    }

    // pause / resume

    void ActionManager::pauseTarget(Node *target)
    {
        tHashElement *element = nullptr;
        HASH_FIND_PTR(_targets, &target, element);
        if (element)
        {
            element->paused = true;
        }
    }

    void ActionManager::resumeTarget(Node *target)
    {
        tHashElement *element = nullptr;
        HASH_FIND_PTR(_targets, &target, element);
        if (element)
        {
            element->paused = false;
        }
    }

    std::vector<Node*> ActionManager::pauseAllRunningActions()
    {
        std::vector<Node*> idsWithActions;

        for (tHashElement *element=_targets; element != nullptr; element = (tHashElement *)element->hh.next)
        {
            if (! element->paused)
            {
                element->paused = true;
                idsWithActions.push_back(element->target);
            }
        }

        return idsWithActions;
    }

    void ActionManager::resumeTargets(const std::vector<Node*>& targetsToResume)
    {
        for(const auto &node : targetsToResume) {
            this->resumeTarget(node);
        }
    }

    // run

    void ActionManager::addAction(Action *action, Node *target, bool paused)
    {
        tHashElement *element = nullptr;
        // we should convert it to Ref*, because we save it as Ref*
        HObject *tmp = target;
        HASH_FIND_PTR(_targets, &tmp, element);
        if (! element)
        {
            element = (tHashElement*)calloc(sizeof(*element), 1);
            element->paused = paused;
            target->retain();
            element->target = target;
            HASH_ADD_PTR(_targets, target, element);
        }

         actionAllocWithHashElement(element);

         element->actions->appendObject(action);
         action->startWithTarget(target);
    }

    // remove

    void ActionManager::removeAllActions()
    {
        for (tHashElement *element = _targets; element != nullptr; )
        {
            auto target = element->target;
            element = (tHashElement*)element->hh.next;
            removeAllActionsFromTarget(target);
        }
    }

    void ActionManager::removeAllActionsFromTarget(Node *target)
    {
        // explicit null handling
        if (target == nullptr)
        {
            return;
        }

        tHashElement *element = nullptr;
        HASH_FIND_PTR(_targets, &target, element);
        if (element)
        {
            if (element->actions->containsObject(element->currentAction) && (! element->currentActionSalvaged))
            {
                element->currentAction->retain();
                element->currentActionSalvaged = true;
            }

            element->actions->removeAllObjects();
            if (_currentTarget == element)
            {
                _currentTargetSalvaged = true;
            }
            else
            {
                deleteHashElement(element);
            }
        }
    }

    void ActionManager::removeAction(Action *action)
    {
        // explicit null handling
        if (action == nullptr)
        {
            return;
        }

        tHashElement *element = nullptr;
        HObject *target = action->getOriginalTarget();
        HASH_FIND_PTR(_targets, &target, element);
        if (element)
        {
            auto i = element->actions->getIndexOfObject(action);
            if (i != -1)
            {
                removeActionAtIndex(i, element);
            }
        }
    }

    void ActionManager::removeActionByTag(int tag, Node *target)
    {
        tHashElement *element = nullptr;
        HASH_FIND_PTR(_targets, &target, element);

        if (element)
        {
            auto limit = element->actions->number();
            for (int i = 0; i < limit; ++i)
            {
                Action *action = (Action*)(*element->actions)[i];

                if (action->getTag() == (int)tag && action->getOriginalTarget() == target)
                {
                    removeActionAtIndex(i, element);
                    break;
                }
            }
        }
    }

    void ActionManager::removeAllActionsByTag(int tag, Node *target)
    {
        tHashElement *element = nullptr;
        HASH_FIND_PTR(_targets, &target, element);

        if (element)
        {
            auto limit = element->actions->number();
            for (int i = 0; i < limit;)
            {
                Action *action = (Action*)(*element->actions)[i];

                if (action->getTag() == (int)tag && action->getOriginalTarget() == target)
                {
                    removeActionAtIndex(i, element);
                    --limit;
                }
                else
                {
                    ++i;
                }
            }
        }
    }

    // get

    // FIXME: Passing "const O *" instead of "const O&" because HASH_FIND_IT requries the address of a pointer
    // and, it is not possible to get the address of a reference
    Action* ActionManager::getActionByTag(int tag, const Node *target) const
    {
        tHashElement *element = nullptr;
        HASH_FIND_PTR(_targets, &target, element);

        if (element)
        {
            if (element->actions != nullptr)
            {
                auto limit = element->actions->number();
                for (int i = 0; i < limit; ++i)
                {
                    Action *action = (Action*)(*element->actions)[i];

                    if (action->getTag() == (int)tag)
                    {
                        return action;
                    }
                }
            }
        }

        return nullptr;
    }

    // FIXME: Passing "const O *" instead of "const O&" because HASH_FIND_IT requries the address of a pointer
    // and, it is not possible to get the address of a reference
    ssize_t ActionManager::getNumberOfRunningActionsInTarget(const Node *target) const
    {
        tHashElement *element = nullptr;
        HASH_FIND_PTR(_targets, &target, element);
        if (element)
        {
            return element->actions ? element->actions->number() : 0;
        }

        return 0;
    }

    // main loop
    void ActionManager::update(float dt)
    {
        for (tHashElement *elt = _targets; elt != nullptr; )
        {
            _currentTarget = elt;
            _currentTargetSalvaged = false;

            if (! _currentTarget->paused)
            {
                // The 'actions' MutableArray may change while inside this loop.
                for (_currentTarget->actionIndex = 0; _currentTarget->actionIndex < _currentTarget->actions->number();
                    _currentTarget->actionIndex++)
                {
                    _currentTarget->currentAction = (Action*)(*_currentTarget->actions)[_currentTarget->actionIndex];
                    if (_currentTarget->currentAction == nullptr)
                    {
                        continue;
                    }

                    _currentTarget->currentActionSalvaged = false;

                    _currentTarget->currentAction->step(dt);

                    if (_currentTarget->currentActionSalvaged)
                    {
                        // The currentAction told the node to remove it. To prevent the action from
                        // accidentally deallocating itself before finishing its step, we retained
                        // it. Now that step is done, it's safe to release it.
                        _currentTarget->currentAction->release();
                    }
                    else
                    if (_currentTarget->currentAction->isDone())
                    {
                        _currentTarget->currentAction->stop();

                        Action *action = _currentTarget->currentAction;
                        // Make currentAction nil to prevent removeAction from salvaging it.
                        _currentTarget->currentAction = nullptr;
                        removeAction(action);
                    }

                    _currentTarget->currentAction = nullptr;
                }
            }

            // elt, at this moment, is still valid
            // so it is safe to ask this here (issue #490)
            elt = (tHashElement*)(elt->hh.next);

            // only delete currentTarget if no actions were scheduled during the cycle (issue #481)
            if (_currentTargetSalvaged && _currentTarget->actions->number() == 0)
            {
                deleteHashElement(_currentTarget);
            }
        }

        _currentTarget = nullptr;
    }
}
