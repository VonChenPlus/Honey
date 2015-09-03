#ifndef ACTIONINTERVAL_H
#define ACTIONINTERVAL_H

#include <vector>
#include "GRAPH/BASE/Action.h"

namespace GRAPH
{
    class Node;
    class SpriteFrame;
    class EventCustom;

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
        /** initializes the action */
        bool initWithDuration(float d);

    protected:
        float _elapsed;
        bool   _firstTick;
    };

    /** @class Sequence
     * @brief Runs actions sequentially, one after another.
     */
    class Sequence : public ActionInterval
    {
    public:
        static Sequence* create(FiniteTimeAction *action1, ...);
        static Sequence* create(const HObjectVector<FiniteTimeAction*>& arrayOfActions);
        static Sequence* createWithVariableList(FiniteTimeAction *action1, va_list args);
        static Sequence* createWithTwoActions(FiniteTimeAction *actionOne, FiniteTimeAction *actionTwo);

        virtual void startWithTarget(Node *target) override;
        virtual void stop(void) override;
        virtual void update(float t) override;

    public:
        Sequence() {}
        virtual ~Sequence(void);

        /** initializes the action */
        bool initWithTwoActions(FiniteTimeAction *pActionOne, FiniteTimeAction *pActionTwo);

    protected:
        FiniteTimeAction *_actions[2];
        float _split;
        int _last;

    private:
        DISALLOW_COPY_AND_ASSIGN(Sequence)
    };

    class Repeat : public ActionInterval
    {
    public:
        static Repeat* create(FiniteTimeAction *action, unsigned int times);

        /** Sets the inner action.
         *
         * @param action The inner action.
         */
        inline void setInnerAction(FiniteTimeAction *action)
        {
            if (_innerAction != action)
            {
                SAFE_RETAIN(action);
                SAFE_RELEASE(_innerAction);
                _innerAction = action;
            }
        }

        inline FiniteTimeAction* getInnerAction()
        {
            return _innerAction;
        }

        virtual void startWithTarget(Node *target) override;
        virtual void stop(void) override;
        /**
         * @param dt In seconds.
         */
        virtual void update(float dt) override;
        virtual bool isDone(void) const override;

    public:
        Repeat() {}
        virtual ~Repeat();

        /** initializes a Repeat action. Times is an unsigned integer between 1 and pow(2,30) */
        bool initWithAction(FiniteTimeAction *pAction, unsigned int times);

    protected:
        unsigned int _times;
        unsigned int _total;
        float _nextDt;
        bool _actionInstant;
        /** Inner action */
        FiniteTimeAction *_innerAction;

    private:
        DISALLOW_COPY_AND_ASSIGN(Repeat)
    };

    class RepeatForever : public ActionInterval
    {
    public:
        /** Creates the action.
         *
         * @param action The action need to repeat forever.
         * @return An autoreleased RepeatForever object.
         */
        static RepeatForever* create(ActionInterval *action);

        /** Sets the inner action.
         *
         * @param action The inner action.
         */
        inline void setInnerAction(ActionInterval *action)
        {
            if (_innerAction != action)
            {
                SAFE_RELEASE(_innerAction);
                _innerAction = action;
                SAFE_RETAIN(_innerAction);
            }
        }

        /** Gets the inner action.
         *
         * @return The inner action.
         */
        inline ActionInterval* getInnerAction()
        {
            return _innerAction;
        }

        virtual void startWithTarget(Node* target) override;
        /**
         * @param dt In seconds.
         */
        virtual void step(float dt) override;
        virtual bool isDone(void) const override;

    public:
        RepeatForever()
        : _innerAction(nullptr)
        {}
        virtual ~RepeatForever();

        /** initializes the action */
        bool initWithAction(ActionInterval *action);

    protected:
        /** Inner action */
        ActionInterval *_innerAction;

    private:
        DISALLOW_COPY_AND_ASSIGN(RepeatForever)
    };

    class Spawn : public ActionInterval
    {
    public:
        static Spawn* create(FiniteTimeAction *action1, ...);

        /** Helper constructor to create an array of spawned actions.
         *
         * @param action1   The first sequenceable action.
         * @param args  The va_list variable.
         * @return  An autoreleased Spawn object.
         * @js NA
         */
        static Spawn* createWithVariableList(FiniteTimeAction *action1, va_list args);

        /** Helper constructor to create an array of spawned actions given an array.
         *
         * @param arrayOfActions    An array of spawned actions.
         * @return  An autoreleased Spawn object.
         */
        static Spawn* create(const HObjectVector<FiniteTimeAction*>& arrayOfActions);

        /** Creates the Spawn action.
         *
         * @param action1   The first spawned action.
         * @param action2   THe second spawned action.
         * @return An autoreleased Spawn object.
         * @js NA
         */
        static Spawn* createWithTwoActions(FiniteTimeAction *action1, FiniteTimeAction *action2);

        virtual void startWithTarget(Node *target) override;
        virtual void stop(void) override;
        /**
         * @param time In seconds.
         */
        virtual void update(float time) override;

    public:
        Spawn() {}
        virtual ~Spawn();

        /** initializes the Spawn action with the 2 actions to spawn */
        bool initWithTwoActions(FiniteTimeAction *action1, FiniteTimeAction *action2);

    protected:
        FiniteTimeAction *_one;
        FiniteTimeAction *_two;

    private:
        DISALLOW_COPY_AND_ASSIGN(Spawn)
    };

    class RotateTo : public ActionInterval
    {
    public:
        /**
         * Creates the action with separate rotation angles.
         *
         * @param duration Duration time, in seconds.
         * @param dstAngleX In degreesCW.
         * @param dstAngleY In degreesCW.
         * @return An autoreleased RotateTo object.
         */
        static RotateTo* create(float duration, float dstAngleX, float dstAngleY);

        /**
         * Creates the action.
         *
         * @param duration Duration time, in seconds.
         * @param dstAngle In degreesCW.
         * @return An autoreleased RotateTo object.
         */
        static RotateTo* create(float duration, float dstAngle);

        virtual void startWithTarget(Node *target) override;
        /**
         * @param time In seconds.
         */
        virtual void update(float time) override;

    public:
        RotateTo();
        virtual ~RotateTo() {}

        /**
         * initializes the action
         * @param duration in seconds
         * @param dstAngleX in degreesCW
         * @param dstAngleY in degreesCW
         */
        bool initWithDuration(float duration, float dstAngleX, float dstAngleY);

        /**
         * calculates the start and diff angles
         * @param dstAngle in degreesCW
         */
        void calculateAngles(float &startAngle, float &diffAngle, float dstAngle);

    protected:
        bool _is3D;
        MATH::Vector3f _dstAngle;
        MATH::Vector3f _startAngle;
        MATH::Vector3f _diffAngle;

    private:
        DISALLOW_COPY_AND_ASSIGN(RotateTo)
    };

    /** @class RotateBy
     * @brief Rotates a Node object clockwise a number of degrees by modifying it's rotation attribute.
    */
    class RotateBy : public ActionInterval
    {
    public:
        /**
         * Creates the action.
         *
         * @param duration Duration time, in seconds.
         * @param deltaAngle In degreesCW.
         * @return An autoreleased RotateBy object.
         */
        static RotateBy* create(float duration, float deltaAngle);

        virtual void startWithTarget(Node *target) override;
        /**
         * @param time In seconds.
         */
        virtual void update(float time) override;

    public:
        RotateBy();
        virtual ~RotateBy() {}

        /** initializes the action */
        bool initWithDuration(float duration, float deltaAngle);
        /**
         * @warning The physics body contained in Node doesn't support rotate with different x and y angle.
         * @param deltaAngleZ_X in degreesCW
         * @param deltaAngleZ_Y in degreesCW
         */
        bool initWithDuration(float duration, float deltaAngleZ_X, float deltaAngleZ_Y);

    protected:
        MATH::Vector3f _deltaAngle;
        MATH::Vector3f _startAngle;

    private:
        DISALLOW_COPY_AND_ASSIGN(RotateBy)
    };

    /** @class MoveBy
     * @brief Moves a Node object x,y pixels by modifying it's position attribute.
     x and y are relative to the position of the object.
     Several MoveBy actions can be concurrently called, and the resulting
     movement will be the sum of individual movements.
     @since v2.1beta2-custom
     */
    class MoveBy : public ActionInterval
    {
    public:
        /**
         * Creates the action.
         *
         * @param duration Duration time, in seconds.
         * @param deltaPosition The delta distance in 2d, it's a Vec2 type.
         * @return An autoreleased MoveBy object.
         */
        static MoveBy* create(float duration, const MATH::Vector2f& deltaPosition);
        static MoveBy* create(float duration, const MATH::Vector3f& deltaPosition);

        virtual void startWithTarget(Node *target) override;
        /**
         * @param time in seconds
         */
        virtual void update(float time) override;

    public:
        MoveBy() {}
        virtual ~MoveBy() {}

        /** initializes the action */
        bool initWithDuration(float duration, const MATH::Vector2f& deltaPosition);
        bool initWithDuration(float duration, const MATH::Vector3f& deltaPosition);

    protected:
        MATH::Vector3f _positionDelta;
        MATH::Vector3f _startPosition;
        MATH::Vector3f _previousPosition;

    private:
        DISALLOW_COPY_AND_ASSIGN(MoveBy)
    };

    /** @class MoveTo
     * @brief Moves a Node object to the position x,y. x and y are absolute coordinates by modifying it's position attribute.
     Several MoveTo actions can be concurrently called, and the resulting
     movement will be the sum of individual movements.
     @since v2.1beta2-custom
     */
    class MoveTo : public MoveBy
    {
    public:
        /**
         * Creates the action.
         * @param duration Duration time, in seconds.
         * @param position The destination position in 2d.
         * @return An autoreleased MoveTo object.
         */
        static MoveTo* create(float duration, const MATH::Vector2f& position);
        static MoveTo* create(float duration, const MATH::Vector3f& position);

        virtual void startWithTarget(Node *target) override;

    public:
        MoveTo() {}
        virtual ~MoveTo() {}

        /**
         * initializes the action
         * @param duration in seconds
         */
        bool initWithDuration(float duration, const MATH::Vector2f& position);
        bool initWithDuration(float duration, const MATH::Vector3f& position);

    protected:
        MATH::Vector3f _endPosition;

    private:
        DISALLOW_COPY_AND_ASSIGN(MoveTo)
    };

    /** @class SkewTo
     * @brief Skews a Node object to given angles by modifying it's skewX and skewY attributes
    @since v1.0
    */
    class SkewTo : public ActionInterval
    {
    public:
        /**
         * Creates the action.
         * @param t Duration time, in seconds.
         * @param sx Skew x angle.
         * @param sy Skew y angle.
         * @return An autoreleased SkewTo object.
         */
        static SkewTo* create(float t, float sx, float sy);

        virtual void startWithTarget(Node *target) override;
        /**
         * @param time In seconds.
         */
        virtual void update(float time) override;

    public:
        SkewTo();
        virtual ~SkewTo() {}
        /**
         * @param t In seconds.
         */
        bool initWithDuration(float t, float sx, float sy);

    protected:
        float _skewX;
        float _skewY;
        float _startSkewX;
        float _startSkewY;
        float _endSkewX;
        float _endSkewY;
        float _deltaX;
        float _deltaY;

    private:
        DISALLOW_COPY_AND_ASSIGN(SkewTo)
    };

    /** @class SkewBy
    * @brief Skews a Node object by skewX and skewY degrees.
    @since v1.0
    */
    class SkewBy : public SkewTo
    {
    public:
        /**
         * Creates the action.
         * @param t Duration time, in seconds.
         * @param deltaSkewX Skew x delta angle.
         * @param deltaSkewY Skew y delta angle.
         * @return An autoreleased SkewBy object.
         */
        static SkewBy* create(float t, float deltaSkewX, float deltaSkewY);

        //
        // Overrides
        //
        virtual void startWithTarget(Node *target) override;

    public:
        SkewBy() {}
        virtual ~SkewBy() {}
        /**
         * @param t In seconds.
         */
        bool initWithDuration(float t, float sx, float sy);

    private:
        DISALLOW_COPY_AND_ASSIGN(SkewBy)
    };

    /** @class JumpBy
     * @brief Moves a Node object simulating a parabolic jump movement by modifying it's position attribute.
    */
    class JumpBy : public ActionInterval
    {
    public:
        /**
         * Creates the action.
         * @param duration Duration time, in seconds.
         * @param position The jumping distance.
         * @param height The jumping height.
         * @param jumps The jumping times.
         * @return An autoreleased JumpBy object.
         */
        static JumpBy* create(float duration, const MATH::Vector2f& position, float height, int jumps);

        virtual void startWithTarget(Node *target) override;
        /**
         * @param time In seconds.
         */
        virtual void update(float time) override;

    public:
        JumpBy() {}
        virtual ~JumpBy() {}

        /**
         * initializes the action
         * @param duration in seconds
         */
        bool initWithDuration(float duration, const MATH::Vector2f& position, float height, int jumps);

    protected:
        MATH::Vector2f           _startPosition;
        MATH::Vector2f           _delta;
        float           _height;
        int             _jumps;
        MATH::Vector2f           _previousPos;

    private:
        DISALLOW_COPY_AND_ASSIGN(JumpBy)
    };

    /** @class JumpTo
     * @brief Moves a Node object to a parabolic position simulating a jump movement by modifying it's position attribute.
    */
    class JumpTo : public JumpBy
    {
    public:
        /**
         * Creates the action.
         * @param duration Duration time, in seconds.
         * @param position The jumping destination position.
         * @param height The jumping height.
         * @param jumps The jumping times.
         * @return An autoreleased JumpTo object.
         */
        static JumpTo* create(float duration, const MATH::Vector2f& position, float height, int jumps);

        //
        // Override
        //
        virtual void startWithTarget(Node *target) override;

    public:
        JumpTo() {}
        virtual ~JumpTo() {}

        /**
         * initializes the action
         * @param duration In seconds.
         */
        bool initWithDuration(float duration, const MATH::Vector2f& position, float height, int jumps);

    protected:
        MATH::Vector2f _endPosition;

    private:
        DISALLOW_COPY_AND_ASSIGN(JumpTo)
    };

    /** @struct Bezier configuration structure
     */
    typedef struct _ccBezierConfig {
        //! end position of the bezier
        MATH::Vector2f endPosition;
        //! Bezier control point 1
        MATH::Vector2f controlPoint_1;
        //! Bezier control point 2
        MATH::Vector2f controlPoint_2;
    } ccBezierConfig;

    /** @class BezierBy
     * @brief An action that moves the target with a cubic Bezier curve by a certain distance.
     */
    class BezierBy : public ActionInterval
    {
    public:
        /** Creates the action with a duration and a bezier configuration.
         * @param t Duration time, in seconds.
         * @param c Bezier config.
         * @return An autoreleased BezierBy object.
         * @code
         * When this function bound to js or lua,the input params are changed.
         * in js: var create(var t,var table)
         * in lua: lcaol create(local t, local table)
         * @endcode
         */
        static BezierBy* create(float t, const ccBezierConfig& c);

        virtual void startWithTarget(Node *target) override;
        /**
         * @param time In seconds.
         */
        virtual void update(float time) override;

    public:
        BezierBy() {}
        virtual ~BezierBy() {}

        /**
         * initializes the action with a duration and a bezier configuration
         * @param t in seconds
         */
        bool initWithDuration(float t, const ccBezierConfig& c);

    protected:
        ccBezierConfig _config;
        MATH::Vector2f _startPosition;
        MATH::Vector2f _previousPosition;

    private:
        DISALLOW_COPY_AND_ASSIGN(BezierBy)
    };

    /** @class BezierTo
     * @brief An action that moves the target with a cubic Bezier curve to a destination point.
     @since v0.8.2
     */
    class BezierTo : public BezierBy
    {
    public:
        /** Creates the action with a duration and a bezier configuration.
         * @param t Duration time, in seconds.
         * @param c Bezier config.
         * @return An autoreleased BezierTo object.
         * @code
         * when this function bound to js or lua,the input params are changed
         * in js: var create(var t,var table)
         * in lua: lcaol create(local t, local table)
         * @endcode
         */
        static BezierTo* create(float t, const ccBezierConfig& c);

        //
        // Overrides
        //
        virtual void startWithTarget(Node *target) override;

    public:
        BezierTo() {}
        virtual ~BezierTo() {}
        /**
         * @param t In seconds.
         */
        bool initWithDuration(float t, const ccBezierConfig &c);

    protected:
        ccBezierConfig _toConfig;

    private:
        DISALLOW_COPY_AND_ASSIGN(BezierTo)
    };

    /** @class ScaleTo
     @brief Scales a Node object to a zoom factor by modifying it's scale attribute.
     @warning This action doesn't support "reverse".
     @warning The physics body contained in Node doesn't support this action.
     */
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

    /** @class ScaleBy
     * @brief Scales a Node object a zoom factor by modifying it's scale attribute.
     @warning The physics body contained in Node doesn't support this action.
    */
    class  ScaleBy : public ScaleTo
    {
    public:
        /**
         * Creates the action with the same scale factor for X and Y.
         * @param duration Duration time, in seconds.
         * @param s Scale factor of x and y.
         * @return An autoreleased ScaleBy object.
         */
        static ScaleBy* create(float duration, float s);

        /**
         * Creates the action with and X factor and a Y factor.
         * @param duration Duration time, in seconds.
         * @param sx Scale factor of x.
         * @param sy Scale factor of y.
         * @return An autoreleased ScaleBy object.
         */
        static ScaleBy* create(float duration, float sx, float sy);

        /**
         * Creates the action with X Y Z factor.
         * @param duration Duration time, in seconds.
         * @param sx Scale factor of x.
         * @param sy Scale factor of y.
         * @param sz Scale factor of z.
         * @return An autoreleased ScaleBy object.
         */
        static ScaleBy* create(float duration, float sx, float sy, float sz);

        //
        // Overrides
        //
        virtual void startWithTarget(Node *target) override;

    public:
        ScaleBy() {}
        virtual ~ScaleBy() {}

    private:
        DISALLOW_COPY_AND_ASSIGN(ScaleBy)
    };

    /** @class Blink
     * @brief Blinks a Node object by modifying it's visible attribute.
    */
    class  Blink : public ActionInterval
    {
    public:
        /**
         * Creates the action.
         * @param duration Duration time, in seconds.
         * @param blinks Blink times.
         * @return An autoreleased Blink object.
         */
        static Blink* create(float duration, int blinks);

        /**
         * @param time In seconds.
         */
        virtual void update(float time) override;
        virtual void startWithTarget(Node *target) override;
        virtual void stop() override;

    public:
        Blink() {}
        virtual ~Blink() {}

        /**
         * initializes the action
         * @param duration in seconds
         */
        bool initWithDuration(float duration, int blinks);

    protected:
        int _times;
        bool _originalState;

    private:
        DISALLOW_COPY_AND_ASSIGN(Blink)
    };


    /** @class FadeTo
     * @brief Fades an object that implements the RGBAProtocol protocol. It modifies the opacity from the current value to a custom one.
     @warning This action doesn't support "reverse"
     */
    class  FadeTo : public ActionInterval
    {
    public:
        /**
         * Creates an action with duration and opacity.
         * @param duration Duration time, in seconds.
         * @param opacity A certain opacity, the range is from 0 to 255.
         * @return An autoreleased FadeTo object.
         */
        static FadeTo* create(float duration, GLubyte opacity);

        virtual void startWithTarget(Node *target) override;
        /**
         * @param time In seconds.
         */
        virtual void update(float time) override;

    public:
        FadeTo() {}
        virtual ~FadeTo() {}

        /**
         * initializes the action with duration and opacity
         * @param duration in seconds
         */
        bool initWithDuration(float duration, GLubyte opacity);

    protected:
        GLubyte _toOpacity;
        GLubyte _fromOpacity;
        friend class FadeOut;
        friend class FadeIn;
    private:
        DISALLOW_COPY_AND_ASSIGN(FadeTo)
    };

    /** @class FadeIn
     * @brief Fades In an object that implements the RGBAProtocol protocol. It modifies the opacity from 0 to 255.
     The "reverse" of this action is FadeOut
     */
    class  FadeIn : public FadeTo
    {
    public:
        /**
         * Creates the action.
         * @param d Duration time, in seconds.
         * @return An autoreleased FadeIn object.
         */
        static FadeIn* create(float d);

        //
        // Overrides
        //
        virtual void startWithTarget(Node *target) override;

        /**
         * @js NA
         */
        void setReverseAction(FadeTo* ac);

    public:
        FadeIn():_reverseAction(nullptr) {}
        virtual ~FadeIn() {}

    private:
        DISALLOW_COPY_AND_ASSIGN(FadeIn)
        FadeTo* _reverseAction;
    };

    /** @class FadeOut
     * @brief Fades Out an object that implements the RGBAProtocol protocol. It modifies the opacity from 255 to 0.
     The "reverse" of this action is FadeIn
    */
    class  FadeOut : public FadeTo
    {
    public:
        /**
         * Creates the action.
         * @param d Duration time, in seconds.
         */
        static FadeOut* create(float d);

        //
        // Overrides
        //
        virtual void startWithTarget(Node *target) override;

        /**
         * @js NA
         */
        void setReverseAction(FadeTo* ac);

    public:
        FadeOut():_reverseAction(nullptr) {}
        virtual ~FadeOut() {}
    private:
        DISALLOW_COPY_AND_ASSIGN(FadeOut)
        FadeTo* _reverseAction;
    };

    /** @class TintTo
     * @brief Tints a Node that implements the NodeRGB protocol from current tint to a custom one.
     @warning This action doesn't support "reverse"
     @since v0.7.2
    */
    class  TintTo : public ActionInterval
    {
    public:
        /**
         * Creates an action with duration and color.
         * @param duration Duration time, in seconds.
         * @param red Red Color, from 0 to 255.
         * @param green Green Color, from 0 to 255.
         * @param blue Blue Color, from 0 to 255.
         * @return An autoreleased TintTo object.
         */
        static TintTo* create(float duration, GLubyte red, GLubyte green, GLubyte blue);
        /**
         * Creates an action with duration and color.
         * @param duration Duration time, in seconds.
         * @param color It's a Color3B type.
         * @return An autoreleased TintTo object.
         */
        static TintTo* create(float duration, const Color3B& color);

        virtual void startWithTarget(Node *target) override;
        /**
         * @param time In seconds.
         */
        virtual void update(float time) override;

    public:
        TintTo() {}
        virtual ~TintTo() {}

        /** initializes the action with duration and color */
        bool initWithDuration(float duration, GLubyte red, GLubyte green, GLubyte blue);

    protected:
        Color3B _to;
        Color3B _from;

    private:
        DISALLOW_COPY_AND_ASSIGN(TintTo)
    };

    /** @class TintBy
     @brief Tints a Node that implements the NodeRGB protocol from current tint to a custom one.
     @since v0.7.2
     */
    class  TintBy : public ActionInterval
    {
    public:
        /**
         * Creates an action with duration and color.
         * @param duration Duration time, in seconds.
         * @param deltaRed Delta red color.
         * @param deltaGreen Delta green color.
         * @param deltaBlue Delta blue color.
         * @return An autoreleased TintBy object.
         */
        static TintBy* create(float duration, GLshort deltaRed, GLshort deltaGreen, GLshort deltaBlue);

        virtual void startWithTarget(Node *target) override;
        /**
         * @param time In seconds.
         */
        virtual void update(float time) override;

    public:
        TintBy() {}
        virtual ~TintBy() {}

        /** initializes the action with duration and color */
        bool initWithDuration(float duration, GLshort deltaRed, GLshort deltaGreen, GLshort deltaBlue);

    protected:
        GLshort _deltaR;
        GLshort _deltaG;
        GLshort _deltaB;

        GLshort _fromR;
        GLshort _fromG;
        GLshort _fromB;

    private:
        DISALLOW_COPY_AND_ASSIGN(TintBy)
    };

    class DelayTime : public ActionInterval
    {
    public:
        /**
         * Creates the action.
         * @param d Duration time, in seconds.
         * @return An autoreleased DelayTime object.
         */
        static DelayTime* create(float d);

        //
        // Overrides
        //
        /**
         * @param time In seconds.
         */
        virtual void update(float time) override;

    public:
        DelayTime() {}
        virtual ~DelayTime() {}

    private:
        DISALLOW_COPY_AND_ASSIGN(DelayTime)
    };

    /**
     * @class ActionFloat
     * @brief Action used to animate any value in range [from,to] over specified time interval
     */
    class  ActionFloat : public ActionInterval
    {
    public:
        /**
         *  Callback function used to report back result
         */
        typedef std::function<void(float value)> ActionFloatCallback;

        /**
         * Creates FloatAction with specified duration, from value, to value and callback to report back
         * results
         * @param duration of the action
         * @param from value to start from
         * @param to value to be at the end of the action
         * @param callback to report back result
         *
         * @return An autoreleased ActionFloat object
         */
        static ActionFloat* create(float duration, float from, float to, ActionFloatCallback callback);

        /**
         * Overrided ActionInterval methods
         */
        void startWithTarget(Node* target) override;
        void update(float delta) override;

    public:
        ActionFloat() {}
        virtual ~ActionFloat() {}

        bool initWithDuration(float duration, float from, float to, ActionFloatCallback callback);

    protected:
        /* From value */
        float _from;
        /* To value */
        float _to;
        /* delta time */
        float _delta;

        /* Callback to report back results */
        ActionFloatCallback _callback;
    private:
        DISALLOW_COPY_AND_ASSIGN(ActionFloat)
    };
}

#endif //__ACTION_CCINTERVAL_ACTION_H__
