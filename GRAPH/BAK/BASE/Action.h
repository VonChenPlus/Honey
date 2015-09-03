#ifndef ACTIONS_H
#define ACTIONS_H

#include "BASE/HObject.h"
#include "MATH/Rectangle.h"
#include "GRAPH/BASE/Node.h"

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
        : _duration(0)
        {}
        virtual ~FiniteTimeAction(){}

    protected:
        //! Duration in seconds.
        float _duration;

    private:
        DISALLOW_COPY_AND_ASSIGN(FiniteTimeAction)
    };

    class Follow : public Action
    {
    public:
        static Follow* create(Node *followedNode, const MATH::Rectf& rect = MATH::RectfZERO);

        inline bool isBoundarySet() const { return _boundarySet; }
        inline void setBoundarySet(bool value) { _boundarySet = value; }

        virtual void step(float dt) override;
        virtual bool isDone() const override;
        virtual void stop() override;

    protected:
        Follow()
        : _followedNode(nullptr)
        , _boundarySet(false)
        , _boundaryFullyCovered(false)
        , _leftBoundary(0.0)
        , _rightBoundary(0.0)
        , _topBoundary(0.0)
        , _bottomBoundary(0.0)
        , _worldRect(MATH::RectfZERO)
        {}

        virtual ~Follow();

        bool initWithTarget(Node *followedNode, const MATH::Rectf& rect = MATH::RectfZERO);

    protected:
        Node *_followedNode;
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
}

#endif // ACTIONS_H
