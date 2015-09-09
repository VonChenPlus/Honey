#include "GRAPH/Scheduler.h"
#include "MATH/MathDef.h"

namespace GRAPH
{
    Timer::Timer()
    : _scheduler(nullptr)
    , _elapsed(-1)
    , _runForever(false)
    , _useDelay(false)
    , _timesExecuted(0)
    , _repeat(0)
    , _delay(0.0f)
    , _interval(0.0f)
    {
    }

    void Timer::setupTimerWithInterval(float seconds, unsigned int repeat, float delay)
    {
        _elapsed = -1;
        _interval = seconds;
        _delay = delay;
        _useDelay = (_delay > 0.0f) ? true : false;
        _repeat = repeat;
        _runForever = (_repeat == MATH::MATH_UINT32_MAX()) ? true : false;
    }

    void Timer::update(float dt)
    {
        if (_elapsed == -1)
        {
            _elapsed = 0;
            _timesExecuted = 0;
        }
        else
        {
            if (_runForever && !_useDelay)
            {//standard timer usage
                _elapsed += dt;
                if (_elapsed >= _interval)
                {
                    trigger();

                    _elapsed = 0;
                }
            }
            else
            {//advanced usage
                _elapsed += dt;
                if (_useDelay)
                {
                    if( _elapsed >= _delay )
                    {
                        trigger();

                        _elapsed = _elapsed - _delay;
                        _timesExecuted += 1;
                        _useDelay = false;
                    }
                }
                else
                {
                    if (_elapsed >= _interval)
                    {
                        trigger();

                        _elapsed = 0;
                        _timesExecuted += 1;

                    }
                }

                if (!_runForever && _timesExecuted > _repeat)
                {    //unschedule timer
                    cancel();
                }
            }
        }
    }


    // TimerTargetSelector

    TimerTargetSelector::TimerTargetSelector()
    : _target(nullptr)
    , _selector(nullptr)
    {
    }

    bool TimerTargetSelector::initWithSelector(Scheduler* scheduler, CallFuncF selector, HObject* target, float seconds, unsigned int repeat, float delay)
    {
        _scheduler = scheduler;
        _target = target;
        _selector = selector;
        setupTimerWithInterval(seconds, repeat, delay);
        return true;
    }

    void TimerTargetSelector::trigger()
    {
        if (_target && _selector)
        {
            (_target->*_selector)(_elapsed);
        }
    }

    void TimerTargetSelector::cancel()
    {
        _scheduler->unschedule(_selector, _target);
    }

    // TimerTargetCallback

    TimerTargetCallback::TimerTargetCallback()
    : _target(nullptr)
    , _callback(nullptr)
    {
    }

    bool TimerTargetCallback::initWithCallback(Scheduler* scheduler, const SchedulerFunc& callback, void *target, const std::string& key, float seconds, unsigned int repeat, float delay)
    {
        _scheduler = scheduler;
        _target = target;
        _callback = callback;
        _key = key;
        setupTimerWithInterval(seconds, repeat, delay);
        return true;
    }

    void TimerTargetCallback::trigger()
    {
        if (_callback)
        {
            _callback(_elapsed);
        }
    }

    void TimerTargetCallback::cancel()
    {
        _scheduler->unschedule(_key, _target);
    }

    // implementation of Scheduler

    // Priority level reserved for system services.
    const int Scheduler::PRIORITY_SYSTEM = MATH::MATH_INT32_MIN();

    // Minimum priority level for user scheduling.
    const int Scheduler::PRIORITY_NON_SYSTEM_MIN = PRIORITY_SYSTEM + 1;

    Scheduler::Scheduler(void)
    : _timeScale(1.0f)
    , _currentTargetSalvaged(false)
    , _updateHashLocked(false)
    {
        // I don't expect to have more than 30 functions to all per frame
        _functionsToPerform.reserve(30);
    }

    Scheduler::~Scheduler(void)
    {
        unscheduleAll();
    }

