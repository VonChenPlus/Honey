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
        Scheduler* scheduler_;
        float elapsed_;
        bool runForever_;
        bool useDelay_;
        unsigned int timesExecuted_;
        unsigned int repeat_; //0 = once, 1 is 2 x executed
        float delay_;
        float _interval;
    };

    class TimerTargetSelector : public Timer
    {
    public:
        TimerTargetSelector();

        bool initWithSelector(Scheduler* scheduler, SelectorF selector, HObject* target, float seconds, unsigned int repeat, float delay);

        inline SelectorF getSelector() const { return selector_; }

        virtual void trigger() override;
        virtual void cancel() override;

    protected:
        HObject* target_;
        SelectorF selector_;
    };

    class TimerTargetCallback : public Timer
    {
    public:
        TimerTargetCallback();

        bool initWithCallback(Scheduler* scheduler, const SchedulerFunc& callback, void *target, const std::string& key, float seconds, unsigned int repeat, float delay);

        inline const SchedulerFunc& getCallback() const { return callback_; }
        inline const std::string& getKey() const { return key_; }

        virtual void trigger() override;
        virtual void cancel() override;

    protected:
        void* target_;
        SchedulerFunc callback_;
        std::string key_;
    };

    struct ListEntry
    {
        SchedulerFunc       callback;
        void                *target;
        int                 priority;
        bool                paused;
        bool                markedForDeletion;
    };

    struct UpdateEntry
    {
        std::list<ListEntry *> *list;
        ListEntry *entry;
        void                *target;
        SchedulerFunc     callback;
    };

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

        inline float getTimeScale() { return timeScale_; }
        inline void setTimeScale(float timeScale) { timeScale_ = timeScale; }

        void schedule(const SchedulerFunc& callback, void *target, float interval, unsigned int repeat, float delay, bool paused, const std::string& key);
        void schedule(const SchedulerFunc& callback, void *target, float interval, bool paused, const std::string& key);
        void schedule(SelectorF selector, HObject *target, float interval, unsigned int repeat, float delay, bool paused);
        void schedule(SelectorF selector, HObject *target, float interval, bool paused);

        template <class T>
        void scheduleUpdate(T *target, int priority, bool paused)
        {
            this->schedulePerFrame([target](float dt){
                target->update(dt);
            }, target, priority, paused);
        }

        void unschedule(const std::string& key, void *target);
        void unschedule(SelectorF selector, HObject *target);
        void unscheduleUpdate(void *target);
        void unscheduleAllForTarget(void *target);
        void unscheduleAll();
        void unscheduleAllWithMinPriority(int minPriority);

        bool isScheduled(const std::string& key, void *target);
        bool isScheduled(SelectorF selector, HObject *target);

        void update(float dt);

        void pauseTarget(void *target);
        void resumeTarget(void *target);
        bool isTargetPaused(void *target);

        std::set<void*> pauseAllTargets();
        std::set<void*> pauseAllTargetsWithMinPriority(int minPriority);

        void resumeTargets(const std::set<void*>& targetsToResume);

    protected:
        void schedulePerFrame(const SchedulerFunc& callback, void *target, int priority, bool paused);

        void removeTimerElement(TimerEntry *element);
        void removeUpdateElement(ListEntry *entry);

        void priorityIn(std::list<ListEntry *> &list, const SchedulerFunc& callback, void *target, int priority, bool paused);
        void appendIn(std::list<ListEntry *> &list, const SchedulerFunc& callback, void *target, bool paused);

    private:
        float timeScale_;
        std::list<ListEntry *> updatesNegList_;
        std::list<ListEntry *> updates0List_;
        std::list<ListEntry *> updatesPosList_;
        std::unordered_map<void *, UpdateEntry *> updatesMap_;
        std::unordered_map<void *, TimerEntry *> timersMap_;
        TimerEntry * currentTimer_;
        bool currentTimerSalvaged_;
        bool updateMapLocked_;
    };
}

#endif // SCHEDULER_H
