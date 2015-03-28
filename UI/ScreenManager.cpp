#include "BASE/BasicTypes.h"
#include "UI/ScreenManager.h"
#include "UI/Screen.h"

namespace UI
{
    ScreenManager::ScreenManager()
    {
        nextScreen_ = NULLPTR;
    }

    ScreenManager::~ScreenManager()
    {
        shutdown();
    }

    void ScreenManager::switchScreen(Screen *screen)
    {
        if (screen == nextScreen_)
        {
            //ELOG("Already switching to this screen");
            return;
        }
        // Note that if a dialog is found, this will be a silent background switch that
        // will only become apparent if the dialog is closed. The previous screen will stick around
        // until that switch.
        // TODO: is this still true?
        if (nextScreen_ != NULLPTR)
        {
            //FLOG("Already had a nextScreen_");
        }
        if (screen == NULLPTR)
        {
            //WLOG("Swiching to a zero screen, this can't be good");
        }
        if (stack_.empty() || screen != stack_.back().screen)
        {
            nextScreen_ = screen;
            nextScreen_->setScreenManager(this);
        }
    }

    void ScreenManager::update()
    {
        if (nextScreen_)
        {
            switchToNext();
        }

        if (stack_.size())
        {
            stack_.back().screen->update();
        }
    }

    void ScreenManager::resized()
    {
        // Have to notify the whole stack, otherwise there will be problems when going back
        // to non-top screens.
        for (const auto &iter : stack_ )
            iter.screen->resized();
    }

    void ScreenManager::render()
    {
        if (!stack_.empty())
        {
            switch (stack_.back().flags)
            {
            case LAYER_SIDEMENU:
            case LAYER_TRANSPARENT:
                if (stack_.size() == 1)
                {
                    //ELOG("Can't have sidemenu over nothing");
                    break;
                }
                else
                {
                    auto iter = stack_.end();
                    iter-=2;
                    Layer backback = *iter;
                    // Also shift to the right somehow...
                    backback.screen->render();
                    stack_.back().screen->render();
                    break;
                }
            default:
                stack_.back().screen->render();
                break;
            }
        }
        else
        {
            //ELOG("No current screen!");
        }
    }

    void ScreenManager::sendMessage(const char *msg, const char *value)
    {
        if (!stack_.empty())
            stack_.back().screen->sendMessage(msg, value);
    }

    void ScreenManager::shutdown()
    {
        for (const auto &iter : stack_)
            delete iter.screen;
        stack_.clear();
        delete nextScreen_;
        nextScreen_ = 0;
    }

    bool ScreenManager::touch(const TouchInput &touch)
    {
        if (!stack_.empty())
        {
            return stack_.back().screen->touch(touch);
        }

        return false;
    }

    bool ScreenManager::key(const KeyInput &key)
    {
        if (!stack_.empty())
        {
            return stack_.back().screen->key(key);
        }

        return false;
    }

    bool ScreenManager::axis(const AxisInput &axis)
    {
        if (!stack_.empty())
        {
            return stack_.back().screen->axis(axis);
        }

        return false;
    }

    void ScreenManager::push(Screen *screen, LAYER_FLAG layerFlags)
    {
        if (nextScreen_ && stack_.empty())
        {
            // we're during init, this is OK
            switchToNext();
        }
        screen->setScreenManager(this);
        if (screen->isTransparent())
        {
            layerFlags = LAYER_TRANSPARENT;
        }

        Layer layer = {screen, layerFlags};
        stack_.push_back(layer);
    }

    void ScreenManager::pop()
    {
        if (stack_.size())
        {
            delete stack_.back().screen;
            stack_.pop_back();
        }
        else
        {
            //ELOG("Can't pop when stack empty");
        }
    }


    void ScreenManager::switchToNext()
    {
        if (!nextScreen_)
        {
            //ELOG("switchToNext: No nextScreen_!");
        }

        Layer temp = {0, LAYER_DEFAULT};
        if (!stack_.empty())
        {
            temp = stack_.back();
            stack_.pop_back();
        }
        Layer newLayer = {nextScreen_, LAYER_DEFAULT};
        stack_.push_back(newLayer);
        if (temp.screen)
        {
            delete temp.screen;
        }
        nextScreen_ = 0;
    }
}