    void Scheduler::removeHashElement(TimerEntry *element)
    {
        delete element->timers;
        _hashForTimers.erase(element->target);
        free(element);
    }

    void Scheduler::schedule(const SchedulerFunc& callback, void *target, float interval, bool paused, const std::string& key)
    {
        this->schedule(callback, target, interval, MATH::MATH_UINT32_MAX(), 0.0f, paused, key);
    }

    void Scheduler::schedule(const SchedulerFunc& callback, void *target, float interval, unsigned int repeat, float delay, bool paused, const std::string& key)
    {
        TimerEntry *element = nullptr;
        if (_hashForTimers.find(target) != _hashForTimers.end())
            element = _hashForTimers[target];

        if (! element)
        {
            element = (TimerEntry *)calloc(sizeof(*element), 1);
            element->target = target;

            _hashForTimers.insert(std::unordered_map<void *, TimerEntry *>::value_type(target, element));

            // Is this the 1st element ? Then set the pause level to all the selectors of this target
            element->paused = paused;
        }

        if (element->timers == nullptr)
        {
            element->timers = new HObjectArray(10);
        }
        else
        {
            for (int i = 0; i < element->timers->number(); ++i)
            {
                TimerTargetCallback *timer = dynamic_cast<TimerTargetCallback*>((*element->timers)[i]);

                if (timer && key == timer->getKey())
                {
                    timer->setInterval(interval);
                    return;
                }
            }

            element->timers->ensureExtraCapacity(1);
        }

        TimerTargetCallback *timer = new (std::nothrow) TimerTargetCallback();
        timer->initWithCallback(this, callback, target, key, interval, repeat, delay);
        element->timers->appendObject(timer);
        timer->release();
    }

    void Scheduler::unschedule(const std::string &key, void *target)
    {
        // explicity handle nil arguments when removing an object
        if (target == nullptr || key.empty())
        {
            return;
        }

        TimerEntry *element = nullptr;
        if (_hashForTimers.find(target) != _hashForTimers.end())
            element = _hashForTimers[target];

        if (element)
        {
            for (int i = 0; i < element->timers->number(); ++i)
            {
                TimerTargetCallback *timer = static_cast<TimerTargetCallback*>((*element->timers)[i]);

                if (key == timer->getKey())
                {
                    if (timer == element->currentTimer && (! element->currentTimerSalvaged))
                    {
                        element->currentTimer->retain();
                        element->currentTimerSalvaged = true;
                    }

                    element->timers->removeObjectAtIndex(i, true);

                    // update timerIndex in case we are in tick:, looping over the actions
                    if (element->timerIndex >= i)
                    {
                        element->timerIndex--;
                    }

                    if (element->timers->number() == 0)
                    {
                        if (_currentTarget == element)
                        {
                            _currentTargetSalvaged = true;
                        }
                        else
                        {
                            removeHashElement(element);
                        }
                    }

                    return;
                }
            }
        }
    }

    void Scheduler::priorityIn(std::list<ListEntry *> *list, const SchedulerFunc& callback, void *target, int priority, bool paused)
    {
        ListEntry *listElement = new ListEntry();

        listElement->callback = callback;
        listElement->target = target;
        listElement->priority = priority;
        listElement->paused = paused;
        listElement->markedForDeletion = false;

        // empty list ?
        if (list->empty())
        {
            list->push_back(listElement);
        }
        else
        {
            bool added = false;

            for (auto element : *list)
            {
                if (priority < element->priority)
                {
                    list->insert(element, listElement);
                    added = true;
                    break;
                }
            }

            // Not added? priority has the higher value. Append it.
            if (! added)
            {
                list->push_back(listElement);
            }
        }

        // update hash entry for quick access
        UpdateEntry *hashElement = (UpdateEntry *)calloc(sizeof(*hashElement), 1);
        hashElement->target = target;
        hashElement->list = &list;
        hashElement->entry = listElement;
        _hashForUpdates.insert(std::unordered_map<void *, TimerEntry *>::value_type(target, hashElement));
    }

