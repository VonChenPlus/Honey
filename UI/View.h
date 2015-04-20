#ifndef VIEW_H
#define VIEW_H

#include <string>
#include <functional>

#include "BASE/Native.h"
#include "INPUT/InputState.h"
#include "MATH/Bounds.h"
#include "MATH/Matrix.h"
#include "BASE/SmartPtr.h"
#include "UI/UIContext.h"
#include "UI/Theme.h"

namespace UI
{
    enum MeasureSpecType
    {
        UNSPECIFIED,
        EXACTLY,
        AT_MOST,
    };

    class View;
    // Should cover all bases.
    struct EventParams
    {
        View *v;
        uint32_t a, b, x, y;
        float f;
        std::string s;
    };

    // I hope I can find a way to simplify this one day.
    enum EventReturn
    {
        EVENT_DONE,  // Return this when no other view may process this event, for example if you changed the view hierarchy
        EVENT_SKIPPED,  // Return this if you ignored an event
        EVENT_CONTINUE,  // Return this if it's safe to send this event to further listeners. This should normally be the default choice but often EVENT_DONE is necessary.
    };

    struct MeasureSpec
    {
        MeasureSpec(MeasureSpecType t, float s = 0.0f) : type(t), size(s) {}
        MeasureSpec() : type(UNSPECIFIED), size(0) {}

        MeasureSpec operator -(float amount) {
            // TODO: Check type
            return MeasureSpec(type, size - amount);
        }
        MeasureSpecType type;
        float size;
    };

    // The four cardinal directions should be enough, plus Prev/Next in "element order".
    enum FocusDirection
    {
        FOCUS_UP,
        FOCUS_DOWN,
        FOCUS_LEFT,
        FOCUS_RIGHT,
        FOCUS_NEXT,
        FOCUS_PREV,
    };

    inline FocusDirection Opposite(FocusDirection d) {
        switch (d) {
        case FOCUS_UP: return FOCUS_DOWN;
        case FOCUS_DOWN: return FOCUS_UP;
        case FOCUS_LEFT: return FOCUS_RIGHT;
        case FOCUS_RIGHT: return FOCUS_LEFT;
        case FOCUS_PREV: return FOCUS_NEXT;
        case FOCUS_NEXT: return FOCUS_PREV;
        }
        return d;
    }

    enum FocusFlags
    {
        FF_LOSTFOCUS = 1,
        FF_GOTFOCUS = 2
    };

    enum
    {
        WRAP_CONTENT = -1,
        FILL_PARENT = -2,
    };

    enum LayoutParamsType
    {
        LP_PLAIN = 0,
        LP_LINEAR = 1,
        LP_ANCHOR = 2,
    };

    // Need a virtual destructor so vtables are created, otherwise RTTI can't work
    class LayoutParams
    {
    public:
        LayoutParams(LayoutParamsType type = LP_PLAIN)
            : width(WRAP_CONTENT), height(WRAP_CONTENT), type_(type) {}
        LayoutParams(float w, float h, LayoutParamsType type = LP_PLAIN)
            : width(w), height(h), type_(type) {}
        virtual ~LayoutParams() {}
        float width;
        float height;

        // Fake RTTI
        bool is(LayoutParamsType type) const { return type_ == type; }

    private:
        LayoutParamsType type_;
    };

    enum Visibility
    {
        V_VISIBLE,
        V_INVISIBLE,  // Keeps position, not drawn or interacted with
        V_GONE,  // Does not participate in layout
    };

    class View
    {
    public:
        View(LayoutParams *layoutParams = 0)
            : layoutParams_(layoutParams)
            , visibility_(V_VISIBLE)
            , measuredWidth_(0)
            , measuredHeight_(0)
            , enabledPtr_(0)
            , enabled_(true)
            , enabledMeansDisabled_(false) {
            if (!layoutParams)
                layoutParams_.reset(new LayoutParams());
        }
        virtual ~View();

