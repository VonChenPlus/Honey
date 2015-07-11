#include "PopScreen.h"
#include "GRAPH/UI/KeyCodes.h"
#include "GRAPH/UI/ScreenManager.h"
#include "GRAPH/UI/CONTROLS/PopupHeader.h"

namespace UI
{
    PopupScreen::PopupScreen(std::string title, std::string button1, std::string button2)
        : box_(0), defaultButton_(NULL) , title_(title) {
        if (!button1.empty())
            button1_ = "OK";
        if (!button2.empty())
            button2_ = "Cancel";
    }

    bool PopupScreen::touch(const TouchInput &touch) {
        if (!box_ || (touch.flags & TOUCH_DOWN) == 0 || touch.id != 0) {
            return UIDialogScreen::touch(touch);
        }

        return UIDialogScreen::touch(touch);
    }

    bool PopupScreen::key(const KeyInput &key) {
        if (key.flags & KEY_DOWN) {
            if (key.keyCode == NKCODE_ENTER && defaultButton_) {
                UI::EventParams e;
                defaultButton_->OnClick.trigger(e);
                return true;
            }
        }

        return UIDialogScreen::key(key);
    }

    void PopupScreen::createViews() {
        using namespace UI;

        UIContext &dc = *screenManager()->getUIContext();

        root_ = new AnchorLayout(new LayoutParams(FILL_PARENT, FILL_PARENT));

        float yres = screenManager()->getUIContext()->getBounds().height;

        box_ = new LinearLayout(ORIENT_VERTICAL,
            new AnchorLayoutParams(550.0, fillVertical() ? yres - 30.0 : (float)WRAP_CONTENT, dc.getBounds().centerX(), dc.getBounds().centerY(), NONE, NONE, true));

        root_->add(box_);
        box_->setBG(UI::Drawable(0xFF303030));
        box_->setHasDropShadow(true);

        View *title = new PopupHeader(title_);
        box_->add(title);

        createPopupContents(box_);
        root_->setDefaultFocusView(box_);

        if (showButtons() && !button1_.empty()) {
            // And the two buttons at the bottom.
            LinearLayout *buttonRow = new LinearLayout(ORIENT_HORIZONTAL, new LinearLayoutParams(200.0, WRAP_CONTENT));
            buttonRow->setSpacing(0);

            // Adjust button order to the platform default.
            if (!button2_.empty())
                buttonRow->add(new Button(button2_, new LinearLayoutParams(1.0f)))->OnClick.handle(this, &PopupScreen::OnCancel);
            defaultButton_ = buttonRow->add(new Button(button1_, new LinearLayoutParams(1.0f)));
            defaultButton_->OnClick.handle(this, &PopupScreen::OnOK);

            box_->add(buttonRow);
        }
    }

    EventReturn PopupScreen::OnOK(EventParams &) {
        if (onCompleted(DR_OK))
            screenManager()->finishDialog(this, DR_OK);
        return UI::EVENT_DONE;
    }

    EventReturn PopupScreen::OnCancel(EventParams &) {
        if (onCompleted(DR_CANCEL))
            screenManager()->finishDialog(this, DR_CANCEL);
        return UI::EVENT_DONE;
    }
}