    void Scheduler::appendIn(std::list<ListEntry *> *list, const SchedulerFunc& callback, void *target, bool paused)
    {
        ListEntry *listElement = new ListEntry();

        listElement->callback = callback;
        listElement->target = target;
        listElement->paused = paused;
        listElement->priority = 0;
        listElement->markedForDeletion = false;

        list->push_back(listElement);

        // update hash entry for quicker access
        UpdateEntry *hashElement = (UpdateEntry *)calloc(sizeof(*hashElement), 1);
        hashElement->target = target;
        hashElement->list = list;
        hashElement->entry = listElement;
        _hashForUpdates.insert(std::unordered_map<void *, TimerEntry *>::value_type(target, hashElement));
    }

    void Scheduler::schedulePerFrame(const SchedulerFunc& callback, void *target, int priority, bool paused)
    {
        UpdateEntry *hashElement = nullptr;
        if (_hashForUpdates.find(target) != _hashForUpdates.end())
            hashElement = _hashForUpdates[target];
        if (hashElement)
        {
            // check if priority has changed
            if ((*hashElement->list)->priority != priority)
            {
                if (_updateHashLocked)
                {
                    hashElement->entry->markedForDeletion = false;
                    hashElement->entry->paused = paused;
                    return;
                }
                else
                {
                    // will be added again outside if (hashElement).
                    unscheduleUpdate(target);
                }
            }
            else
            {
                hashElement->entry->markedForDeletion = false;
                hashElement->entry->paused = paused;
                return;
            }
        }

        // most of the updates are going to be 0, that's way there
        // is an special list for updates with priority 0
        if (priority == 0)
        {
            appendIn(&_updates0List, callback, target, paused);
        }
        else if (priority < 0)
        {
            priorityIn(&_updatesNegList, callback, target, priority, paused);
        }
        else
        {
            // priority > 0
            priorityIn(&_updatesPosList, callback, target, priority, paused);
        }
    }

    bool Scheduler::isScheduled(const std::string& key, void *target)
    {
        TimerEntry *element = nullptr;
        if (_hashForTimers.find(target) != _hashForTimers.end())
            hashElement = _hashForTimers[target];

        if (!element)
        {
            return false;
        }

        if (element->timers == nullptr)
        {
            return false;
        }
        else
        {
            for (int i = 0; i < element->timers->number(); ++i)
            {
                TimerTargetCallback *timer = static_cast<TimerTargetCallback*>((*element->timers)[i]);

                if (key == timer->getKey())
                {
                    return true;
                }
            }

            return false;
        }

        return false;  // should never get here
    }

    void Scheduler::removeUpdateFromHash(UpdateEntry *entry)
    {
        UpdateEntry *element = nullptr;
        if (_hashForUpdates.find(entry->target) != _hashForUpdates.end())
            hashElement = _hashForUpdates[target];
        if (element)
        {
            // list entry
            (*element->list)->erase(element->entry);
            SAFE_DELETE(element->entry);

            // hash entry
            _hashForUpdates.erase(element);
            free(element);
        }
    }

    void Scheduler::unscheduleUpdate(void *target)
    {
        if (target == nullptr)
        {
            return;
        }

        UpdateEntry *element = nullptr;
        HASH_FIND_PTR(_hashForUpdates, &target, element);
        if (element)
        {
            if (_updateHashLocked)
            {
                element->entry->markedForDeletion = true;
            }
            else
            {
                this->removeUpdateFromHash(element->entry);
            }
        }
    }

    void Scheduler::unscheduleAll(void)
    {
        unscheduleAllWithMinPriority(PRIORITY_SYSTEM);
    }

