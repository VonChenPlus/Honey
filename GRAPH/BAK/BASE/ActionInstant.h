#ifndef ACTIONINSTANT_H
#define ACTIONINSTANT_H

#include <functional>
#include "GRAPH/BASE/Action.h"

namespace GRAPH
{
    class ActionInstant : public FiniteTimeAction
    {
    public:
        virtual bool isDone() const override;
        /**
         * @param dt In seconds.
         */
        virtual void step(float dt) override;
        /**
         * @param time In seconds.
         */
        virtual void update(float time) override;
    };

    class Show : public ActionInstant
    {
    public:
        /** Allocates and initializes the action.
         *
         * @return  An autoreleased Show object.
         */
        static Show * create();

        //
        // Overrides
        //
        /**
         * @param time In seconds.
         */
        virtual void update(float time) override;

    public:
        Show(){}
        virtual ~Show(){}

    private:
        DISALLOW_COPY_AND_ASSIGN(Show)
    };

    /** @class Hide
    * @brief Hide the node.
    */
    class Hide : public ActionInstant
    {
    public:
        /** Allocates and initializes the action.
         *
         * @return An autoreleased Hide object.
         */
        static Hide * create();

        //
        // Overrides
        //
        /**
         * @param time In seconds.
         */
        virtual void update(float time) override;

    public:
        Hide(){}
        virtual ~Hide(){}

    private:
        DISALLOW_COPY_AND_ASSIGN(Hide)
    };

    /** @class ToggleVisibility
    * @brief Toggles the visibility of a node.
    */
    class ToggleVisibility : public ActionInstant
    {
    public:
        /** Allocates and initializes the action.
         *
         * @return An autoreleased ToggleVisibility object.
         */
        static ToggleVisibility * create();

        //
        // Overrides
        //
        /**
         * @param time In seconds.
         */
        virtual void update(float time) override;

    public:
        ToggleVisibility(){}
        virtual ~ToggleVisibility(){}

    private:
        DISALLOW_COPY_AND_ASSIGN(ToggleVisibility)
    };

    /** @class RemoveSelf
    * @brief Remove the node.
    */
    class RemoveSelf : public ActionInstant
    {
    public:
        /** Create the action.
         *
         * @param isNeedCleanUp Is need to clean up, the default value is true.
         * @return An autoreleased RemoveSelf object.
         */
        static RemoveSelf * create(bool isNeedCleanUp = true);

        //
        // Override
        //
        /**
         * @param time In seconds.
         */
        virtual void update(float time) override;

    public:
        RemoveSelf() : _isNeedCleanUp(true){}
        virtual ~RemoveSelf(){}

        /** init the action */
        bool init(bool isNeedCleanUp);

    protected:
        bool _isNeedCleanUp;

    private:
        DISALLOW_COPY_AND_ASSIGN(RemoveSelf)
    };

    /** @class FlipX
    * @brief Flips the sprite horizontally.
    * @since v0.99.0
    */
    class FlipX : public ActionInstant
    {
    public:
        /** Create the action.
         *
         * @param x Flips the sprite horizontally if true.
         * @return  An autoreleased FlipX object.
         */
        static FlipX * create(bool x);

        //
        // Overrides
        //
        /**
         * @param time In seconds.
         */
        virtual void update(float time) override;

    public:
        FlipX() :_flipX(false) {}
        virtual ~FlipX() {}

        /** init the action */
        bool initWithFlipX(bool x);

    protected:
        bool    _flipX;

    private:
        DISALLOW_COPY_AND_ASSIGN(FlipX)
    };

    /** @class FlipY
    * @brief Flips the sprite vertically.
    * @since v0.99.0
    */
    class FlipY : public ActionInstant
    {
    public:
        /** Create the action.
         *
         * @param y Flips the sprite vertically if true.
         * @return An autoreleased FlipY object.
         */
        static FlipY * create(bool y);

        //
        // Overrides
        //
        /**
         * @param time In seconds.
         */
        virtual void update(float time) override;

    public:
        FlipY() :_flipY(false) {}
        virtual ~FlipY() {}

        /** init the action */
        bool initWithFlipY(bool y);

    protected:
        bool    _flipY;

    private:
        DISALLOW_COPY_AND_ASSIGN(FlipY)
    };

    /** @class Place
    * @brief Places the node in a certain position.
    */
    class Place : public ActionInstant //<NSCopying>
    {
    public:

        /** Creates a Place action with a position.
         *
         * @param pos  A certain position.
         * @return  An autoreleased Place object.
         */
        static Place * create(const MATH::Vector2f& pos);

        //
        // Overrides
        //
        /**
         * @param time In seconds.
         */
        virtual void update(float time) override;

