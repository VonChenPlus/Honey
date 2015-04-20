#include "View.h"

#include <queue>

using MATH::Point;
using MATH::Bounds;

namespace UI
{
    extern void EventTriggered(Event *e, EventParams params);
    extern void RemoveQueuedEvents(View *v);
    extern View *GetFocusedView();
    extern void SetFocusedView(View *view, bool force);
    extern bool IsFocusMovementEnabled();

    void MeasureBySpec(float sz, float contentWidth, MeasureSpec spec, float *measured) {
        *measured = sz;
        if (sz == WRAP_CONTENT) {
            if (spec.type == UNSPECIFIED || spec.type == AT_MOST)
                *measured = contentWidth;
            else if (spec.type == EXACTLY)
                *measured = spec.size;
        }
        else if (sz == FILL_PARENT) {
            if (spec.type == UNSPECIFIED)
                *measured = contentWidth;  // We have no value to set
            else
                *measured = spec.size;
        }
        else if (spec.type == EXACTLY || (spec.type == AT_MOST && *measured > spec.size)) {
            *measured = spec.size;
        }
    }

    View::~View() {
        if (hasFocus())
                SetFocusedView(0, false);
        RemoveQueuedEvents(this);
    }

    void View::measure(const UIContext &dc, MeasureSpec horiz, MeasureSpec vert) {
        float contentW = 0.0f, contentH = 0.0f;
        getContentDimensions(dc, contentW, contentH);
        MeasureBySpec(layoutParams_->width, contentW, horiz, &measuredWidth_);
        MeasureBySpec(layoutParams_->height, contentH, vert, &measuredHeight_);
    }

    // Default values

    void View::getContentDimensions(const UIContext &dc, float &w, float &h) const {
        UNUSED(dc);
        w = 10.0f;
        h = 10.0f;
    }

    Point View::getFocusPosition(FocusDirection dir) {
        // The +2/-2 is some extra fudge factor to cover for views sitting right next to each other.
        // Distance zero yields strange results otherwise.
        switch (dir) {
        case FOCUS_LEFT: return Point(bounds_.x + 2, bounds_.centerY());
        case FOCUS_RIGHT: return Point(bounds_.x2() - 2, bounds_.centerY());
        case FOCUS_UP: return Point(bounds_.centerX(), bounds_.y + 2);
        case FOCUS_DOWN: return Point(bounds_.centerX(), bounds_.y2() - 2);

        default:
            return bounds_.center();
        }
    }

    bool View::setFocus() {
        if (IsFocusMovementEnabled()) {
            if (canBeFocused()) {
                SetFocusedView(this, false);
                return true;
            }
        }
        return false;
    }

    bool View::hasFocus() const {
        return GetFocusedView() == this;
    }

    void Event::add(std::function<EventReturn(EventParams&)> func) {
        HandlerRegistration reg;
        reg.func = func;
        handlers_.push_back(reg);
    }

    // Call this from input thread or whatever, it doesn't matter
    void Event::trigger(EventParams &e) {
        EventTriggered(this, e);
    }

    // Call this from UI thread
    EventReturn Event::dispatch(EventParams &e) {
        for (auto iter = handlers_.begin(); iter != handlers_.end(); ++iter) {
            if ((iter->func)(e) == EVENT_DONE) {
                // Event is handled, stop looping immediately. This event might even have gotten deleted.
                return EVENT_DONE;
            }
        }
        return EVENT_SKIPPED;
    }
}
