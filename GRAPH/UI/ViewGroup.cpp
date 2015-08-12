#include "ViewGroup.h"

#include <algorithm>

#include "MATH/Bounds.h"
using MATH::Boundsf;

namespace UI
{
    extern void MeasureBySpec(float sz, float contentWidth, MeasureSpec spec, float *measured);
    extern float GetDirectionScore(View *origin, View *destination, FocusDirection direction);

    void ApplyGravity(const Boundsf outer, const Margins &margins, float width, float height, int gravity, Boundsf &inner) {
        inner.width = width - (margins.left + margins.right);
        inner.height = height - (margins.right + margins.left);

        switch (gravity & G_HORIZMASK) {
        case G_LEFT: inner.left = outer.left + margins.left; break;
        case G_RIGHT: inner.left = outer.left + outer.width - width - margins.right; break;
        case G_HCENTER: inner.left = outer.left + (outer.width - width) / 2; break;
        }

        switch (gravity & G_VERTMASK) {
        case G_TOP: inner.top = outer.top + margins.top; break;
        case G_BOTTOM: inner.top = outer.top + outer.height - height - margins.bottom; break;
        case G_VCENTER: inner.top = outer.top + (outer.height - height) / 2; break;
        }
    }

    ViewGroup::~ViewGroup() {
        // Tear down the contents recursively.
        clear();
    }

    void ViewGroup::removeSubview(View *view) {
        lock_guard guard(modifyLock_);
        for (size_t i = 0; i < views_.size(); i++) {
            if (views_[i] == view) {
                views_.erase(views_.begin() + i);
                delete view;
                return;
            }
        }
    }

    NeighborResult ViewGroup::findNeighbor(View *view, FocusDirection direction, NeighborResult result) {
        if (!isEnabled())
            return result;
        if (getVisibility() != V_VISIBLE)
            return result;

        // First, find the position of the view in the list.
        int num = -1;
        for (size_t i = 0; i < views_.size(); i++) {
            if (views_[i] == view) {
                num = (int)i;
                break;
            }
        }

        // TODO: Do the cardinal directions right. Now we just map to
        // prev/next.

        switch (direction) {
        case FOCUS_PREV:
            // If view not found, no neighbor to find.
            if (num == -1)
                return NeighborResult(0, 0.0f);
            return NeighborResult(views_[(num + views_.size() - 1) % views_.size()], 0.0f);

        case FOCUS_NEXT:
            // If view not found, no neighbor to find.
            if (num == -1)
                return NeighborResult(0, 0.0f);
            return NeighborResult(views_[(num + 1) % views_.size()], 0.0f);

        case FOCUS_UP:
        case FOCUS_LEFT:
        case FOCUS_RIGHT:
        case FOCUS_DOWN:
            {
                // First, try the child views themselves as candidates
                for (size_t i = 0; i < views_.size(); i++) {
                    if (views_[i] == view)
                        continue;

                    float score = GetDirectionScore(view, views_[i], direction);
                    if (score > result.score) {
                        result.score = score;
                        result.view = views_[i];
                    }
                }

                // Then go right ahead and see if any of the children contain any better candidates.
                for (auto iter = views_.begin(); iter != views_.end(); ++iter) {
                    if ((*iter)->isViewGroup()) {
                        ViewGroup *vg = static_cast<ViewGroup *>(*iter);
                        if (vg)
                            result = vg->findNeighbor(view, direction, result);
                    }
                }

                // Boost neighbors with the same parent
                if (num != -1) {
                    //result.score += 100.0f;
                }

                return result;
            }

        default:
            return result;
        }
    }

    void ViewGroup::clear() {
        lock_guard guard(modifyLock_);
        for (size_t i = 0; i < views_.size(); i++) {
            delete views_[i];
            views_[i] = 0;
        }
        views_.clear();
    }

    void ViewGroup::touch(const TouchInput &input) {
        lock_guard guard(modifyLock_);
        for (auto iter = views_.begin(); iter != views_.end(); ++iter) {
            // TODO: If there is a transformation active, transform input coordinates accordingly.
            if ((*iter)->getVisibility() == V_VISIBLE)
                (*iter)->touch(input);
        }
    }

    bool ViewGroup::key(const KeyInput &input) {
        lock_guard guard(modifyLock_);
        bool ret = false;
        for (auto iter = views_.begin(); iter != views_.end(); ++iter) {
            // TODO: If there is a transformation active, transform input coordinates accordingly.
            if ((*iter)->getVisibility() == V_VISIBLE)
                ret = ret || (*iter)->key(input);
        }
        return ret;
    }

