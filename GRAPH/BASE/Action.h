#ifndef ACTIONS_H
#define ACTIONS_H

#include "BASE/HObject.h"
#include "MATH/Rectangle.h"
#include "GRAPH/BASE/Node.h"

namespace GRAPH
{
    class Node;
    /**
     * @addtogroup actions
     * @{
     */

    /**
     * @brief Base class for Action objects.
     */
    class Action : public HObject
    {
    public:
        /** Default tag used for all the actions. */
        static const int INVALID_TAG = -1;
        /**
         * @js NA
         * @lua NA
         */
        virtual std::string description() const;

        /** Returns a new action that performs the exactly the reverse action.
         *
         * @return A new action that performs the exactly the reverse action.
         * @js NA
         */
        virtual Action* reverse() const
        {
            return nullptr;
        }

        /** Return true if the action has finished.
         *
         * @return Is true if the action has finished.
         */
        virtual bool isDone() const;

        /** Called before the action start. It will also set the target.
         *
         * @param target A certain target.
         */
        virtual void startWithTarget(Node *target);

        /**
         * Called after the action has finished. It will set the 'target' to nil.
         * IMPORTANT: You should never call "Action::stop()" manually. Instead, use: "target->stopAction(action);".
         */
        virtual void stop();

        /** Called every frame with it's delta time, dt in seconds. DON'T override unless you know what you are doing.
         *
         * @param dt In seconds.
         */
        virtual void step(float dt);

        /**
         * Called once per frame. time a value between 0 and 1.

         * For example:
         * - 0 Means that the action just started.
         * - 0.5 Means that the action is in the middle.
         * - 1 Means that the action is over.
         *
         * @param time A value between 0 and 1.
         */
        virtual void update(float time);
        /** Return certain target.
         *
         * @return A certain target.
         */
        inline Node* getTarget() const { return _target; }
        /** The action will modify the target properties.
         *
         * @param target A certain target.
         */
        inline void setTarget(Node *target) { _target = target; }
        /** Return a original Target.
         *
         * @return A original Target.
         */
        inline Node* getOriginalTarget() const { return _originalTarget; }
        /**
         * Set the original target, since target can be nil.
         * Is the target that were used to run the action. Unless you are doing something complex, like ActionManager, you should NOT call this method.
         * The target is 'assigned', it is not 'retained'.
         * @since v0.8.2
         *
         * @param originalTarget Is 'assigned', it is not 'retained'.
         */
        inline void setOriginalTarget(Node *originalTarget) { _originalTarget = originalTarget; }
        /** Returns a tag that is used to identify the action easily.
         *
         * @return A tag.
         */
        inline int getTag() const { return _tag; }
        /** Changes the tag that is used to identify the action easily.
         *
         * @param tag Used to identify the action easily.
         */
        inline void setTag(int tag) { _tag = tag; }

    public:
        Action();
        virtual ~Action();

    protected:
        Node    *_originalTarget;
        /**
         * The "target".
         * The target will be set with the 'startWithTarget' method.
         * When the 'stop' method is called, target will be set to nil.
         * The target is 'assigned', it is not 'retained'.
         */
        Node    *_target;
        /** The action tag. An identifier of the action. */
        int     _tag;

    private:
        DISALLOW_COPY_AND_ASSIGN(Action)
    };

    /** @class FiniteTimeAction
     * @brief
     * Base class actions that do have a finite time duration.
     * Possible actions:
     * - An action with a duration of 0 seconds.
     * - An action with a duration of 35.5 seconds.
     * Infinite time actions are valid.
     */
    class FiniteTimeAction : public Action
    {
    public:
        /** Get duration in seconds of the action.
         *
         * @return The duration in seconds of the action.
         */
        inline float getDuration() const { return _duration; }
        /** Set duration in seconds of the action.
         *
         * @param duration In seconds of the action.
         */
        inline void setDuration(float duration) { _duration = duration; }

        //
        // Overrides
        //
        virtual FiniteTimeAction* reverse() const override
        {
            return nullptr;
        }

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

    /** @class Follow
     * @brief Follow is an action that "follows" a node.
     * Eg:
     * @code
     * layer->runAction(Follow::actionWithTarget(hero));
     * @endcode
     * Instead of using Camera as a "follower", use this action instead.
     * @since v0.99.2
     */
    class Follow : public Action
    {
    public:
        /**
         * Creates the action with a set boundary or with no boundary.
         *
         * @param followedNode  The node to be followed.
         * @param rect  The boundary. If \p rect is equal to Rect::ZERO, it'll work
         *              with no boundary.
         */
        static Follow* create(Node *followedNode, const MATH::Rectf& rect = MATH::RectfZERO);
        /** Return boundarySet.
         *
         * @return Return boundarySet.
         */
        inline bool isBoundarySet() const { return _boundarySet; }
        /** Alter behavior - turn on/off boundary.
         *
         * @param value Turn on/off boundary.
         */
        inline void setBoundarySet(bool value) { _boundarySet = value; }

        /**
         * @param dt in seconds.
         * @js NA
         */
        virtual void step(float dt) override;
        virtual bool isDone() const override;
        virtual void stop() override;

    public:
        /**
         * @js ctor
         */
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
        /**
         * @js NA
         * @lua NA
         */
        virtual ~Follow();

        /**
         * Initializes the action with a set boundary or with no boundary.
         *
         * @param followedNode  The node to be followed.
         * @param rect  The boundary. If \p rect is equal to Rect::ZERO, it'll work
         *              with no boundary.
         */
        bool initWithTarget(Node *followedNode, const MATH::Rectf& rect = MATH::RectfZERO);

    protected:
        /** Node to follow. */
        Node *_followedNode;

        /** Whether camera should be limited to certain area. */
        bool _boundarySet;

        /** If screen size is bigger than the boundary - update not needed. */
        bool _boundaryFullyCovered;

        /** Fast access to the screen dimensions. */
        MATH::Vector2f _halfScreenSize;
        MATH::Vector2f _fullScreenSize;

        /** World boundaries. */
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
