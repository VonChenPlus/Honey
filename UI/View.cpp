#include "View.h"
using MATH::Point;
using MATH::Bounds;

namespace UI
{
    View::~View()
    {
    }

    void View::measure(const UIContext &dc, MeasureSpec horiz, MeasureSpec vert) {
        float contentW = 0.0f, contentH = 0.0f;
        GetContentDimensions(dc, contentW, contentH);
        MeasureBySpec(layoutParams_->width, contentW, horiz, &measuredWidth_);
        MeasureBySpec(layoutParams_->height, contentH, vert, &measuredHeight_);
    }

    // Default values

    void View::getContentDimensions(const UIContext &dc, float &w, float &h) const
    {
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
            return bounds_.Center();
        }
    }

    bool View::SetFocus()
    {
        return false;
    }
}