    void ViewGroup::axis(const AxisInput &input) {
        lock_guard guard(modifyLock_);
        for (auto iter = views_.begin(); iter != views_.end(); ++iter) {
            // TODO: If there is a transformation active, transform input coordinates accordingly.
            if ((*iter)->getVisibility() == V_VISIBLE)
                (*iter)->axis(input);
        }
    }

    void ViewGroup::draw(UIContext &dc) {
        if (hasDropShadow_) {
            // Darken things behind.
            dc.fillRect(Drawable(0x60000000), dc.getBounds());
            float dropsize = 30;
            dc.draw()->drawImage4Grid(dc.theme->dropShadow4Grid,
                bounds_.left - dropsize, bounds_.top,
                bounds_.right() + dropsize, bounds_.bottom()+dropsize*1.5, 0xDF000000, 3.0f);
        }

        if (clip_) {
            dc.pushScissor(bounds_);
        }

        dc.fillRect(bg_, bounds_);
        for (auto iter = views_.begin(); iter != views_.end(); ++iter) {
            // TODO: If there is a transformation active, transform input coordinates accordingly.
            if ((*iter)->getVisibility() == V_VISIBLE) {
                // Check if bounds are in current scissor rectangle.
                if (dc.getScissorBounds().intersects((*iter)->getBounds()))
                    (*iter)->draw(dc);
            }
        }
        if (clip_) {
            dc.popScissor();
        }
    }

    void ViewGroup::update(const InputState &input_state) {
        for (auto iter = views_.begin(); iter != views_.end(); ++iter) {
            // TODO: If there is a transformation active, transform input coordinates accordingly.
            if ((*iter)->getVisibility() != V_GONE)
                (*iter)->update(input_state);
        }
    }

    bool ViewGroup::setFocus() {
        lock_guard guard(modifyLock_);
        if (!canBeFocused() && !views_.empty()) {
            for (size_t i = 0; i < views_.size(); i++) {
                if (views_[i]->setFocus())
                    return true;
            }
        }
        return false;
    }

    bool ViewGroup::subviewFocused(View *view) {
        for (size_t i = 0; i < views_.size(); i++) {
            if (views_[i] == view)
                return true;
            if (views_[i]->subviewFocused(view))
                return true;
        }
        return false;
    }

    void AnchorLayout::measure(const UIContext &dc, MeasureSpec horiz, MeasureSpec vert) {
        MeasureBySpec(layoutParams_->width, 0.0f, horiz, &measuredWidth_);
        MeasureBySpec(layoutParams_->height, 0.0f, vert, &measuredHeight_);

        for (size_t i = 0; i < views_.size(); i++) {
            int width = WRAP_CONTENT;
            int height = WRAP_CONTENT;

            MeasureSpec specW(UNSPECIFIED, 0.0f);
            MeasureSpec specH(UNSPECIFIED, 0.0f);

            const AnchorLayoutParams *params = static_cast<const AnchorLayoutParams *>(views_[i]->getLayoutParams());
            if (!params->is(LP_ANCHOR)) params = 0;
            if (params) {
                width = params->width;
                height = params->height;

                if (!params->center) {
                    if (params->left >= 0 && params->right >= 0) 	{
                        width = measuredWidth_ - params->left - params->right;
                    }
                    if (params->top >= 0 && params->bottom >= 0) 	{
                        height = measuredHeight_ - params->top - params->bottom;
                    }
                }
                specW = width < 0 ? MeasureSpec(UNSPECIFIED) : MeasureSpec(EXACTLY, width);
                specH = height < 0 ? MeasureSpec(UNSPECIFIED) : MeasureSpec(EXACTLY, height);
            }

            views_[i]->measure(dc, specW, specH);
        }
    }

