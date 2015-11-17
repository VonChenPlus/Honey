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

        inline Node* getTarget() const { return target_; }
        inline void setTarget(Node *target) { target_ = target; }

        inline Node* getOriginalTarget() const { return originalTarget_; }
        inline void setOriginalTarget(Node *originalTarget) { originalTarget_ = originalTarget; }

        inline int getTag() const { return tag_; }
        inline void setTag(int tag) { tag_ = tag; }

    public:
        Action();
        virtual ~Action();

    protected:
        Node    *originalTarget_;
        Node    *target_;
        int     tag_;

    private:
        DISALLOW_COPY_AND_ASSIGN(Action)
    };

    class FiniteTimeAction : public Action
    {
    public:
        inline float getDuration() const { return duration_; }
        inline void setDuration(float duration) { duration_ = duration; }

    public:
        FiniteTimeAction()
            : duration_(0) {

        }
        virtual ~FiniteTimeAction(){}

    protected:
        float duration_;

    private:
        DISALLOW_COPY_AND_ASSIGN(FiniteTimeAction)
    };

    class ActionInterval : public FiniteTimeAction
    {
    public:
        inline float getElapsed(void) { return elapsed_; }

        void setAmplitudeRate(float amp);
        float getAmplitudeRate(void);

        virtual bool isDone(void) const override;
        virtual void step(float dt) override;
        virtual void startWithTarget(Node *target) override;

    public:
        bool initWithDuration(float d);

    protected:
        float elapsed_;
        bool   firstTick_;
    };

    class  ScaleTo : public ActionInterval
    {
    public:
        static ScaleTo* create(float duration, float s);
        static ScaleTo* create(float duration, float sx, float sy);
        static ScaleTo* create(float duration, float sx, float sy, float sz);

        virtual void startWithTarget(Node *target) override;
        virtual void update(float time) override;

    public:
        ScaleTo() {}
        virtual ~ScaleTo() {}

        bool initWithDuration(float duration, float s);
        bool initWithDuration(float duration, float sx, float sy);
        bool initWithDuration(float duration, float sx, float sy, float sz);

    protected:
        float scaleX_;
        float scaleY_;
        float scaleZ_;
        float startScaleX_;
        float startScaleY_;
        float startScaleZ_;
        float endScaleX_;
        float endScaleY_;
        float endScaleZ_;
        float deltaX_;
        float deltaY_;
        float deltaZ_;

    private:
        DISALLOW_COPY_AND_ASSIGN(ScaleTo)
    };

    class Follow : public Action
    {
    public:
        static Follow* create(HObject *followedHObject, const MATH::Rectf& rect = MATH::RectfZERO);

        inline bool isBoundarySet() const { return boundarySet_; }
        inline void setBoundarySet(bool value) { boundarySet_ = value; }

        virtual void step(float dt) override;
        virtual bool isDone() const override;
        virtual void stop() override;

    protected:
        Follow();
        virtual ~Follow();

        bool initWithTarget(HObject *followedHObject, const MATH::Rectf& rect = MATH::RectfZERO);

    protected:
        HObject *followedObject_;
        bool boundarySet_;
        bool boundaryFullyCovered_;

        MATH::Vector2f halfScreenSize_;
        MATH::Vector2f fullScreenSize_;

        float leftBoundary_;
        float rightBoundary_;
        float topBoundary_;
        float bottomBoundary_;
        MATH::Rectf worldRect_;

    private:
        DISALLOW_COPY_AND_ASSIGN(Follow)
    };

    struct ActionEntry
    {
        HObjectArray *actions;
        Node *target;
        int64 actionIndex;
        Action *currentAction;
        bool currentActionSalvaged;
        bool paused;
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
        void removeActionAtIndex(int64 index, ActionEntry *element);
        void deleteHashElement(ActionEntry *element);
        void actionAllocWithHashElement(ActionEntry *element);

    protected:
        std::unordered_map<Node *, ActionEntry *> targets_;
        ActionEntry    *currentTarget_;
        bool            currentTargetSalvaged_;
    };
}

#endif // ACTIONS_H
