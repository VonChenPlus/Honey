#ifndef VIEWGROUP_H
#define VIEWGROUP_H

#include "SMARTGRAPH/UI/View.h"

namespace UI
{
    struct NeighborResult
    {
        NeighborResult() : view(0), score(0) {}
        NeighborResult(View *v, float s) : view(v), score(s) {}

        View *view;
        float score;
    };

    enum {
        NONE = -1,
    };

    class ViewGroup : public View
    {
    public:
        ViewGroup(LayoutParams *layoutParams = 0)
            : View(layoutParams), defaultFocusView_(0), hasDropShadow_(false), clip_(false) {}
        virtual ~ViewGroup();

        // Pass through external events to children.
        virtual bool key(const KeyInput &input) override;
        virtual void touch(const TouchInput &input) override;
        virtual void axis(const AxisInput &input) override;

        // By default, a container will layout to its own bounds.
        virtual void measure(const UIContext &dc, MeasureSpec horiz, MeasureSpec vert) = 0;
        virtual void layout() = 0;
        virtual void update(const InputState &input_state);

        virtual void draw(UIContext &dc) override;

        // These should be unused.
        virtual float getContentWidth() const { return 0.0f; }
        virtual float getContentHeight() const { return 0.0f; }

        // Takes ownership! DO NOT add a view to multiple parents!
        template <class T>
        T *add(T *view) {
            lock_guard guard(modifyLock_);
            views_.push_back(view);
            return view;
        }

        virtual bool setFocus() override;
        virtual bool subviewFocused(View *view);
        virtual void removeSubview(View *view);

        void setDefaultFocusView(View *view) { defaultFocusView_ = view; }
        View *getDefaultFocusView() { return defaultFocusView_; }

        // Assumes that layout has taken place.
        NeighborResult findNeighbor(View *view, FocusDirection direction, NeighborResult best);

        virtual bool canBeFocused() const { return false; }
        virtual bool isViewGroup() const { return true; }

        virtual void setBG(const Drawable &bg) { bg_ = bg; }

        virtual void clear();
        View *getViewByIndex(int index) { return views_[index]; }
        int getNumSubviews() const { return (int)views_.size(); }
        void setHasDropShadow(bool has) { hasDropShadow_ = has; }

        void lock() { modifyLock_.lock(); }
        void unlock() { modifyLock_.unlock(); }

        void setClip(bool clip) { clip_ = clip; }

    protected:
        recursive_mutex modifyLock_;  // Hold this when changing the subviews.
        std::vector<View *> views_;
        View *defaultFocusView_;
        Drawable bg_;
        bool hasDropShadow_;
        bool clip_;
    };

    class AnchorLayoutParams : public LayoutParams
    {
    public:
        AnchorLayoutParams(float w, float h, float l, float t, float r, float b, bool c = false)
            : LayoutParams(w, h, LP_ANCHOR), left(l), top(t), right(r), bottom(b), center(c) {

        }
        AnchorLayoutParams(float w, float h, bool c = false)
            : LayoutParams(w, h, LP_ANCHOR), left(0), top(0), right(NONE), bottom(NONE), center(c) {
        }
        AnchorLayoutParams(float l, float t, float r, float b, bool c = false)
            : LayoutParams(WRAP_CONTENT, WRAP_CONTENT, LP_ANCHOR), left(l), top(t), right(r), bottom(b), center(c) {}

        // These are not bounds, but distances from the container edges.
        // Set to NONE to not attach this edge to the container.
        float left, top, right, bottom;
        bool center;  // If set, only two "sides" can be set, and they refer to the center, not the edge, of the view being layouted.
    };

    class AnchorLayout : public ViewGroup
    {
    public:
        AnchorLayout(LayoutParams *layoutParams = 0) : ViewGroup(layoutParams) {}
        void measure(const UIContext &dc, MeasureSpec horiz, MeasureSpec vert) override;
        void layout() override;
    };

    class LinearLayoutParams : public LayoutParams
    {
    public:
        LinearLayoutParams()
            : LayoutParams(LP_LINEAR), weight(0.0f), gravity(G_TOPLEFT), hasMargins_(false) {}
        explicit LinearLayoutParams(float wgt, Gravity grav = G_TOPLEFT)
            : LayoutParams(LP_LINEAR), weight(wgt), gravity(grav), hasMargins_(false) {}
        LinearLayoutParams(float wgt, const Margins &mgn)
            : LayoutParams(LP_LINEAR), weight(wgt), gravity(G_TOPLEFT), margins(mgn), hasMargins_(true) {}
        LinearLayoutParams(float w, float h, float wgt = 0.0f, Gravity grav = G_TOPLEFT)
            : LayoutParams(w, h, LP_LINEAR), weight(wgt), gravity(grav), hasMargins_(false) {}
        LinearLayoutParams(float w, float h, float wgt, Gravity grav, const Margins &mgn)
            : LayoutParams(w, h, LP_LINEAR), weight(wgt), gravity(grav), margins(mgn), hasMargins_(true) {}
        LinearLayoutParams(float w, float h, const Margins &mgn)
            : LayoutParams(w, h, LP_LINEAR), weight(0.0f), gravity(G_TOPLEFT), margins(mgn), hasMargins_(true) {}
        LinearLayoutParams(float w, float h, float wgt, const Margins &mgn)
            : LayoutParams(w, h, LP_LINEAR), weight(wgt), gravity(G_TOPLEFT), margins(mgn), hasMargins_(true) {}
        LinearLayoutParams(const Margins &mgn)
            : LayoutParams(WRAP_CONTENT, WRAP_CONTENT, LP_LINEAR), weight(0.0f), gravity(G_TOPLEFT), margins(mgn), hasMargins_(true) {}

        float weight;
        Gravity gravity;
        Margins margins;

        bool hasMargins() const { return hasMargins_; }

    private:
        bool hasMargins_;
    };

    class LinearLayout : public ViewGroup
    {
    public:
        LinearLayout(Orientation orientation, LayoutParams *layoutParams = 0)
            : ViewGroup(layoutParams), orientation_(orientation), defaultMargins_(0), spacing_(10) {}

        void measure(const UIContext &dc, MeasureSpec horiz, MeasureSpec vert) override;
        void layout() override;
        void setSpacing(float spacing) {
            spacing_ = spacing;
        }
    protected:
        Orientation orientation_;
    private:
        Margins defaultMargins_;
        float spacing_;
    };
}

#endif // VIEWGROUP_H