        // Please note that Touch is called ENTIRELY asynchronously from drawing!
        // Can even be called on a different thread! This is to really minimize latency, and decouple
        // touch response from the frame rate. Same with Key and Axis.
        virtual bool key(const _INPUT::KeyInput &input) { UNUSED(input); return false; }
        virtual void touch(const _INPUT::TouchInput &input) { UNUSED(input); }
        virtual void axis(const _INPUT::AxisInput &input) { UNUSED(input); }
        virtual void update(const _INPUT::InputState &input_state) { UNUSED(input_state); }

        virtual void focusChanged(int focusFlags) { UNUSED(focusFlags); }

        void move(MATH::Bounds bounds) {
            bounds_ = bounds;
        }

        // Views don't do anything here in Layout, only containers implement this.
        virtual void measure(const UI::UIContext &dc, MeasureSpec horiz, MeasureSpec vert);
        virtual void layout() {}
        virtual void draw(UIContext &dc) { UNUSED(dc); }

        virtual float getMeasuredWidth() const { return measuredWidth_; }
        virtual float getMeasuredHeight() const { return measuredHeight_; }

        // Override this for easy standard behaviour. No need to override Measure.
        virtual void getContentDimensions(const UIContext &dc, float &w, float &h) const;

        // Called when the layout is done.
        void setBounds(MATH::Bounds bounds) { bounds_ = bounds; }
        virtual const LayoutParams *getLayoutParams() const { return layoutParams_.get(); }
        virtual void replaceLayoutParams(LayoutParams *newLayoutParams) { layoutParams_.reset(newLayoutParams); }
        const MATH::Bounds &getBounds() const { return bounds_; }

        virtual bool setFocus();
        bool hasFocus() const;
        virtual bool canBeFocused() const { return true; }
        virtual bool subviewFocused(View *view) { UNUSED(view); return false; }

        void setEnabled(bool enabled) { enabled_ = enabled; enabledMeansDisabled_ = false; }
        bool isEnabled() const {
            if (enabledPtr_)
                return *enabledPtr_ != enabledMeansDisabled_;
            else
                return enabled_ != enabledMeansDisabled_;
        }
        void setEnabledPtr(bool *enabled) { enabledPtr_ = enabled; enabledMeansDisabled_ = false; }
        void setDisabledPtr(bool *disabled) { enabledPtr_ = disabled; enabledMeansDisabled_ = true;  }

        void setVisibility(Visibility visibility) { visibility_ = visibility; }
        Visibility getVisibility() const { return visibility_; }

        const std::string &tag() const { return tag_; }
        void setTag(const std::string &str) { tag_ = str; }

        // Fake RTTI
        virtual bool isViewGroup() const { return false; }

        MATH::Point getFocusPosition(FocusDirection dir);

    protected:
        // Inputs to layout
        scoped_ptr<LayoutParams> layoutParams_;

        std::string tag_;
        Visibility visibility_;

        // Results of measure pass. Set these in Measure.
        float measuredWidth_;
        float measuredHeight_;

        // Outputs of layout. X/Y are absolute screen coordinates, hierarchy is "gone" here.
        MATH::Bounds bounds_;

        scoped_ptr<MATH::Matrix4x4> transform_;

    private:
        bool *enabledPtr_;
        bool enabled_;
        bool enabledMeansDisabled_;

        DISALLOW_COPY_AND_ASSIGN(View)
    };

    struct HandlerRegistration {
        std::function<EventReturn(EventParams&)> func;
    };
    class Event {
    public:
        Event() {}
        ~Event() {
            handlers_.clear();
        }
        // Call this from input thread or whatever, it doesn't matter
        void trigger(EventParams &e);
        // Call this from UI thread
        EventReturn dispatch(EventParams &e);

        // This is suggested for use in most cases. Autobinds, allowing for neat syntax.
        template<class T>
        T *handle(T *thiz, EventReturn (T::* theCallback)(EventParams &e)) {
            add(std::bind(theCallback, thiz, std::placeholders::_1));
            return thiz;
        }

        // Sometimes you have an already-bound function<>, just use this then.
        void add(std::function<EventReturn(EventParams&)> func);

    private:
        std::vector<HandlerRegistration> handlers_;
        DISALLOW_COPY_AND_ASSIGN(Event)
    };
}

#endif // VIEW_H
