#ifndef TEXTEDIT_H
#define TEXTEDIT_H

#include "GRAPH/UI/View.h"

namespace UI
{
    class TextEdit : public View
    {
    public:
        TextEdit(const std::string &text, const std::string &placeholderText, LayoutParams *layoutParams = 0);
        void setText(const std::string &text) { text_ = text; caret_ = (int)text_.size(); }
        const std::string &getText() const { return text_; }
        void setMaxLen(size_t maxLen) { maxLen_ = maxLen; }

        void getContentDimensions(const UIContext &dc, float &w, float &h) const override;
        void draw(UIContext &dc) override;
        bool key(const KeyInput &key) override;
        void touch(const TouchInput &touch) override;

        Event OnTextChange;
        Event OnEnter;

    private:
        void insertAtCaret(const char *text);

        std::string text_;
        std::string undo_;
        std::string placeholderText_;
        int caret_;
        size_t maxLen_;
        bool ctrlDown_;  // TODO: Make some global mechanism for this.
        // TODO: Selections
    };
}

#endif // TEXTEDIT_H