    void AnchorLayout::layout() {
        for (size_t i = 0; i < views_.size(); i++) {
            const AnchorLayoutParams *params = static_cast<const AnchorLayoutParams *>(views_[i]->getLayoutParams());
            if (!params->is(LP_ANCHOR)) params = 0;

            Boundsf vBounds;
            vBounds.width = views_[i]->getMeasuredWidth();
            vBounds.height = views_[i]->getMeasuredHeight();

            // Clamp width/height to our own
            if (vBounds.width > bounds_.width) vBounds.width = bounds_.width;
            if (vBounds.height > bounds_.height) vBounds.height = bounds_.height;

            float left = 0, top = 0, right = 0, bottom = 0, center = false;
            if (params) {
                left = params->left;
                top = params->top;
                right = params->right;
                bottom = params->bottom;
                center = params->center;
            }

            if (left >= 0) {
                vBounds.left = bounds_.left + left;
                if (center)
                    vBounds.left -= vBounds.width * 0.5f;
            } else if (right >= 0) {
                vBounds.left = bounds_.bottom() - right - vBounds.width;
                if (center) {
                    vBounds.left += vBounds.width * 0.5f;
                }
            }

            if (top >= 0) {
                vBounds.top = bounds_.top + top;
                if (center)
                    vBounds.top -= vBounds.height * 0.5f;
            } else if (bottom >= 0) {
                vBounds.height = bounds_.bottom() - bottom - vBounds.height;
                if (center)
                    vBounds.top += vBounds.height * 0.5f;
            }

            views_[i]->setBounds(vBounds);
            views_[i]->layout();
        }
    }

    // TODO: This code needs some cleanup/restructuring...
    void LinearLayout::measure(const UIContext &dc, MeasureSpec horiz, MeasureSpec vert) {
        MeasureBySpec(layoutParams_->width, 0.0f, horiz, &measuredWidth_);
        MeasureBySpec(layoutParams_->height, 0.0f, vert, &measuredHeight_);

        if (views_.empty())
            return;

        float sum = 0.0f;
        float maxOther = 0.0f;
        float totalWeight = 0.0f;
        float weightSum = 0.0f;
        float weightZeroSum = 0.0f;

        int numVisible = 0;

        for (size_t i = 0; i < views_.size(); i++) {
            if (views_[i]->getVisibility() == V_GONE)
                continue;
            numVisible++;

            const LayoutParams *layoutParams = views_[i]->getLayoutParams();
            const LinearLayoutParams *linLayoutParams = static_cast<const LinearLayoutParams *>(layoutParams);
            if (!linLayoutParams->is(LP_LINEAR)) linLayoutParams = 0;

            Margins margins = defaultMargins_;

            if (linLayoutParams) {
                totalWeight += linLayoutParams->weight;
                if (linLayoutParams->hasMargins())
                    margins = linLayoutParams->margins;
            }

            if (orientation_ == ORIENT_HORIZONTAL) {
                MeasureSpec v = vert;
                if (v.type == UNSPECIFIED && measuredHeight_ != 0.0)
                    v = MeasureSpec(AT_MOST, measuredHeight_);
                views_[i]->measure(dc, MeasureSpec(UNSPECIFIED, measuredWidth_), v - (float)(margins.top + margins.bottom));
            } else if (orientation_ == ORIENT_VERTICAL) {
                MeasureSpec h = horiz;
                if (h.type == UNSPECIFIED && measuredWidth_ != 0) h = MeasureSpec(AT_MOST, measuredWidth_);
                views_[i]->measure(dc, h - (float)(margins.left + margins.right), MeasureSpec(UNSPECIFIED, measuredHeight_));
            }

            float amount;
            if (orientation_ == ORIENT_HORIZONTAL) {
                amount = views_[i]->getMeasuredWidth() + margins.left + margins.right;
                maxOther = MATH::MATH_MAX(maxOther, views_[i]->getMeasuredHeight() + margins.top + margins.bottom);
            } else {
                amount = views_[i]->getMeasuredHeight() + margins.top + margins.bottom;
                maxOther = MATH::MATH_MAX(maxOther, views_[i]->getMeasuredWidth() + margins.left + margins.right);
            }

            sum += amount;
            if (linLayoutParams) {
                if (linLayoutParams->weight == 0.0f)
                    weightZeroSum += amount;

                weightSum += linLayoutParams->weight;
            } else {
                weightZeroSum += amount;
            }
        }

        weightZeroSum += spacing_ * (numVisible - 1);

        // Alright, got the sum. Let's take the remaining space after the fixed-size views,
        // and distribute among the weighted ones.
        if (orientation_ == ORIENT_HORIZONTAL) {
            MeasureBySpec(layoutParams_->width, weightZeroSum, horiz, &measuredWidth_);
            MeasureBySpec(layoutParams_->height, maxOther, vert, &measuredHeight_);

            float unit = (measuredWidth_ - weightZeroSum) / weightSum;
            // Redistribute the stretchy ones! and remeasure the children!
            for (size_t i = 0; i < views_.size(); i++) {
                if (views_[i]->getVisibility() == V_GONE)
                    continue;
                const LayoutParams *layoutParams = views_[i]->getLayoutParams();
                const LinearLayoutParams *linLayoutParams = static_cast<const LinearLayoutParams *>(layoutParams);
                if (!linLayoutParams->is(LP_LINEAR)) linLayoutParams = 0;

                if (linLayoutParams && linLayoutParams->weight > 0.0f) {
                    Margins margins = defaultMargins_;
                    if (linLayoutParams->hasMargins())
                        margins = linLayoutParams->margins;
                    int marginSum = margins.left + margins.right;
                    MeasureSpec v = vert;
                    if (v.type == UNSPECIFIED && measuredHeight_ != 0.0)
                        v = MeasureSpec(AT_MOST, measuredHeight_);
                    views_[i]->measure(dc, MeasureSpec(EXACTLY, unit * linLayoutParams->weight - marginSum), v - (float)(margins.top + margins.bottom));
                }
            }
        } else {
            //MeasureBySpec(layoutParams_->height, vert.type == UNSPECIFIED ? sum : weightZeroSum, vert, &measuredHeight_);
            MeasureBySpec(layoutParams_->height, weightZeroSum, vert, &measuredHeight_);
            MeasureBySpec(layoutParams_->width, maxOther, horiz, &measuredWidth_);

            float unit = (measuredHeight_ - weightZeroSum) / weightSum;

            // Redistribute the stretchy ones! and remeasure the children!
            for (size_t i = 0; i < views_.size(); i++) {
                if (views_[i]->getVisibility() == V_GONE)
                    continue;
                const LayoutParams *layoutParams = views_[i]->getLayoutParams();
                const LinearLayoutParams *linLayoutParams = static_cast<const LinearLayoutParams *>(layoutParams);
                if (!linLayoutParams->is(LP_LINEAR)) linLayoutParams = 0;

                if (linLayoutParams && linLayoutParams->weight > 0.0f) {
                    Margins margins = defaultMargins_;
                    if (linLayoutParams->hasMargins())
                        margins = linLayoutParams->margins;
                    int marginSum = margins.top + margins.bottom;
                    MeasureSpec h = horiz;
                    if (h.type == UNSPECIFIED && measuredWidth_ != 0.0)
                        h = MeasureSpec(AT_MOST, measuredWidth_);
                    views_[i]->measure(dc, h - (float)(margins.left + margins.right), MeasureSpec(EXACTLY, unit * linLayoutParams->weight - marginSum));
                }
            }
        }
    }