    void Scheduler::unscheduleAllWithMinPriority(int minPriority)
    {
        // Custom Selectors
        tHashTimerEntry *element = nullptr;
        tHashTimerEntry *nextElement = nullptr;
        for (element = _hashForTimers; element != nullptr;)
        {
            // element may be removed in unscheduleAllSelectorsForTarget
            nextElement = (tHashTimerEntry *)element->hh.next;
            unscheduleAllForTarget(element->target);

            element = nextElement;
        }

        // Updates selectors
        if(minPriority < 0)
        {
            for (auto entry : _updatesNegList) {
                if(entry->priority >= minPriority)
                {
                    unscheduleUpdate(entry->target);
                }
            }
        }

        if(minPriority <= 0)
        {
            for (auto entry : _updates0List) {
                unscheduleUpdate(entry->target);
            }
        }

        for (auto entry : _updatesPosList) {
            if(entry->priority >= minPriority)
            {
                unscheduleUpdate(entry->target);
            }
        }
    }

    void Scheduler::unscheduleAllForTarget(void *target)
    {
        // explicit nullptr handling
        if (target == nullptr)
        {
            return;
        }

        // Custom Selectors
        tHashTimerEntry *element = nullptr;
        HASH_FIND_PTR(_hashForTimers, &target, element);

        if (element)
        {
            if (element->timers->containsObject(element->currentTimer)
                && (! element->currentTimerSalvaged))
            {
                element->currentTimer->retain();
                element->currentTimerSalvaged = true;
            }
            element->timers->removeAllObjects();

            if (_currentTarget == element)
            {
                _currentTargetSalvaged = true;
            }
            else
            {
                removeHashElement(element);
            }
        }

        // update selector
        unscheduleUpdate(target);
    }

    void Scheduler::resumeTarget(void *target)
    {
        // custom selectors
        tHashTimerEntry *element = nullptr;
        HASH_FIND_PTR(_hashForTimers, &target, element);
        if (element)
        {
            element->paused = false;
        }

        // update selector
        UpdateEntry *elementUpdate = nullptr;
        HASH_FIND_PTR(_hashForUpdates, &target, elementUpdate);
        if (elementUpdate)
        {
            elementUpdate->entry->paused = false;
        }
    }

    void Scheduler::pauseTarget(void *target)
    {
        // custom selectors
        tHashTimerEntry *element = nullptr;
        HASH_FIND_PTR(_hashForTimers, &target, element);
        if (element)
        {
            element->paused = true;
        }

        // update selector
        UpdateEntry *elementUpdate = nullptr;
        HASH_FIND_PTR(_hashForUpdates, &target, elementUpdate);
        if (elementUpdate)
        {
            elementUpdate->entry->paused = true;
        }
    }

    bool Scheduler::isTargetPaused(void *target)
    {
        // Custom selectors
        tHashTimerEntry *element = nullptr;
        HASH_FIND_PTR(_hashForTimers, &target, element);
        if( element )
        {
            return element->paused;
        }

        // We should check update selectors if target does not have custom selectors
        UpdateEntry *elementUpdate = nullptr;
        HASH_FIND_PTR(_hashForUpdates, &target, elementUpdate);
        if ( elementUpdate )
        {
            return elementUpdate->entry->paused;
        }

        return false;  // should never get here
    }

    std::set<void*> Scheduler::pauseAllTargets()
    {
        return pauseAllTargetsWithMinPriority(PRIORITY_SYSTEM);
    }

    std::set<void*> Scheduler::pauseAllTargetsWithMinPriority(int minPriority)
    {
        std::set<void*> idsWithSelectors;

        // Custom Selectors
        for(tHashTimerEntry *element = _hashForTimers; element != nullptr;
            element = (tHashTimerEntry*)element->hh.next)
        {
            element->paused = true;
            idsWithSelectors.insert(element->target);
        }

        // Updates selectors
        tListEntry *entry, *tmp;
        if(minPriority < 0)
        {
            DL_FOREACH_SAFE( _updatesNegList, entry, tmp )
            {
                if(entry->priority >= minPriority)
                {
                    entry->paused = true;
                    idsWithSelectors.insert(entry->target);
                }
            }
        }

        if(minPriority <= 0)
        {
            DL_FOREACH_SAFE( _updates0List, entry, tmp )
            {
                entry->paused = true;
                idsWithSelectors.insert(entry->target);
            }
        }

        DL_FOREACH_SAFE( _updatesPosList, entry, tmp )
        {
            if(entry->priority >= minPriority)
            {
                entry->paused = true;
                idsWithSelectors.insert(entry->target);
            }
        }

        return idsWithSelectors;
    }

