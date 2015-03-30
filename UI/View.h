#ifndef VIEW_H
#define VIEW_H

#include <string>

#include "BASE/BasicTypes.h"
#include "INPUT/InputState.h"
#include "MATH/Bounds.h"
#include "MATH/Matrix.h"
#include "BASE/SmartPtr.h"

namespace UI
{
    enum MeasureSpecType
    {
        UNSPECIFIED,
        EXACTLY,
        AT_MOST,
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
        LayoutParams(Size w, Size h, LayoutParamsType type = LP_PLAIN)
            : width(w), height(h), type_(type) {}
        virtual ~LayoutParams() {}
        Size width;
        Size height;

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
            , enabledMeansDisabled_(false)
        {
            if (!layoutParams)
                layoutParams_.reset(new LayoutParams());
        }
        virtual ~View();

        // Please note that Touch is called ENTIRELY asynchronously from drawing!
        // Can even be called on a different thread! This is to really minimize latency, and decouple
        // touch response from the frame rate. Same with Key and Axis.
        virtual bool key(const Input::KeyInput &input) { UNUSED(input); return false; }
        virtual void touch(const Input::TouchInput &input) { UNUSED(input); }
        virtual void axis(const Input::AxisInput &input) { UNUSED(input); }
        virtual void update(const Input::InputState &input_state) { UNUSED(input_state); }

        virtual void focusChanged(int focusFlags) {}

        void move(Bounds bounds) {
            bounds_ = bounds;
        }

        // Views don't do anything here in Layout, only containers implement this.
        virtual void measure(const UIContext &dc, MeasureSpec horiz, MeasureSpec vert);
        virtual void layout() {}
        virtual void draw(UIContext &dc) {}

        virtual float getMeasuredWidth() const { return measuredWidth_; }
        virtual float getMeasuredHeight() const { return measuredHeight_; }

        // Override this for easy standard behaviour. No need to override Measure.
        virtual void getContentDimensions(const UIContext &dc, float &w, float &h) const;

        // Called when the layout is done.
        void setBounds(Bounds bounds) { bounds_ = bounds; }
        virtual const LayoutParams *getLayoutParams() const { return layoutParams_.get(); }
        virtual void replaceLayoutParams(LayoutParams *newLayoutParams) { layoutParams_.reset(newLayoutParams); }
        const Bounds &getBounds() const { return bounds_; }

        virtual bool setFocus();

        virtual bool canBeFocused() const { return true; }
        virtual bool subviewFocused(View *view) { return false; }

        void setEnabled(bool enabled) { enabled_ = enabled; enabledMeansDisabled_ = false; }
        bool isEnabled() const
        {
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

}

#endif // VIEW_H
