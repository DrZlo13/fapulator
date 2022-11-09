#include <core/thread.h>
#include <core/event_flag.h>
#include <core/log.h>
#include <thread>
#include <map>
#include <mutex>

static std::mutex thread_map_mutex;
static std::map<std::thread::id, FuriThread*> thread_map;

void thread_map_push(std::thread::id id, FuriThread* thread) {
    std::lock_guard<std::mutex> lock(thread_map_mutex);
    thread_map[id] = thread;
}

FuriThread* thread_map_get(std::thread::id id) {
    std::lock_guard<std::mutex> lock(thread_map_mutex);
    return thread_map[id];
}

void thread_map_erase(std::thread::id id) {
    std::lock_guard<std::mutex> lock(thread_map_mutex);
    thread_map.erase(id);
}

class ThreadInstance {
private:
    std::thread thread;
    std::string name;
    FuriThreadCallback callback;
    FuriEventFlag* event_flag;
    size_t stack_size;
    FuriThread* thread_ptr;

    static void furi_thread_body(void* context) {
        ThreadInstance* instance = (ThreadInstance*)context;
        thread_map_push(std::this_thread::get_id(), instance->thread_ptr);

        instance->callback(NULL);
        thread_map_erase(std::this_thread::get_id());
    }

public:
    ThreadInstance(FuriThread* _thread_ptr) {
        event_flag = furi_event_flag_alloc();
        thread_ptr = _thread_ptr;
    }

    ~ThreadInstance() {
        furi_event_flag_free(event_flag);
    }

    void set_name(const char* name) {
        this->name = name;
    }

    void set_callback(FuriThreadCallback callback) {
        this->callback = callback;
    }

    void set_stack_size(size_t stack_size) {
        this->stack_size = stack_size;
    }

    void start() {
        thread = std::thread(furi_thread_body, this);
    }

    uint32_t flags_clear(uint32_t flags) {
        return furi_event_flag_clear(event_flag, flags);
    }

    uint32_t flags_set(uint32_t flags) {
        return furi_event_flag_set(event_flag, flags);
    }

    uint32_t flags_get() {
        return furi_event_flag_get(event_flag);
    }

    uint32_t flags_wait(uint32_t flags, uint32_t options, uint32_t timeout) {
        return furi_event_flag_wait(event_flag, flags, options, timeout);
    }
};

struct FuriThread {
    ThreadInstance* instance;
};

FuriThread* furi_thread_alloc() {
    FuriThread* thread = new FuriThread();
    thread->instance = new ThreadInstance(thread);
    return thread;
}

void furi_thread_free(FuriThread* thread) {
    delete thread->instance;
    delete thread;
}

void furi_thread_set_name(FuriThread* thread, const char* name) {
    thread->instance->set_name(name);
}

void furi_thread_set_callback(FuriThread* thread, FuriThreadCallback callback) {
    thread->instance->set_callback(callback);
}

void furi_thread_set_stack_size(FuriThread* thread, size_t stack_size) {
    thread->instance->set_stack_size(stack_size);
}

void furi_thread_start(FuriThread* thread) {
    thread->instance->start();
}

FuriThreadId furi_thread_get_current_id() {
    std::thread::id id = std::this_thread::get_id();
    FuriThread* thread = thread_map_get(id);
    return (FuriThreadId)thread;
}

uint32_t furi_thread_flags_set(FuriThreadId thread_id, uint32_t flags) {
    FuriThread* thread = (FuriThread*)thread_id;
    return thread->instance->flags_set(flags);
}

uint32_t furi_thread_flags_clear(uint32_t flags) {
    std::thread::id id = std::this_thread::get_id();
    FuriThread* thread = thread_map_get(id);
    return thread->instance->flags_clear(flags);
}

uint32_t furi_thread_flags_get(void) {
    std::thread::id id = std::this_thread::get_id();
    FuriThread* thread = thread_map_get(id);
    return thread->instance->flags_get();
}

uint32_t furi_thread_flags_wait(uint32_t flags, uint32_t options, uint32_t timeout) {
    std::thread::id id = std::this_thread::get_id();
    FuriThread* thread = thread_map_get(id);
    return thread->instance->flags_wait(flags, options, timeout);
}