    void Scheduler::resumeTargets(const std::set<void*>& targetsToResume)
    {
        for(const auto &obj : targetsToResume) {
            this->resumeTarget(obj);
        }
    }

    void Scheduler::performFunctionInCocosThread(const std::function<void ()> &function)
    {
        _performMutex.lock();

        _functionsToPerform.push_back(function);

        _performMutex.unlock();
    }

    // main loop
    void Scheduler::update(float dt)
    {
        _updateHashLocked = true;

        if (_timeScale != 1.0f)
        {
            dt *= _timeScale;
        }

        //
        // Selector callbacks
        //

        // Iterate over all the Updates' selectors
        tListEntry *entry, *tmp;

        // updates with priority < 0
        DL_FOREACH_SAFE(_updatesNegList, entry, tmp)
        {
            if ((! entry->paused) && (! entry->markedForDeletion))
            {
                entry->callback(dt);
            }
        }

        // updates with priority == 0
        DL_FOREACH_SAFE(_updates0List, entry, tmp)
        {
            if ((! entry->paused) && (! entry->markedForDeletion))
            {
                entry->callback(dt);
            }
        }

        // updates with priority > 0
        DL_FOREACH_SAFE(_updatesPosList, entry, tmp)
        {
            if ((! entry->paused) && (! entry->markedForDeletion))
            {
                entry->callback(dt);
            }
        }

        // Iterate over all the custom selectors
        for (tHashTimerEntry *elt = _hashForTimers; elt != nullptr; )
        {
            _currentTarget = elt;
            _currentTargetSalvaged = false;

            if (! _currentTarget->paused)
            {
                // The 'timers' array may change while inside this loop
                for (elt->timerIndex = 0; elt->timerIndex < elt->timers->number(); ++(elt->timerIndex))
                {
                    elt->currentTimer = (Timer*)((*elt->timers)[elt->timerIndex]);
                    elt->currentTimerSalvaged = false;

                    elt->currentTimer->update(dt);

                    if (elt->currentTimerSalvaged)
                    {
                        // The currentTimer told the remove itself. To prevent the timer from
                        // accidentally deallocating itself before finishing its step, we retained
                        // it. Now that step is done, it's safe to release it.
                        elt->currentTimer->release();
                    }

                    elt->currentTimer = nullptr;
                }
            }

            // elt, at this moment, is still valid
            // so it is safe to ask this here (issue #490)
            elt = (tHashTimerEntry *)elt->hh.next;

            // only delete currentTarget if no actions were scheduled during the cycle (issue #481)
            if (_currentTargetSalvaged && _currentTarget->timers->number() == 0)
            {
                removeHashElement(_currentTarget);
            }
        }

        // delete all updates that are marked for deletion
        // updates with priority < 0
        DL_FOREACH_SAFE(_updatesNegList, entry, tmp)
        {
            if (entry->markedForDeletion)
            {
                this->removeUpdateFromHash(entry);
            }
        }

        // updates with priority == 0
        DL_FOREACH_SAFE(_updates0List, entry, tmp)
        {
            if (entry->markedForDeletion)
            {
                this->removeUpdateFromHash(entry);
            }
        }

        // updates with priority > 0
        DL_FOREACH_SAFE(_updatesPosList, entry, tmp)
        {
            if (entry->markedForDeletion)
            {
                this->removeUpdateFromHash(entry);
            }
        }

        _updateHashLocked = false;
        _currentTarget = nullptr;

        //
        // Functions allocated from another thread
        //

        // Testing size is faster than locking / unlocking.
        // And almost never there will be functions scheduled to be called.
        if( !_functionsToPerform.empty() ) {
            _performMutex.lock();
            // fixed #4123: Save the callback functions, they must be invoked after '_performMutex.unlock()', otherwise if new functions are added in callback, it will cause thread deadlock.
            auto temp = _functionsToPerform;
            _functionsToPerform.clear();
            _performMutex.unlock();
            for( const auto &function : temp ) {
                function();
            }

        }
    }

