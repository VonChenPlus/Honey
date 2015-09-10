#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <functional>
#include <mutex>
#include <set>
#include <list>
#include <unordered_map>
#include "BASE/HObject.h"

namespace GRAPH
{
    class Scheduler;
    typedef std::function<void(float)> SchedulerFunc;

    class Timer : public HObject
    {
    protected:
        Timer();
    public:
        inline float getInterval() const { return _interval; }
        inline void setInterval(float interval) { _interval = interval; }

        void setupTimerWithInterval(float seconds, unsigned int repeat, float delay);

        virtual void trigger() = 0;
        virtual void cancel() = 0;

        void update(float dt);

    protected:
        Scheduler* _scheduler;
        float _elapsed;
        bool _runForever;
        bool _useDelay;
        unsigned int _timesExecuted;
        unsigned int _repeat; //0 = once, 1 is 2 x executed
        float _delay;
        float _interval;
    };

    class TimerTargetSelector : public Timer
    {
    public:
        TimerTargetSelector();

        bool initWithSelector(Scheduler* scheduler, CallFuncF selector, HObject* target, float seconds, unsigned int repeat, float delay);

        inline CallFuncF getSelector() const { return _selector; }

        virtual void trigger() override;
        virtual void cancel() override;

    protected:
        HObject* _target;
        CallFuncF _selector;
    };

    class TimerTargetCallback : public Timer
    {
    public:
        TimerTargetCallback();

        bool initWithCallback(Scheduler* scheduler, const SchedulerFunc& callback, void *target, const std::string& key, float seconds, unsigned int repeat, float delay);

        inline const SchedulerFunc& getCallback() const { return _callback; }
        inline const std::string& getKey() const { return _key; }

        virtual void trigger() override;
        virtual void cancel() override;

    protected:
        void* _target;
        SchedulerFunc _callback;
        std::string _key;
    };

    struct ListEntry
    {
        SchedulerFunc       callback;
        void                *target;
        int                 priority;
        bool                paused;
        bool                markedForDeletion; // selector will no longer be called and entry will be removed at end of the next tick
    };

    struct UpdateEntry
    {
        std::list<ListEntry *> *list;        // Which list does it belong to ?
        ListEntry *entry;        // entry in the list
        void                *target;
        SchedulerFunc     callback;
    };

    // Hash Element used for "selectors with interval"
    struct TimerEntry
    {
        HObjectArray        *timers;
        void                *target;
        int                 timerIndex;
        Timer               *currentTimer;
        bool                currentTimerSalvaged;
        bool                paused;
    };

    class Scheduler : public HObject
    {
    public:
        static const int PRIORITY_SYSTEM;
        static const int PRIORITY_NON_SYSTEM_MIN;

        Scheduler();
        virtual ~Scheduler();

        inline float getTimeScale() { return _timeScale; }
        inline void setTimeScale(float timeScale) { _timeScale = timeScale; }

        void update(float dt);

        void schedule(const SchedulerFunc& callback, void *target, float interval, unsigned int repeat, float delay, bool paused, const std::string& key);
        void schedule(const SchedulerFunc& callback, void *target, float interval, bool paused, const std::string& key);
        void schedule(CallFuncF selector, HObject *target, float interval, unsigned int repeat, float delay, bool paused);
        void schedule(CallFuncF selector, HObject *target, float interval, bool paused);

        template <class T>
        void scheduleUpdate(T *target, int priority, bool paused)
        {
            this->schedulePerFrame([target](float dt){
                target->update(dt);
            }, target, priority, paused);
        }

        void unschedule(const std::string& key, void *target);
        void unschedule(CallFuncF selector, HObject *target);
        void unscheduleUpdate(void *target);
        void unscheduleAllForTarget(void *target);
        void unscheduleAll();
        void unscheduleAllWithMinPriority(int minPriority);

        bool isScheduled(const std::string& key, void *target);
        bool isScheduled(CallFuncF selector, HObject *target);

        void pauseTarget(void *target);
        void resumeTarget(void *target);
        bool isTargetPaused(void *target);

        std::set<void*> pauseAllTargets();
        std::set<void*> pauseAllTargetsWithMinPriority(int minPriority);

        void resumeTargets(const std::set<void*>& targetsToResume);
        void performFunctionInCocosThread( const std::function<void()> &function);

    protected:
        void schedulePerFrame(const SchedulerFunc& callback, void *target, int priority, bool paused);

        void removeHashElement(TimerEntry *element);
        void removeUpdateFromHash(ListEntry *entry);

        void priorityIn(std::list<ListEntry *> *list, const SchedulerFunc& callback, void *target, int priority, bool paused);
        void appendIn(std::list<ListEntry *> *list, const SchedulerFunc& callback, void *target, bool paused);

        float _timeScale;
        std::list<ListEntry *> _updatesNegList;        // list of priority < 0
        std::list<ListEntry *> _updates0List;            // list priority == 0
        std::list<ListEntry *> _updatesPosList;        // list priority > 0
        std::unordered_map<void *, UpdateEntry *> _hashForUpdates;
        std::unordered_map<void *, TimerEntry *> _hashForTimers;
        TimerEntry * _currentTarget;
        bool _currentTargetSalvaged;
        bool _updateHashLocked;
        std::vector<std::function<void()>> _functionsToPerform;
        std::mutex _performMutex;
    };
}

#endif // SCHEDULER_H
