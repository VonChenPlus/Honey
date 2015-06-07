#ifndef NTHREAD_H
#define NTHREAD_H

#include <functional>

#include "THREAD/NThreadDef.h"
#include "THREAD/NThreadUtils.h"

// partial std::thread implementation for win32/pthread

namespace THREAD
{
    class ThreadID final
    {
        friend class NThread;
    public:
        ThreadID();
        ThreadID(THREAD_ID threadid);

        bool operator==(const ThreadID& rhs) const;
        bool operator!=(const ThreadID& rhs) const;
        bool operator<(const ThreadID& rhs) const;

    private:
        THREAD_ID thread_;
    };

    template <typename FuncName>
    class TheadFunc final
    {
        friend class NThread;
    private:
        TheadFunc(FuncName&& func) : func_(func) {}

        void run() { func_(); }

    private:
        FuncName func_;
    };

    template <typename Callback>
    class ThreadFuncWithArgs final
    {
        friend class NThread;
    private:
        ThreadFuncWithArgs(Callback callback) : callback_(callback) {
        }

        void run() { callback_();  }

    private:
        Callback callback_;
    };

    class NThread final
    {
    public:
        template <typename FuncName>
        NThread(FuncName&& func) {
            startThread(new TheadFunc<FuncName>(func));
        }

        template <typename FuncName, typename... Args>
        NThread(FuncName&& func, Args&&... args) {
            startThread(CreateCallback(std::bind(func, args...)));
        }

        ~NThread() {
            if (joinable())
                detach();
        }

        ThreadID id() { return id_; }
        THREAD_HANDLE handle() {
        #ifdef _WIN32
            return handle_;
        #else
            return id_.thread_;
        #endif
        }

        bool joinable() const;
        void join();
        void detach();
        void swap(NThread &other);

    private:
        template <typename FuncObject>
        void startThread(FuncObject* funcObj)
        {
    #ifdef USE_BEGINTHREADEX
            handle_ = (HANDLE)_beginthreadex(NULLPTR, 0, &RunAndDelete<FuncObject>, funcObj, 0, &id_.thread_);
    #elif defined(_WIN32)
            handle_ = CreateThread(NULLPTR, 0, &RunAndDelete<FuncObject>, funcObj, 0, &id_.thread_);
    #else
            pthread_attr_t attr;
            pthread_attr_init(&attr);
            pthread_attr_setstacksize(&attr, 1024 * 1024);
            if (pthread_create(&id_.thread_, &attr, &RunAndDelete<FuncObject>, funcObj))
                id_ = ThreadID();
    #endif
        }

        template <typename FuncName>
        static THREAD_RETURN RunAndDelete(void* funcObj)
        {
            static_cast<FuncName *>(funcObj)->run();
            delete static_cast<FuncName *>(funcObj);
            return 0;
        }

        template <typename Callback>
        static ThreadFuncWithArgs<Callback> *CreateCallback(Callback&& callback) {
            return new ThreadFuncWithArgs<Callback>(callback);
        }

    private:
        ThreadID id_;

        #ifdef _WIN32
            THREAD_HANDLE handle_;
        #endif
    };
}

#endif // NTHREAD_H
