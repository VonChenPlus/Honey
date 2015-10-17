#ifndef ACTIONS_H
#define ACTIONS_H

#include <unordered_map>
#include "BASE/HObject.h"
#include "MATH/Rectangle.h"

namespace GRAPH
{
    class Node;

    class Action : public HObject
    {
    public:
        static const int INVALID_TAG = -1;

        virtual bool isDone() const;

        virtual void startWithTarget(Node *target);
        virtual void stop();
        virtual void step(float dt);
        virtual void update(float time);

        inline Node* getTarget() const { return _target; }
        inline void setTarget(Node *target) { _target = target; }

        inline Node* getOriginalTarget() const { return _originalTarget; }
        inline void setOriginalTarget(Node *originalTarget) { _originalTarget = originalTarget; }

        inline int getTag() const { return _tag; }
        inline void setTag(int tag) { _tag = tag; }

    public:
        Action();
        virtual ~Action();

    protected:
        Node    *_originalTarget;
        Node    *_target;
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

    class ActionInterval : public FiniteTimeAction
    {
    public:
        inline float getElapsed(void) { return _elapsed; }

        void setAmplitudeRate(float amp);
        float getAmplitudeRate(void);

        virtual bool isDone(void) const override;
        virtual void step(float dt) override;
        virtual void startWithTarget(Node *target) override;

    public:
        bool initWithDuration(float d);

    protected:
        float _elapsed;
        bool   _firstTick;
    };

    class  ScaleTo : public ActionInterval
    {
    public:
        /**
         * Creates the action with the same scale factor for X and Y.
         * @param duration Duration time, in seconds.
         * @param s Scale factor of x and y.
         * @return An autoreleased ScaleTo object.
         */
        static ScaleTo* create(float duration, float s);

        /**
         * Creates the action with and X factor and a Y factor.
         * @param duration Duration time, in seconds.
         * @param sx Scale factor of x.
         * @param sy Scale factor of y.
         * @return An autoreleased ScaleTo object.
         */
        static ScaleTo* create(float duration, float sx, float sy);

        /**
         * Creates the action with X Y Z factor.
         * @param duration Duration time, in seconds.
         * @param sx Scale factor of x.
         * @param sy Scale factor of y.
         * @param sz Scale factor of z.
         * @return An autoreleased ScaleTo object.
         */
        static ScaleTo* create(float duration, float sx, float sy, float sz);

        virtual void startWithTarget(Node *target) override;
        /**
         * @param time In seconds.
         */
        virtual void update(float time) override;

    public:
        ScaleTo() {}
        virtual ~ScaleTo() {}

        /**
         * initializes the action with the same scale factor for X and Y
         * @param duration in seconds
         */
        bool initWithDuration(float duration, float s);
        /**
         * initializes the action with and X factor and a Y factor
         * @param duration in seconds
         */
        bool initWithDuration(float duration, float sx, float sy);
        /**
         * initializes the action with X Y Z factor
         * @param duration in seconds
         */
        bool initWithDuration(float duration, float sx, float sy, float sz);

    protected:
        float _scaleX;
        float _scaleY;
        float _scaleZ;
        float _startScaleX;
        float _startScaleY;
        float _startScaleZ;
        float _endScaleX;
        float _endScaleY;
        float _endScaleZ;
        float _deltaX;
        float _deltaY;
        float _deltaZ;

    private:
        DISALLOW_COPY_AND_ASSIGN(ScaleTo)
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
        Node             *target;
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

        void addAction(Action *action, Node *target, bool paused);

        void removeAllActions();
        void removeAllActionsFromTarget(Node *target);
        void removeAction(Action *action);
        void removeActionByTag(int tag, Node *target);
        void removeAllActionsByTag(int tag, Node *target);

        Action* getActionByTag(int tag, const Node *target) const;
        uint64 getNumberOfRunningActionsInTarget(const Node *target) const;

        void pauseTarget(Node *target);
        void resumeTarget(Node *target);

        std::vector<Node*> pauseAllRunningActions();
        void resumeTargets(const std::vector<Node*>& targetsToResume);

        void update(float dt);

    protected:
        void removeActionAtIndex(uint64 index, ActionEntry *element);
        void deleteHashElement(ActionEntry *element);
        void actionAllocWithHashElement(ActionEntry *element);

    protected:
        std::unordered_map<Node *, ActionEntry *> _targets;
        ActionEntry    *_currentTarget;
        bool            _currentTargetSalvaged;
    };
}

#endif // ACTIONS_H
