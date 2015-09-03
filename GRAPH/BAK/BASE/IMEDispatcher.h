#ifndef IMEDISPATCHER_H
#define IMEDISPATCHER_H

#include "MATH/Rectangle.h"

namespace GRAPH
{
    typedef struct
    {
        MATH::Rectf  begin;              // the soft keyboard rectangle when animation begins
        MATH::Rectf  end;                // the soft keyboard rectangle when animation ends
        float     duration;           // the soft keyboard animation duration
    } IMEKeyboardNotificationInfo;


    class IMEDelegate
    {
    public:
        virtual ~IMEDelegate();

        virtual bool attachWithIME();
        virtual bool detachWithIME();

    protected:
        friend class IMEDispatcher;

        virtual bool canAttachWithIME() { return false; }
        virtual void didAttachWithIME() {}
        virtual bool canDetachWithIME() { return false; }
        virtual void didDetachWithIME() {}

        virtual void insertText(const char * text, size_t len) {}
        virtual void deleteBackward() {}

        virtual const std::string& getContentText() { return std::string(); }

        virtual void keyboardWillShow(IMEKeyboardNotificationInfo& info)   {}
        virtual void keyboardDidShow(IMEKeyboardNotificationInfo& info)    {}
        virtual void keyboardWillHide(IMEKeyboardNotificationInfo& info)   {}
        virtual void keyboardDidHide(IMEKeyboardNotificationInfo& info)    {}

    protected:
        IMEDelegate();
    };

    class IMEDispatcher
    {
    public:
        ~IMEDispatcher();
        static IMEDispatcher* sharedDispatcher();

        void dispatchInsertText(const char * text, size_t len);
        void dispatchDeleteBackward();

        const std::string& getContentText();

        void dispatchKeyboardWillShow(IMEKeyboardNotificationInfo& info);
        void dispatchKeyboardDidShow(IMEKeyboardNotificationInfo& info);
        void dispatchKeyboardWillHide(IMEKeyboardNotificationInfo& info);
        void dispatchKeyboardDidHide(IMEKeyboardNotificationInfo& info);

    protected:
        friend class IMEDelegate;

        void addDelegate(IMEDelegate * delegate);
        bool attachDelegateWithIME(IMEDelegate * delegate);
        bool detachDelegateWithIME(IMEDelegate * delegate);
        void removeDelegate(IMEDelegate * delegate);

    private:
        IMEDispatcher();

        class Impl;
        Impl * _impl;
    };
}

#endif // IMEDISPATCHER_H
