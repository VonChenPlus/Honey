#ifndef VIEW_H
#define VIEW_H

#include <string>

#include "BASE/Honey.h"
#include "GRAPH/UI/InputState.h"
#include "MATH/Bounds.h"
#include "MATH/Matrix.h"
#include "BASE/SmartPtr.h"
#include "GRAPH/UI/UIContext.h"
#include "GRAPH/UI/Theme.h"
#include "GRAPH/UI/UIEvent.h"

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

    enum Gravity {
        G_LEFT = 0,
        G_RIGHT = 1,
        G_HCENTER = 2,

        G_HORIZMASK = 3,

        G_TOP = 0,
        G_BOTTOM = 4,
        G_VCENTER = 8,

        G_TOPLEFT = G_TOP | G_LEFT,
        G_TOPRIGHT = G_TOP | G_RIGHT,

        G_BOTTOMLEFT = G_BOTTOM | G_LEFT,
        G_BOTTOMRIGHT = G_BOTTOM | G_RIGHT,

        G_CENTER = G_HCENTER | G_VCENTER,

        G_VERTMASK = 3 << 2,
    };

    enum Orientation {
        ORIENT_HORIZONTAL,
        ORIENT_VERTICAL,
    };

    inline Orientation Opposite(Orientation o) {
        if (o == ORIENT_HORIZONTAL) return ORIENT_VERTICAL; else return ORIENT_HORIZONTAL;
    }

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

    struct Margins {
        Margins() : top(0), bottom(0), left(0), right(0) {}
        explicit Margins(int8 all) : top(all), bottom(all), left(all), right(all) {}
        Margins(int8 horiz, int8 vert) : top(vert), bottom(vert), left(horiz), right(horiz) {}
        Margins(int8 l, int8 t, int8 r, int8 b) : top(t), bottom(b), left(l), right(r) {}

        int8 top;
        int8 bottom;
        int8 left;
        int8 right;
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
        virtual bool key(const KeyInput &input) { UNUSED(input); return false; }
        virtual void touch(const TouchInput &input) { UNUSED(input); }
        virtual void axis(const AxisInput &input) { UNUSED(input); }
        virtual void update(const InputState &input_state) { UNUSED(input_state); }

        virtual void focusChanged(int focusFlags) { UNUSED(focusFlags); }

        void move(MATH::Boundsf bounds) {
            bounds_ = bounds;
        }

        // Views don't do anything here in Layout, only containers implement this.
        virtual void measure(const UIContext &dc, MeasureSpec horiz, MeasureSpec vert);
        virtual void layout() {}
        virtual void draw(UIContext &dc) { UNUSED(dc); }

        virtual float getMeasuredWidth() const { return measuredWidth_; }
        virtual float getMeasuredHeight() const { return measuredHeight_; }

        // Override this for easy standard behaviour. No need to override Measure.
        virtual void getContentDimensions(const UIContext &dc, float &w, float &h) const;

        // Called when the layout is done.
        void setBounds(MATH::Boundsf bounds) { bounds_ = bounds; }
        virtual const LayoutParams *getLayoutParams() const { return layoutParams_.get(); }
        virtual void replaceLayoutParams(LayoutParams *newLayoutParams) { layoutParams_.reset(newLayoutParams); }
        const MATH::Boundsf &getBounds() const { return bounds_; }

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

        MATH::Vector2f getFocusPosition(FocusDirection dir);

    protected:
        // Inputs to layout
        scoped_ptr<LayoutParams> layoutParams_;

        std::string tag_;
        Visibility visibility_;

        // Results of measure pass. Set these in Measure.
        float measuredWidth_;
        float measuredHeight_;

        // Outputs of layout. X/Y are absolute screen coordinates, hierarchy is "gone" here.
        MATH::Boundsf bounds_;

        scoped_ptr<MATH::Matrix4> transform_;

    private:
        bool *enabledPtr_;
        bool enabled_;
        bool enabledMeansDisabled_;

        DISALLOW_COPY_AND_ASSIGN(View)
    };

    // These don't do anything when touched.
    class InertView : public View
    {
    public:
        InertView(LayoutParams *layoutParams)
            : View(layoutParams) {}

        bool key(const KeyInput &) override { return false; }
        void touch(const TouchInput &) override {}
        bool canBeFocused() const override { return false; }
        void update(const InputState &) override {}
    };

    class Item : public InertView
    {
    public:
        Item(LayoutParams *layoutParams);
        void getContentDimensions(const UIContext &dc, float &w, float &h) const override;
    };
}

#endif // VIEW_H