    // weight != 0 = fill remaining space.
    void LinearLayout::layout() {
        const Boundsf &bounds = bounds_;

        Boundsf itemBounds;
        float pos;

        if (orientation_ == ORIENT_HORIZONTAL) {
            pos = bounds.left;
            itemBounds.top = bounds.top;
            itemBounds.height = measuredHeight_;
        } else {
            pos = bounds.height;
            itemBounds.left = bounds.left;
            itemBounds.width = measuredWidth_;
        }

        for (size_t i = 0; i < views_.size(); i++) {
            if (views_[i]->getVisibility() == V_GONE)
                continue;

            const LayoutParams *layoutParams = views_[i]->getLayoutParams();
            const LinearLayoutParams *linLayoutParams = static_cast<const LinearLayoutParams *>(layoutParams);
            if (!linLayoutParams->is(LP_LINEAR)) linLayoutParams = 0;

            Gravity gravity = G_TOPLEFT;
            Margins margins = defaultMargins_;
            if (linLayoutParams) {
                if (linLayoutParams->hasMargins())
                    margins = linLayoutParams->margins;
                gravity = linLayoutParams->gravity;
            }

            if (orientation_ == ORIENT_HORIZONTAL) {
                itemBounds.left = pos;
                itemBounds.width = views_[i]->getMeasuredWidth() + margins.left + margins.right;
            } else {
                itemBounds.top = pos;
                itemBounds.height = views_[i]->getMeasuredHeight() + margins.top + margins.bottom;
            }

            Boundsf innerBounds;
            ApplyGravity(itemBounds, margins,
                views_[i]->getMeasuredWidth(), views_[i]->getMeasuredHeight(),
                gravity, innerBounds);

            views_[i]->setBounds(innerBounds);
            views_[i]->layout();

            pos += spacing_ + (orientation_ == ORIENT_HORIZONTAL ? itemBounds.width : itemBounds.height);
        }
    }
}

