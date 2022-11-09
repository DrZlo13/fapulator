#include <core/event_flag.h>
#include <mutex>

class EventFlagInstance {
private:
    mutable std::mutex mutex;
    std::condition_variable notifier;
    uint32_t flags = 0;

public:
    EventFlagInstance() {
    }

    ~EventFlagInstance() {
        reset();
    }

    FuriStatus set(uint32_t flags) {
        {
            std::unique_lock<std::mutex> lock(mutex);
            this->flags |= flags;
        }
        notifier.notify_all();
        return FuriStatusOk;
    }

    uint32_t clear(uint32_t flags) {
        uint32_t flags_before = 0;
        {
            std::unique_lock<std::mutex> lock(mutex);
            flags_before = this->flags;
            this->flags &= ~flags;
        }
        notifier.notify_all();
        return flags_before;
    }

    FuriStatus wait(
        uint32_t flags,
        uint32_t* flags_out,
        uint32_t timeout,
        bool clear_on_exit,
        bool wait_all) {
        std::unique_lock<std::mutex> lock(mutex);
        if(wait_all) {
            while((this->flags & flags) != flags) {
                if(timeout == 0) {
                    return FuriStatusErrorTimeout;
                }
                if(timeout == FuriWaitForever) {
                    notifier.wait(lock);
                } else {
                    if(notifier.wait_for(lock, std::chrono::milliseconds(timeout)) ==
                       std::cv_status::timeout) {
                        return FuriStatusErrorTimeout;
                    }
                }
            }
        } else {
            while((this->flags & flags) == 0) {
                if(timeout == 0) {
                    return FuriStatusErrorTimeout;
                }
                if(timeout == FuriWaitForever) {
                    notifier.wait(lock);
                } else {
                    if(notifier.wait_for(lock, std::chrono::milliseconds(timeout)) ==
                       std::cv_status::timeout) {
                        return FuriStatusErrorTimeout;
                    }
                }
            }
        }

        *flags_out = this->flags;
        if(clear_on_exit) {
            this->flags &= ~flags;
        }
        return FuriStatusOk;
    }

    uint32_t get() {
        std::unique_lock<std::mutex> lock(mutex);
        return flags;
    }

    FuriStatus reset() {
        {
            std::unique_lock<std::mutex> lock(mutex);
            flags = 0;
        }
        notifier.notify_all();
        return FuriStatusOk;
    }
};

FuriEventFlag* furi_event_flag_alloc() {
    EventFlagInstance* instance = new EventFlagInstance();
    return reinterpret_cast<FuriEventFlag*>(instance);
}

void furi_event_flag_free(FuriEventFlag* event_flag) {
    EventFlagInstance* instance = reinterpret_cast<EventFlagInstance*>(event_flag);
    delete instance;
}

uint32_t furi_event_flag_set(FuriEventFlag* event_flag, uint32_t flags) {
    EventFlagInstance* instance = reinterpret_cast<EventFlagInstance*>(event_flag);
    return instance->set(flags);
}

uint32_t furi_event_flag_clear(FuriEventFlag* event_flag, uint32_t flags) {
    EventFlagInstance* instance = reinterpret_cast<EventFlagInstance*>(event_flag);
    return instance->clear(flags);
}

uint32_t furi_event_flag_get(FuriEventFlag* event_flag) {
    EventFlagInstance* instance = reinterpret_cast<EventFlagInstance*>(event_flag);
    return instance->get();
}

uint32_t furi_event_flag_wait(
    FuriEventFlag* event_flag,
    uint32_t flags,
    uint32_t options,
    uint32_t timeout) {
    EventFlagInstance* instance = reinterpret_cast<EventFlagInstance*>(event_flag);
    uint32_t flags_out = 0;
    bool wait_all = false;
    bool clear_on_exit = true;

    if(options & FuriFlagWaitAll) {
        wait_all = true;
    }

    if(options & FuriFlagNoClear) {
        clear_on_exit = false;
    }

    // TODO: return raw flags and move options logic to instance
    FuriStatus status = instance->wait(flags, &flags_out, timeout, clear_on_exit, wait_all);

    if(wait_all) {
        if((flags & flags_out) != flags) {
            if(timeout > 0U) {
                flags_out = (uint32_t)FuriStatusErrorTimeout;
            } else {
                flags_out = (uint32_t)FuriStatusErrorResource;
            }
        }
    } else {
        if((flags & flags_out) == 0U) {
            if(timeout > 0U) {
                flags_out = (uint32_t)FuriStatusErrorTimeout;
            } else {
                flags_out = (uint32_t)FuriStatusErrorResource;
            }
        }
    }

    return flags_out;
}