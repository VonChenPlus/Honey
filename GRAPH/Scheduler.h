#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <functional>
#include <mutex>
#include <set>
#include <list>
#include <unordered_map>
#include <string.h>
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
        inline float getInterval() const { return interval_; }
        inline void setInterval(float interval) { interval_ = interval; }

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
        float interval_;
    };

    struct ListEntry
    {
        ListEntry() {
            memset(this, 0, sizeof(ListEntry));
        }

        SchedulerFunc       callback;
        void                *target;
        int                 priority;
        bool                paused;
        bool                markedForDeletion;
    };

    struct UpdateEntry
    {
        UpdateEntry() {
            memset(this, 0, sizeof(UpdateEntry));
        }
        std::list<ListEntry *> *list;
        ListEntry *entry;
        void                *target;
    };

    struct TimerEntry
    {
        TimerEntry() {
            memset(this, 0, sizeof(TimerEntry));
        }
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

        void schedule(const SchedulerFunc& callback, void *target, const std::string& key, bool paused, float interval, unsigned int repeat = -1, float delay = 0.0f);
        void schedule(SelectorF selector, HObject *target, bool paused, float interval, unsigned int repeat = -1, float delay = 0.0f);

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
