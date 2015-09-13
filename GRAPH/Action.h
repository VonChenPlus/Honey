#ifndef ACTIONS_H
#define ACTIONS_H

#include <unordered_map>
#include "BASE/HObject.h"
#include "MATH/Rectangle.h"

namespace GRAPH
{
    class Action : public HObject
    {
    public:
        static const int INVALID_TAG = -1;

        virtual bool isDone() const;

        virtual void startWithTarget(HObject *target);
        virtual void stop();
        virtual void step(float dt);
        virtual void update(float time);

        inline HObject* getTarget() const { return _target; }
        inline void setTarget(HObject *target) { _target = target; }

        inline HObject* getOriginalTarget() const { return _originalTarget; }
        inline void setOriginalTarget(HObject *originalTarget) { _originalTarget = originalTarget; }

        inline int getTag() const { return _tag; }
        inline void setTag(int tag) { _tag = tag; }

    public:
        Action();
        virtual ~Action();

    protected:
        HObject    *_originalTarget;
        HObject    *_target;
        int     _tag;

    private:
        DISALLOW_COPY_AND_ASSIGN(Action)
    };

    class FiniteTimeAction : public Action
    {
    public:
        inline float getDuration() const { return _duration; }
        inline void setDuration(float duration) { _duration = duration; }

    public:
        FiniteTimeAction()
            : _duration(0) {

        }
        virtual ~FiniteTimeAction(){}

    protected:
        float _duration;

    private:
        DISALLOW_COPY_AND_ASSIGN(FiniteTimeAction)
    };

    class Follow : public Action
    {
    public:
        static Follow* create(HObject *followedHObject, const MATH::Rectf& rect = MATH::RectfZERO);

        inline bool isBoundarySet() const { return _boundarySet; }
        inline void setBoundarySet(bool value) { _boundarySet = value; }

        virtual void step(float dt) override;
        virtual bool isDone() const override;
        virtual void stop() override;

    protected:
        Follow();
        virtual ~Follow();

        bool initWithTarget(HObject *followedHObject, const MATH::Rectf& rect = MATH::RectfZERO);

    protected:
        HObject *_followedHObject;
        bool _boundarySet;
        bool _boundaryFullyCovered;

        MATH::Vector2f _halfScreenSize;
        MATH::Vector2f _fullScreenSize;

        float _leftBoundary;
        float _rightBoundary;
        float _topBoundary;
        float _bottomBoundary;
        MATH::Rectf _worldRect;

    private:
        DISALLOW_COPY_AND_ASSIGN(Follow)
    };

    struct ActionEntry
    {
        HObjectArray        *actions;
        HObject             *target;
        int                 actionIndex;
        Action              *currentAction;
        bool                currentActionSalvaged;
        bool                paused;
    };

    class ActionManager : public HObject
    {
    public:
        ActionManager(void);
        ~ActionManager(void);

        void addAction(Action *action, HObject *target, bool paused);

        void removeAllActions();
        void removeAllActionsFromTarget(HObject *target);
        void removeAction(Action *action);
        void removeActionByTag(int tag, HObject *target);
        void removeAllActionsByTag(int tag, HObject *target);

        Action* getActionByTag(int tag, const HObject *target) const;
        ssize_t getNumberOfRunningActionsInTarget(const HObject *target) const;

        void pauseTarget(HObject *target);
        void resumeTarget(HObject *target);

        std::vector<HObject*> pauseAllRunningActions();
        void resumeTargets(const std::vector<HObject*>& targetsToResume);

        void update(float dt);

    protected:
        void removeActionAtIndex(ssize_t index, ActionEntry *element);
        void deleteHashElement(ActionEntry *element);
        void actionAllocWithHashElement(ActionEntry *element);

    protected:
        std::unordered_map<HObject *, ActionEntry *> _targets;
        ActionEntry    *_currentTarget;
        bool            _currentTargetSalvaged;
    };
}

#endif // ACTIONS_H
