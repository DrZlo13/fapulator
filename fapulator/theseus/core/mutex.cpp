#include <core/mutex.h>
#include <mutex>

FuriMutex* furi_mutex_alloc(FuriMutexType type) {
    return (FuriMutex*)new std::timed_mutex();
}

void furi_mutex_free(FuriMutex* instance) {
    delete(std::timed_mutex*)instance;
}

FuriStatus furi_mutex_acquire(FuriMutex* instance, uint32_t timeout) {
    std::timed_mutex* mutex = (std::timed_mutex*)instance;
    if(timeout == FuriWaitForever) {
        mutex->lock();
        return FuriStatusOk;
    } else {
        if(mutex->try_lock_for(std::chrono::milliseconds(timeout))) {
            return FuriStatusOk;
        } else {
            return FuriStatusErrorTimeout;
        }
    }
}

FuriStatus furi_mutex_release(FuriMutex* instance) {
    std::timed_mutex* mutex = (std::timed_mutex*)instance;
    mutex->unlock();
    return FuriStatusOk;
}