    public:
        Place(){}
        virtual ~Place(){}

        /** Initializes a Place action with a position */
        bool initWithPosition(const MATH::Vector2f& pos);

    protected:
        MATH::Vector2f _position;

    private:
        DISALLOW_COPY_AND_ASSIGN(Place)
    };


    /** @class CallFunc
    * @brief Calls a 'callback'.
    */
    class CallFunc : public ActionInstant //<NSCopying>
    {
    public:
        /** Creates the action with the callback of type std::function<void()>.
         This is the pHObjecterred way to create the callback.
         * When this funtion bound in js or lua ,the input param will be changed.
         * In js: var create(var func, var this, var [data]) or var create(var func).
         * In lua:local create(local funcID).
         *
         * @param func  A callback function need to be excuted.
         * @return  An autoreleased CallFunc object.
         */
        static CallFunc * create(const std::function<void()>& func);

    public:
        /** Executes the callback.
         */
        virtual void execute();

        /** Get the selector target.
         *
         * @return The selector target.
         */
        inline HObject* getTargetCallback()
        {
            return _selectorTarget;
        }

        /** Set the selector target.
         *
         * @param sel The selector target.
         */
        inline void setTargetCallback(HObject* sel)
        {
            if (sel != _selectorTarget)
            {
                SAFE_RETAIN(sel);
                SAFE_RELEASE(_selectorTarget);
                _selectorTarget = sel;
            }
        }
        //
        // Overrides
        //
        /**
         * @param time In seconds.
         */
        virtual void update(float time) override;

    public:
        CallFunc()
        : _selectorTarget(nullptr)
        , _callFunc(nullptr)
        , _function(nullptr)
        {
        }
        virtual ~CallFunc();

        /** initializes the action with the std::function<void()>
         * @lua NA
         */
        bool initWithFunction(const std::function<void()>& func);

    protected:
        /** Target that will be called */
        HObject*   _selectorTarget;

        union
        {
            SEL_CallFunc    _callFunc;
            SEL_CallFuncN    _callFuncN;
        };

        /** function that will be called */
        std::function<void()> _function;

    private:
        DISALLOW_COPY_AND_ASSIGN(CallFunc)
    };

    /** @class CallFuncN
    * @brief Calls a 'callback' with the node as the first argument. N means Node.
    * @js NA
    */
    class CallFuncN : public CallFunc
    {
    public:
        /** Creates the action with the callback of type std::function<void()>.
         This is the pHObjecterred way to create the callback.
         *
         * @param func  A callback function need to be excuted.
         * @return  An autoreleased CallFuncN object.
         */
        static CallFuncN * create(const std::function<void(Node*)>& func);

        virtual void execute() override;

    public:
        CallFuncN():_functionN(nullptr){}
        virtual ~CallFuncN(){}

        /** initializes the action with the std::function<void(Node*)> */
        bool initWithFunction(const std::function<void(Node*)>& func);

    protected:
        /** function that will be called with the "sender" as the 1st argument */
        std::function<void(Node*)> _functionN;

    private:
        DISALLOW_COPY_AND_ASSIGN(CallFuncN)
    };

    /** @class __CCCallFuncND
     * @deprecated Please use CallFuncN instead.
     * @brief Calls a 'callback' with the node as the first argument and the 2nd argument is data.
     * ND means: Node and Data. Data is void *, so it could be anything.
     * @js NA
     */
    class CCCallFuncND : public CallFunc
    {
    public:
        virtual void execute() override;

    public:
        CCCallFuncND() {}
        virtual ~CCCallFuncND() {}

        /** initializes the action with the callback and the data to pass as an argument */
        bool initWithTarget(HObject* target, SEL_CallFuncND selector, void* d);

    protected:
        SEL_CallFuncND _callFuncND;
        void* _data;

    private:
        DISALLOW_COPY_AND_ASSIGN(CCCallFuncND)
    };


    /** @class __CCCallFuncO
     @deprecated Please use CallFuncN instead.
     @brief Calls a 'callback' with an object as the first argument. O means Object.
     @since v0.99.5
     @js NA
     */

    class CCCallFuncO : public CallFunc
    {
    public:
        //
        // Overrides
        //
        virtual void execute() override;

        HObject* getObject() const;
        void setObject(HObject* obj);

    public:
        CCCallFuncO();
        virtual ~CCCallFuncO();

    protected:
        /** object to be passed as argument */
        HObject* _object;
        SEL_CallFuncO _callFuncO;

    private:
        DISALLOW_COPY_AND_ASSIGN(CCCallFuncO)
    };
}

#endif //ACTIONINSTANT_H
