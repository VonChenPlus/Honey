#include "GRAPH/Scheduler.h"
#include "MATH/MathDef.h"

namespace GRAPH
{
    Timer::Timer()
        : scheduler_(nullptr)
        , elapsed_(-1)
        , runForever_(false)
        , useDelay_(false)
        , timesExecuted_(0)
        , repeat_(0)
        , delay_(0.0f)
        , _interval(0.0f) {
    }

    void Timer::setupTimerWithInterval(float seconds, unsigned int repeat, float delay) {
        elapsed_ = -1;
        _interval = seconds;
        delay_ = delay;
        useDelay_ = (delay_ > 0.0f) ? true : false;
        repeat_ = repeat;
        runForever_ = (repeat_ == -1) ? true : false;
    }

    void Timer::update(float dt) {
        if (elapsed_ == -1) {
            elapsed_ = 0;
            timesExecuted_ = 0;
        }
        else {
            if (runForever_ && !useDelay_) {
                elapsed_ += dt;
                if (elapsed_ >= _interval) {
                    trigger();
                    elapsed_ = 0;
                }
            }
            else {
                elapsed_ += dt;
                if (useDelay_) {
                    if( elapsed_ >= delay_ ) {
                        trigger();
                        elapsed_ = elapsed_ - delay_;
                        timesExecuted_ += 1;
                        useDelay_ = false;
                    }
                }
                else {
                    if (elapsed_ >= _interval) {
                        trigger();
                        elapsed_ = 0;
                        timesExecuted_ += 1;
                    }
                }

                if (!runForever_ && timesExecuted_ > repeat_) {
                    cancel();
                }
            }
        }
    }

    TimerTargetSelector::TimerTargetSelector()
        : target_(nullptr)
        , selector_(nullptr) {
    }

    bool TimerTargetSelector::initWithSelector(Scheduler* scheduler, SelectorF selector, HObject* target, float seconds, unsigned int repeat, float delay) {
        scheduler_ = scheduler;
        target_ = target;
        selector_ = selector;
        setupTimerWithInterval(seconds, repeat, delay);
        return true;
    }

    void TimerTargetSelector::trigger() {
        if (target_ && selector_) {
            (target_->*selector_)(elapsed_);
        }
    }

    void TimerTargetSelector::cancel() {
        scheduler_->unschedule(selector_, target_);
    }

    TimerTargetCallback::TimerTargetCallback()
        : target_(nullptr)
        , callback_(nullptr) {
    }

    bool TimerTargetCallback::initWithCallback(Scheduler* scheduler, const SchedulerFunc& callback, void *target, const std::string& key, float seconds, unsigned int repeat, float delay) {
        scheduler_ = scheduler;
        target_ = target;
        callback_ = callback;
        key_ = key;
        setupTimerWithInterval(seconds, repeat, delay);
        return true;
    }

    void TimerTargetCallback::trigger() {
        if (callback_) {
            callback_(elapsed_);
        }
    }

    void TimerTargetCallback::cancel() {
        scheduler_->unschedule(key_, target_);
    }

    // Priority level reserved for system services.
    const int Scheduler::PRIORITY_SYSTEM = MATH::MATH_INT32_MIN();

    // Minimum priority level for user scheduling.
    const int Scheduler::PRIORITY_NON_SYSTEM_MIN = PRIORITY_SYSTEM + 1;

    Scheduler::Scheduler(void)
        : timeScale_(1.0f)
        , currentTimerSalvaged_(false)
        , updateMapLocked_(false) {
    }

    Scheduler::~Scheduler(void) {
        unscheduleAll();
    }

    void Scheduler::schedule(const SchedulerFunc& callback, void *target, float interval, bool paused, const std::string& key) {
        this->schedule(callback, target, interval, -1, 0.0f, paused, key);
    }

