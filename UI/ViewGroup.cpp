#include "ViewGroup.h"
using _INPUT::AxisInput;
using _INPUT::KeyInput;
using _INPUT::TouchInput;
using _INPUT::InputState;

namespace UI
{
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
            dc.fillRect(UI::Drawable(0x60000000), dc.getBounds());
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

