#ifndef VIEWGROUP_H
#define VIEWGROUP_H

#include "UI/View.h"

namespace UI
{
    struct NeighborResult
    {
        NeighborResult() : view(0), score(0) {}
        NeighborResult(View *v, float s) : view(v), score(s) {}

        View *view;
        float score;
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
}

#endif // VIEWGROUP_H