    void Scheduler::schedule(const SchedulerFunc& callback, void *target, float interval, unsigned int repeat, float delay, bool paused, const std::string& key) {
        TimerEntry *element = nullptr;
        if (timersMap_.find(target) != timersMap_.end())
            element = timersMap_[target];

        if (! element) {
            element = new TimerEntry;
            element->target = target;

            timersMap_.insert(std::unordered_map<void *, TimerEntry *>::value_type(target, element));

            // Is this the 1st element ? Then set the pause level to all the selectors of this target
            element->paused = paused;
        }

        if (element->timers == nullptr) {
            element->timers = new HObjectArray(10);
        }
        else {
            for (int i = 0; i < element->timers->number(); ++i) {
                TimerTargetCallback *timer = dynamic_cast<TimerTargetCallback*>((*element->timers)[i]);

                if (timer && key == timer->getKey()) {
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

    void Scheduler::unschedule(const std::string &key, void *target) {
        if (target == nullptr || key.empty()) {
            return;
        }

        TimerEntry *element = nullptr;
        if (timersMap_.find(target) != timersMap_.end())
            element = timersMap_[target];

        if (element) {
            for (int i = 0; i < element->timers->number(); ++i) {
                TimerTargetCallback *timer = static_cast<TimerTargetCallback*>((*element->timers)[i]);

                if (key == timer->getKey()) {
                    if (timer == element->currentTimer && (! element->currentTimerSalvaged)) {
                        element->currentTimer->retain();
                        element->currentTimerSalvaged = true;
                    }

                    element->timers->removeObjectAtIndex(i, true);

                    // update timerIndex in case we are in tick:, looping over the actions
                    if (element->timerIndex >= i) {
                        element->timerIndex--;
                    }

                    if (element->timers->number() == 0) {
                        if (currentTimer_ == element) {
                            currentTimerSalvaged_ = true;
                        }
                        else {
                            removeTimerElement(element);
                        }
                    }
                    return;
                }
            }
        }
    }

    bool Scheduler::isScheduled(const std::string& key, void *target) {
        TimerEntry *element = nullptr;
        if (timersMap_.find(target) != timersMap_.end())
            element = timersMap_[target];

        if (!element) {
            return false;
        }

        if (element->timers == nullptr) {
            return false;
        }
        else {
            for (int i = 0; i < element->timers->number(); ++i) {
                TimerTargetCallback *timer = static_cast<TimerTargetCallback*>((*element->timers)[i]);

                if (key == timer->getKey()) {
                    return true;
                }
            }
            return false;
        }

        return false;
    }

    void Scheduler::schedule(SelectorF selector, HObject *target, float interval, unsigned int repeat, float delay, bool paused) {
        TimerEntry *element = nullptr;
        if (timersMap_.find(target) != timersMap_.end())
            element = timersMap_[target];

        if (! element) {
            element = new TimerEntry;
            element->target = target;

            timersMap_.insert(std::unordered_map<void *,TimerEntry *>::value_type(target, element));

            // Is this the 1st element ? Then set the pause level to all the selectors of this target
            element->paused = paused;
        }

        if (element->timers == nullptr) {
            element->timers = new HObjectArray(10);
        }
        else {
            for (int i = 0; i < element->timers->number(); ++i) {
                TimerTargetSelector *timer = dynamic_cast<TimerTargetSelector*>((*element->timers)[i]);

                if (timer && selector == timer->getSelector()) {
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

    void Scheduler::schedule(SelectorF selector, HObject *target, float interval, bool paused) {
        this->schedule(selector, target, interval, -1, 0.0f, paused);
    }

    bool Scheduler::isScheduled(SelectorF selector, HObject *target) {
        TimerEntry *element = nullptr;
        if (timersMap_.find(target) != timersMap_.end())
            element = timersMap_[target];

        if (!element) {
            return false;
        }

        if (element->timers == nullptr) {
            return false;
        }
        else {
            for (int i = 0; i < element->timers->number(); ++i) {
                TimerTargetSelector *timer = static_cast<TimerTargetSelector*>((*element->timers)[i]);
                if (selector == timer->getSelector()) {
                    return true;
                }
            }
            return false;
        }
        return false;  // should never get here
    }

    void Scheduler::unschedule(SelectorF selector, HObject *target) {
        // explicity handle nil arguments when removing an object
        if (target == nullptr || selector == nullptr) {
            return;
        }

        TimerEntry *element = nullptr;
        if (timersMap_.find(target) != timersMap_.end())
            element = timersMap_[target];

        if (element) {
            for (int i = 0; i < element->timers->number(); ++i) {
                TimerTargetSelector *timer = static_cast<TimerTargetSelector*>((*element->timers)[i]);

                if (selector == timer->getSelector()) {
                    if (timer == element->currentTimer && (! element->currentTimerSalvaged)) {
                        element->currentTimer->retain();
                        element->currentTimerSalvaged = true;
                    }

                    element->timers->removeObjectAtIndex(i);

                    // update timerIndex in case we are in tick:, looping over the actions
                    if (element->timerIndex >= i) {
                        element->timerIndex--;
                    }

                    if (element->timers->number() == 0) {
                        if (currentTimer_ == element) {
                            currentTimerSalvaged_ = true;
                        }
                        else {
                            removeTimerElement(element);
                        }
                    }
                    return;
                }
            }
        }
    }

    void Scheduler::unscheduleUpdate(void *target) {
        if (target == nullptr) {
            return;
        }

        UpdateEntry *element = nullptr;
        if (updatesMap_.find(target) != updatesMap_.end())
            element = updatesMap_[target];

        if (element) {
            if (updateMapLocked_) {
                element->entry->markedForDeletion = true;
            }
            else {
                this->removeUpdateElement(element->entry);
            }
        }
    }

    void Scheduler::unscheduleAll(void) {
        unscheduleAllWithMinPriority(PRIORITY_SYSTEM);
    }

    void Scheduler::unscheduleAllWithMinPriority(int minPriority) {
        // Custom Selectors
        for (auto element = timersMap_.begin(); element != timersMap_.end(); ++element) {
            unscheduleAllForTarget(element->first);
        }

        // Updates selectors
        if(minPriority < 0) {
            for (auto entry : updatesNegList_) {
                if(entry->priority >= minPriority) {
                    unscheduleUpdate(entry->target);
                }
            }
        }

        if(minPriority <= 0) {
            for (auto entry : updates0List_) {
                unscheduleUpdate(entry->target);
            }
        }

        for (auto entry : updatesPosList_) {
            if(entry->priority >= minPriority) {
                unscheduleUpdate(entry->target);
            }
        }
    }

    void Scheduler::unscheduleAllForTarget(void *target) {
        // explicit nullptr handling
        if (target == nullptr) {
            return;
        }

        // Custom Selectors
        TimerEntry *element = nullptr;
        if (timersMap_.find(target) != timersMap_.end())
            element = timersMap_[target];

        if (element) {
            if (element->timers->containsObject(element->currentTimer)
                && (! element->currentTimerSalvaged)) {
                element->currentTimer->retain();
                element->currentTimerSalvaged = true;
            }
            element->timers->removeAllObjects();

            if (currentTimer_ == element) {
                currentTimerSalvaged_ = true;
            }
            else {
                removeTimerElement(element);
            }
        }

        // update selector
        unscheduleUpdate(target);
    }

    void Scheduler::resumeTarget(void *target) {
        // custom selectors
        TimerEntry *element = nullptr;
        if (timersMap_.find(target) != timersMap_.end())
            element = timersMap_[target];

        if (element) {
            element->paused = false;
        }

        // update selector
        UpdateEntry *elementUpdate = nullptr;
        if (updatesMap_.find(target) != updatesMap_.end())
            elementUpdate = updatesMap_[target];
        if (elementUpdate) {
            elementUpdate->entry->paused = false;
        }
    }

    void Scheduler::pauseTarget(void *target) {
        // custom selectors
        TimerEntry *element = nullptr;
        if (timersMap_.find(target) != timersMap_.end())
            element = timersMap_[target];

        if (element) {
            element->paused = true;
        }

        // update selector
        UpdateEntry *elementUpdate = nullptr;
        if (updatesMap_.find(target) != updatesMap_.end())
            elementUpdate = updatesMap_[target];
        if (elementUpdate) {
            elementUpdate->entry->paused = true;
        }
    }

    bool Scheduler::isTargetPaused(void *target) {
        // Custom selectors
        TimerEntry *element = nullptr;
        if (timersMap_.find(target) != timersMap_.end())
            element = timersMap_[target];
        if( element ) {
            return element->paused;
        }

        // We should check update selectors if target does not have custom selectors
        UpdateEntry *elementUpdate = nullptr;
        if (updatesMap_.find(target) != updatesMap_.end())
            elementUpdate = updatesMap_[target];
        if ( elementUpdate ) {
            return elementUpdate->entry->paused;
        }

        return false;  // should never get here
    }

    // main loop
    void Scheduler::update(float dt) {
        updateMapLocked_ = true;

        if (timeScale_ != 1.0f) {
            dt *= timeScale_;
        }

        //
        // Selector callbacks
        //

        // updates with priority < 0
        for (auto entry : updatesNegList_) {
            if ((! entry->paused) && (! entry->markedForDeletion)) {
                entry->callback(dt);
            }
        }

        // updates with priority == 0
        for (auto entry : updates0List_) {
            if ((! entry->paused) && (! entry->markedForDeletion)) {
                entry->callback(dt);
            }
        }

        // updates with priority > 0
        for (auto entry : updatesPosList_) {
            if ((! entry->paused) && (! entry->markedForDeletion)) {
                entry->callback(dt);
            }
        }

        // Iterate over all the custom selectors
        for (auto element = timersMap_.begin(); element != timersMap_.end(); ++element) {
            currentTimer_ = element->second;
            currentTimerSalvaged_ = false;

            if (! currentTimer_->paused) {
                // The 'timers' array may change while inside this loop
                for (element->second->timerIndex = 0; element->second->timerIndex < element->second->timers->number(); ++(element->second->timerIndex)) {
                    element->second->currentTimer = (Timer*)((*element->second->timers)[element->second->timerIndex]);
                    element->second->currentTimerSalvaged = false;

                    element->second->currentTimer->update(dt);

                    if (element->second->currentTimerSalvaged) {
                        // The currentTimer told the remove itself. To prevent the timer from
                        // accidentally deallocating itself before finishing its step, we retained
                        // it. Now that step is done, it's safe to release it.
                        element->second->currentTimer->release();
                    }

                    element->second->currentTimer = nullptr;
                }
            }

            if (currentTimerSalvaged_ && currentTimer_->timers->number() == 0) {
                removeTimerElement(currentTimer_);
            }
        }

        // delete all updates that are marked for deletion
        // updates with priority < 0
        for (auto entry : updatesNegList_) {
            if (entry->markedForDeletion) {
                this->removeUpdateElement(entry);
            }
        }

        // updates with priority == 0
        for (auto entry : updates0List_) {
            if (entry->markedForDeletion) {
                this->removeUpdateElement(entry);
            }
        }

        // updates with priority > 0
        for (auto entry : updatesPosList_) {
            if (entry->markedForDeletion) {
                this->removeUpdateElement(entry);
            }
        }

        updateMapLocked_ = false;
        currentTimer_ = nullptr;
    }

    std::set<void*> Scheduler::pauseAllTargets() {
        return pauseAllTargetsWithMinPriority(PRIORITY_SYSTEM);
    }

    std::set<void*> Scheduler::pauseAllTargetsWithMinPriority(int minPriority) {
        std::set<void*> idsWithSelectors;

        // Custom Selectors
        for(auto element = timersMap_.begin(); element != timersMap_.end(); ++element) {
            element->second->paused = true;
            idsWithSelectors.insert(element->first);
        }

        // Updates selectors
        if(minPriority < 0) {
            for (auto entry : updatesNegList_) {
                if (entry->priority >= minPriority) {
                    entry->paused = true;
                    idsWithSelectors.insert(entry->target);
                }
            }
        }

        if(minPriority <= 0) {
            for (auto entry : updates0List_) {
                entry->paused = true;
                idsWithSelectors.insert(entry->target);
            }
        }

        for (auto entry : updatesPosList_) {
            if (entry->priority >= minPriority) {
                entry->paused = true;
                idsWithSelectors.insert(entry->target);
            }
        }

        return idsWithSelectors;
    }

    void Scheduler::resumeTargets(const std::set<void*>& targetsToResume) {
        for(const auto &obj : targetsToResume) {
            this->resumeTarget(obj);
        }
    }

    void Scheduler::schedulePerFrame(const SchedulerFunc& callback, void *target, int priority, bool paused) {
        UpdateEntry *hashElement = nullptr;
        if (updatesMap_.find(target) != updatesMap_.end())
            hashElement = updatesMap_[target];
        if (hashElement) {
            // check if priority has changed
            if ((*hashElement->list->begin())->priority != priority) {
                if (updateMapLocked_) {
                    hashElement->entry->markedForDeletion = false;
                    hashElement->entry->paused = paused;
                    return;
                }
                else {
                    // will be added again outside if (hashElement).
                    unscheduleUpdate(target);
                }
            }
            else {
                hashElement->entry->markedForDeletion = false;
                hashElement->entry->paused = paused;
                return;
            }
        }

        // most of the updates are going to be 0, that's way there
        // is an special list for updates with priority 0
        if (priority == 0) {
            appendIn(updates0List_, callback, target, paused);
        }
        else if (priority < 0) {
            priorityIn(updatesNegList_, callback, target, priority, paused);
        }
        else {
            // priority > 0
            priorityIn(updatesPosList_, callback, target, priority, paused);
        }
    }

    void Scheduler::removeTimerElement(TimerEntry *element) {
        SAFE_DELETE(element->timers);
        timersMap_.erase(element->target);
        SAFE_DELETE(element);
    }

    void Scheduler::removeUpdateElement(ListEntry *entry) {
        UpdateEntry *element = nullptr;
        if (updatesMap_.find(entry->target) != updatesMap_.end())
            element = updatesMap_[entry->target];
        if (element) {
            for (auto iter = element->list->begin(); iter != element->list->end(); ++iter) {
                if ((*iter) == element->entry) {
                    element->list->erase(iter);
                }
            }
            SAFE_DELETE(element->entry);
            updatesMap_.erase(element);
            SAFE_DELETE(element);
        }
    }

    void Scheduler::priorityIn(std::list<ListEntry *> &list, const SchedulerFunc& callback, void *target, int priority, bool paused) {
        ListEntry *listElement = new ListEntry();

        listElement->callback = callback;
        listElement->target = target;
        listElement->priority = priority;
        listElement->paused = paused;
        listElement->markedForDeletion = false;

        // empty list ?
        if (list.empty()) {
            list.push_back(listElement);
        }
        else {
            bool added = false;

            for (auto iter = list.begin(); iter != list.end(); ++iter) {
                if (priority < (*iter)->priority) {
                    list.insert(iter, listElement);
                    added = true;
                    break;
                }
            }

            if (! added) {
                list.push_back(listElement);
            }
        }

        UpdateEntry *hashElement = new UpdateEntry;
        hashElement->target = target;
        hashElement->list = &list;
        hashElement->entry = listElement;
        updatesMap_.insert(std::unordered_map<void *, UpdateEntry *>::value_type(target, hashElement));
    }

    void Scheduler::appendIn(std::list<ListEntry *> &list, const SchedulerFunc& callback, void *target, bool paused) {
        ListEntry *listElement = new ListEntry();

        listElement->callback = callback;
        listElement->target = target;
        listElement->paused = paused;
        listElement->priority = 0;
        listElement->markedForDeletion = false;

        list.push_back(listElement);

        UpdateEntry *hashElement = new UpdateEntry;
        hashElement->target = target;
        hashElement->list = &list;
        hashElement->entry = listElement;
        updatesMap_.insert(std::unordered_map<void *, UpdateEntry *>::value_type(target, hashElement));
    }
}
