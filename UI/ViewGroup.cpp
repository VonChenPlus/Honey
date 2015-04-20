#include "ViewGroup.h"

namespace UI
{
    extern float GetDirectionScore(View *origin, View *destination, FocusDirection direction);

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
                bounds_.x - dropsize, bounds_.y,
                bounds_.x2() + dropsize, bounds_.y2()+dropsize*1.5, 0xDF000000, 3.0f);
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
}

