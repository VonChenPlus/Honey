#include "NThread.h"

namespace THREAD
{
    ThreadID::ThreadID()
        : thread_(0) {
    }

    ThreadID::ThreadID(THREAD_ID threadid)
        : thread_(threadid) {
    }

    bool ThreadID::operator==(const ThreadID& rhs) const {
        return thread_ == rhs.thread_;
    }

    bool ThreadID::operator!=(const ThreadID& rhs) const {
        return !(*this == rhs);
    }

    bool ThreadID::operator<(const ThreadID& rhs) const {
        return thread_ < rhs.thread_;
    }

    bool NThread::joinable() const {
        return id_ == ThreadID();
    }

    void NThread::join() {
    #ifdef _WIN32
        WaitForSingleObject(handle_, INFINITE);
    #else
        pthread_join(id_.thread_, NULL);
    #endif
        detach();
    }

    void NThread::detach() {
    #ifdef _WIN32
        CloseHandle(handle_);
    #else
        pthread_detach(id_.thread_);
    #endif
        id_ = ThreadID();
    }

    void NThread::swap(NThread &other) {
        std::swap(id_, other.id_);
    #ifdef _WIN32
        std::swap(handle_, other.handle_);
    #endif
    }
}
