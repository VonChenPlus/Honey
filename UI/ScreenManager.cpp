#include "BASE/BasicTypes.h"
#include "UI/ScreenManager.h"
#include "UI/Screen.h"
#include "INPUT/InputState.h"
using _INPUT::AxisInput;
using _INPUT::KeyInput;
using _INPUT::TouchInput;
#include "UI/View.h"

namespace UI
{
    extern void SetFocusedView(View *view, bool force);
    extern void UIDisableBegin();
    extern void UIDisableEnd();

    ScreenManager::ScreenManager() {
        nextScreen_ = NULLPTR;
    }

    ScreenManager::~ScreenManager() {
        shutdown();
    }

    void ScreenManager::switchScreen(Screen *screen) {
        if (screen == nextScreen_) {
            throw _NException_Normal("Already switching to this screen");
        }
        // Note that if a dialog is found, this will be a silent background switch that
        // will only become apparent if the dialog is closed. The previous screen will stick around
        // until that switch.
        // TODO: is this still true?
        if (nextScreen_ != NULLPTR) {
            //FLOG("Already had a nextScreen_");
        }
        if (screen == NULLPTR) {
            //WLOG("Swiching to a zero screen, this can't be good");
        }
        if (stack_.empty() || screen != stack_.back().screen) {
            nextScreen_ = screen;
            nextScreen_->setScreenManager(this);
        }
    }

    void ScreenManager::update(_INPUT::InputState &input) {
        if (nextScreen_) {
            switchToNext();
        }

        if (stack_.size()) {
            stack_.back().screen->update(input);
        }
    }

    void ScreenManager::resized() {
        // Have to notify the whole stack, otherwise there will be problems when going back
        // to non-top screens.
        for (const auto &iter : stack_ )
            iter.screen->resized();
    }

    void ScreenManager::render() {
        if (!stack_.empty()) {
            switch (stack_.back().flags) {
            case LAYER_SIDEMENU:
            case LAYER_TRANSPARENT:
                if (stack_.size() == 1) {
                    throw _NException_Normal("Can't have sidemenu over nothing");
                }
                else {
                    auto iter = stack_.end();
                    iter-=2;
                    Layer backback = *iter;
                    UIDisableBegin();
                    // Also shift to the right somehow...
                    backback.screen->render();
                    UIDisableEnd();
                    stack_.back().screen->render();
                    break;
                }
            default:
                stack_.back().screen->render();
                break;
            }
        }
        else {
            throw _NException_Normal("No current screen");
        }

        processFinishDialog();
    }

    void ScreenManager::sendMessage(const char *msg, const char *value) {
        if (!strcmp(msg, "recreateviews"))
            recreateAllViews();
        if (!stack_.empty())
            stack_.back().screen->sendMessage(msg, value);
    }

    void ScreenManager::shutdown() {
        for (const auto &iter : stack_)
            delete iter.screen;
        stack_.clear();
        delete nextScreen_;
        nextScreen_ = 0;
    }

    void ScreenManager::recreateAllViews() {
        for (auto it = stack_.begin(); it != stack_.end(); ++it) {
            it->screen->recreateViews();
        }
    }

    void ScreenManager::finishDialog(Screen *dialog, DialogResult result) {
        if (stack_.empty()) {
            throw _NException_Normal("Must be in a dialog to finishDialog");
        }
        if (dialog != stack_.back().screen) {
            throw _NException_Normal("Wrong dialog being finished!");
        }
        dialog->onFinish(result);
        dialogFinished_ = dialog;
        dialogResult_ = result;
    }

    bool ScreenManager::touch(const TouchInput &touch) {
        if (!stack_.empty())
        {
            return stack_.back().screen->touch(touch);
        }

        return false;
    }

    bool ScreenManager::key(const KeyInput &key) {
        if (!stack_.empty()) {
            return stack_.back().screen->key(key);
        }

        return false;
    }

    bool ScreenManager::axis(const AxisInput &axis) {
        if (!stack_.empty()) {
            return stack_.back().screen->axis(axis);
        }

        return false;
    }

    void ScreenManager::push(Screen *screen, LAYER_FLAG layerFlags) {
        if (nextScreen_ && stack_.empty()) {
            // we're during init, this is OK
            switchToNext();
        }
        screen->setScreenManager(this);
        if (screen->isTransparent()) {
            layerFlags = LAYER_TRANSPARENT;
        }

        SetFocusedView(0, false);
        Layer layer = {screen, layerFlags};
        stack_.push_back(layer);
    }

    void ScreenManager::pop() {
        if (stack_.size()) {
            delete stack_.back().screen;
            stack_.pop_back();
        }
        else {
            throw _NException_Normal("Can't pop when stack empty");
        }
    }

    void ScreenManager::switchToNext() {
        if (!nextScreen_) {
            throw _NException_Normal("switchToNext: No nextScreen_");
        }

        Layer temp = {0, LAYER_DEFAULT};
        if (!stack_.empty()) {
            temp = stack_.back();
            stack_.pop_back();
        }
        Layer newLayer = {nextScreen_, LAYER_DEFAULT};
        stack_.push_back(newLayer);
        if (temp.screen) {
            delete temp.screen;
        }
        nextScreen_ = 0;
        SetFocusedView(0, false);
    }

    void ScreenManager::processFinishDialog() {
        if (dialogFinished_) {
            // Another dialog may have been pushed before the render, so search for it.
            Screen *caller = 0;
            for (size_t i = 0; i < stack_.size(); ++i) {
                if (stack_[i].screen != dialogFinished_) {
                    continue;
                }

                stack_.erase(stack_.begin() + i);
                // The previous screen was the caller (not necessarily the topmost.)
                if (i > 0) {
                    caller = stack_[i - 1].screen;
                }
            }

            if (!caller) {
                throw _NException_Normal("ERROR: no top screen when finishing dialog");
            } else if (caller != topScreen()) {
                // The caller may get confused if we call dialogFinished() now.
                //WLOG("Skipping non-top dialog when finishing dialog.");
            } else {
                caller->dialogFinished(dialogFinished_, dialogResult_);
            }
            delete dialogFinished_;
            dialogFinished_ = 0;
        }
    }
}