    void Scheduler::schedule(CallFuncF selector, HObject *target, float interval, unsigned int repeat, float delay, bool paused)
    {
        tHashTimerEntry *element = nullptr;
        HASH_FIND_PTR(_hashForTimers, &target, element);

        if (! element)
        {
            element = (tHashTimerEntry *)calloc(sizeof(*element), 1);
            element->target = target;

            HASH_ADD_PTR(_hashForTimers, target, element);

            // Is this the 1st element ? Then set the pause level to all the selectors of this target
            element->paused = paused;
        }

        if (element->timers == nullptr)
        {
            element->timers = new HObjectArray(10);
        }
        else
        {
            for (int i = 0; i < element->timers->number(); ++i)
            {
                TimerTargetSelector *timer = dynamic_cast<TimerTargetSelector*>((*element->timers)[i]);

                if (timer && selector == timer->getSelector())
                {
                    timer->setInterval(interval);
                    return;
                }
            }
            element->timers->ensureExtraCapacity(1);
        }

        TimerTargetSelector *timer = new (std::nothrow) TimerTargetSelector();
        timer->initWithSelector(this, selector, target, interval, repeat, delay);
        element->timers->appendObject(timer);
        timer->release();
    }

    void Scheduler::schedule(CallFuncF selector, HObject *target, float interval, bool paused)
    {
        this->schedule(selector, target, interval, MATH::MATH_UINT32_MAX(), 0.0f, paused);
    }

    bool Scheduler::isScheduled(CallFuncF selector, HObject *target)
    {
        tHashTimerEntry *element = nullptr;
        HASH_FIND_PTR(_hashForTimers, &target, element);

        if (!element)
        {
            return false;
        }

        if (element->timers == nullptr)
        {
            return false;
        }
        else
        {
            for (int i = 0; i < element->timers->number(); ++i)
            {
                TimerTargetSelector *timer = static_cast<TimerTargetSelector*>((*element->timers)[i]);

                if (selector == timer->getSelector())
                {
                    return true;
                }
            }

            return false;
        }

        return false;  // should never get here
    }

    void Scheduler::unschedule(CallFuncF selector, HObject *target)
    {
        // explicity handle nil arguments when removing an object
        if (target == nullptr || selector == nullptr)
        {
            return;
        }

        //CCASSERT(target);
        //CCASSERT(selector);

        tHashTimerEntry *element = nullptr;
        HASH_FIND_PTR(_hashForTimers, &target, element);

        if (element)
        {
            for (int i = 0; i < element->timers->number(); ++i)
            {
                TimerTargetSelector *timer = static_cast<TimerTargetSelector*>((*element->timers)[i]);

                if (selector == timer->getSelector())
                {
                    if (timer == element->currentTimer && (! element->currentTimerSalvaged))
                    {
                        element->currentTimer->retain();
                        element->currentTimerSalvaged = true;
                    }

                    element->timers->removeObjectAtIndex(i);

                    // update timerIndex in case we are in tick:, looping over the actions
                    if (element->timerIndex >= i)
                    {
                        element->timerIndex--;
                    }

                    if (element->timers->number() == 0)
                    {
                        if (_currentTarget == element)
                        {
                            _currentTargetSalvaged = true;
                        }
                        else
                        {
                            removeHashElement(element);
                        }
                    }

                    return;
                }
            }
        }
    }
}
