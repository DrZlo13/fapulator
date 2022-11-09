#include <core/timer.h>
#include <thread>

class TimerInstance {
private:
    std::thread thread;
    bool continued;
    FuriTimerCallback callback;
    void* context;
    uint32_t ticks;

    void run() {
        do {
            std::this_thread::sleep_for(std::chrono::milliseconds(ticks));
            callback(context);
        } while(continued);
    }

public:
    TimerInstance(FuriTimerCallback callback, void* context)
        : callback(callback)
        , context(context) {
        continued = true;
    }

    ~TimerInstance() {
        continued = false;
        thread.join();
    }

    void start(uint32_t ticks) {
        this->ticks = ticks;
        thread = std::thread(&TimerInstance::run, this);
    }

    void stop() {
        continued = false;
        thread.join();
    }

    bool is_running() {
        return continued;
    }
};

FuriTimer* furi_timer_alloc(FuriTimerCallback func, FuriTimerType type, void* context) {
    return new TimerInstance(func, context);
}

void furi_timer_free(FuriTimer* instance) {
    TimerInstance* timer = (TimerInstance*)instance;
    delete timer;
}

FuriStatus furi_timer_start(FuriTimer* instance, uint32_t ticks) {
    TimerInstance* timer = (TimerInstance*)instance;
    timer->start(ticks);
    return FuriStatusOk;
}

FuriStatus furi_timer_stop(FuriTimer* instance) {
    TimerInstance* timer = (TimerInstance*)instance;
    timer->stop();
    return FuriStatusOk;
}

uint32_t furi_timer_is_running(FuriTimer* instance) {
    TimerInstance* timer = (TimerInstance*)instance;
    return timer->is_running();
}