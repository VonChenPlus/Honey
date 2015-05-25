#include "TextEdit.h"
#include "MATH/Bounds.h"
using MATH::Bounds;
#include "SGF/UI/KeyCodes.h"
#include "SGF/GFX/DrawBuffer.h"
using GFX::ALIGN_CENTER;
using GFX::ALIGN_VCENTER;
using GFX::ALIGN_LEFT;
#include "UTILS/TEXT/UTF8.h"
using UTILS::TEXT::u8_dec;
using UTILS::TEXT::u8_inc;
using UTILS::TEXT::u8_wc_toutf8;

namespace UI
{
    extern void SetFocusedView(View *view, bool force = false);

    TextEdit::TextEdit(const std::string &text, const std::string &placeholderText, LayoutParams *layoutParams)
      : View(layoutParams), text_(text), undo_(text), placeholderText_(placeholderText), maxLen_(255), ctrlDown_(false) {
        caret_ = (int)text_.size();
    }

    void TextEdit::draw(UIContext &dc) {
        dc.pushScissor(bounds_);
        dc.setFontStyle(dc.theme->uiFont);
        dc.fillRect(hasFocus() ? UI::Drawable(0x80000000) : UI::Drawable(0x30000000), bounds_);

        float textX = bounds_.x;
        float w, h;

        Bounds textBounds = bounds_;
        textBounds.x = textX;

        if (text_.empty()) {
            if (placeholderText_.size()) {
                dc.drawTextRect(placeholderText_.c_str(), bounds_, 0x50FFFFFF, ALIGN_CENTER);
            }
        }
        else {
            dc.drawTextRect(text_.c_str(), textBounds, 0xFFFFFFFF, ALIGN_VCENTER | ALIGN_LEFT);
        }

        if (hasFocus()) {
            // Hack to find the caret position. Might want to find a better way...
            dc.measureTextCount(dc.theme->uiFont, text_.c_str(), caret_, &w, &h, ALIGN_VCENTER | ALIGN_LEFT);
            float caretX = w;
            caretX += textX;

            if (caretX > bounds_.w) {
                // Scroll text to the left if the caret won't fit. Not ideal but looks better than not scrolling it.
                textX -= caretX - bounds_.w;
            }
            dc.fillRect(UI::Drawable(0xFFFFFFFF), Bounds(caretX - 1, bounds_.y + 2, 3, bounds_.h - 4));
        }
        dc.popScissor();
    }

    void TextEdit::getContentDimensions(const UIContext &dc, float &w, float &h) const {
        dc.measureText(dc.theme->uiFont, text_.size() ? text_.c_str() : "Wj", &w, &h);
        w += 2;
        h += 2;
    }

    // Handles both windows and unix line endings.
    static std::string FirstLine(const std::string &text) {
        size_t pos = text.find("\r\n");
        if (pos != std::string::npos) {
            return text.substr(0, pos);
        }
        pos = text.find('\n');
        if (pos != std::string::npos) {
            return text.substr(0, pos);
        }
        return text;
    }

    void TextEdit::touch(const TouchInput &touch) {
        if (touch.flags & TOUCH_DOWN) {
            if (bounds_.contains(touch.x, touch.y)) {
                SetFocusedView(this, true);
            }
        }
    }

    bool TextEdit::key(const KeyInput &input) {
        if (!hasFocus())
            return false;
        bool textChanged = false;
        // Process navigation keys. These aren't chars.
        if (input.flags & KEY_DOWN) {
            switch (input.keyCode) {
            case NKCODE_CTRL_LEFT:
            case NKCODE_CTRL_RIGHT:
                ctrlDown_ = true;
                break;
            case NKCODE_DPAD_LEFT:  // ASCII left arrow
                u8_dec(text_.c_str(), &caret_);
                break;
            case NKCODE_DPAD_RIGHT: // ASCII right arrow
                u8_inc(text_.c_str(), &caret_);
                break;
            case NKCODE_MOVE_HOME:
            case NKCODE_PAGE_UP:
                caret_ = 0;
                break;
            case NKCODE_MOVE_END:
            case NKCODE_PAGE_DOWN:
                caret_ = (int)text_.size();
                break;
            case NKCODE_FORWARD_DEL:
                if (caret_ < (int)text_.size()) {
                    int endCaret = caret_;
                    u8_inc(text_.c_str(), &endCaret);
                    undo_ = text_;
                    text_.erase(text_.begin() + caret_, text_.begin() + endCaret);
                    textChanged = true;
                }
                break;
            case NKCODE_DEL:
                if (caret_ > 0) {
                    int begCaret = caret_;
                    u8_dec(text_.c_str(), &begCaret);
                    undo_ = text_;
                    text_.erase(text_.begin() + begCaret, text_.begin() + caret_);
                    caret_--;
                    textChanged = true;
                }
                break;
            case NKCODE_ENTER:
                {
                    EventParams e;
                    e.s = text_;
                    OnEnter.trigger(e);
                    break;
                }
            case NKCODE_BACK:
            case NKCODE_ESCAPE:
                return false;
            }

            if (ctrlDown_) {
                switch (input.keyCode) {
                case NKCODE_C:
                    // Just copy the entire text contents, until we get selection support.
                    // TODO
                    break;
                case NKCODE_V:
                    {
                        // TODO
                        std::string clipText = "";
                        clipText = FirstLine(clipText);
                        if (clipText.size()) {
                            // Until we get selection, replace the whole text
                            undo_ = text_;
                            text_.clear();
                            caret_ = 0;

                            size_t maxPaste = maxLen_ - text_.size();
                            if (clipText.size() > maxPaste) {
                                int end = 0;
                                while ((size_t)end < maxPaste) {
                                    u8_inc(clipText.c_str(), &end);
                                }
                                if (end > 0) {
                                    u8_dec(clipText.c_str(), &end);
                                }
                                clipText = clipText.substr(0, end);
                            }
                            insertAtCaret(clipText.c_str());
                            textChanged = true;
                        }
                    }
                    break;
                case NKCODE_Z:
                    text_ = undo_;
                    break;
                }
            }

            if (caret_ < 0) {
                caret_ = 0;
            }
            if (caret_ > (int)text_.size()) {
                caret_ = (int)text_.size();
            }
        }

        if (input.flags & KEY_UP) {
            switch (input.keyCode) {
            case NKCODE_CTRL_LEFT:
            case NKCODE_CTRL_RIGHT:
                ctrlDown_ = false;
                break;
            }
        }

        // Process chars.
        if (input.flags & KEY_CHAR) {
            int unichar = input.keyCode;
            if (unichar >= 0x20 && !ctrlDown_) {  // Ignore control characters.
                // Insert it! (todo: do it with a string insert)
                char buf[8];
                buf[u8_wc_toutf8(buf, unichar)] = '\0';
                if (strlen(buf) + text_.size() < maxLen_) {
                    undo_ = text_;
                    insertAtCaret(buf);
                    textChanged = true;
                }
            }
        }

        if (textChanged) {
            UI::EventParams e;
            e.v = this;
            OnTextChange.trigger(e);
        }
        return true;
    }

    void TextEdit::insertAtCaret(const char *text) {
        size_t len = strlen(text);
        for (size_t i = 0; i < len; i++) {
            text_.insert(text_.begin() + caret_, text[i]);
            caret_++;
        }
    }
}
