#include <core/message_queue.h>
#include <queue>
#include <mutex>

// TODO: add queue size limit and timeout for send

class QueueInstance {
private:
    uint32_t msg_size;

    mutable std::mutex mutex;
    std::queue<void*> queue;
    std::condition_variable notifier;

    void push(void* msg) {
        void* msg_copy = malloc(msg_size);
        memcpy(msg_copy, msg, msg_size);
        queue.push(msg_copy);
    }

    void pop(void* msg) {
        void* msg_copy = queue.front();
        memcpy(msg, msg_copy, msg_size);
        free(msg_copy);
        queue.pop();
    }

    void pop_and_free() {
        void* msg_copy = queue.front();
        free(msg_copy);
        queue.pop();
    }

public:
    QueueInstance(uint32_t _msg_size) {
        msg_size = _msg_size;
    }

    ~QueueInstance() {
        reset();
    }

    FuriStatus send(void* msg) {
        {
            std::unique_lock<std::mutex> lock(mutex);
            push(msg);
        }
        notifier.notify_one();
        return FuriStatusOk;
    }

    FuriStatus receive(void* msg) {
        std::unique_lock<std::mutex> lock(mutex);
        while(queue.empty()) {
            notifier.wait(lock);
        }
        pop(msg);
        return FuriStatusOk;
    }

    FuriStatus receive(void* msg, std::chrono::milliseconds timeout) {
        std::unique_lock<std::mutex> lock(mutex);
        FuriStatus status = FuriStatusErrorTimeout;
        if(notifier.wait_for(lock, timeout, [this] { return !queue.empty(); })) {
            pop(msg);
            status = FuriStatusOk;
        }
        return status;
    }

    uint32_t get_msg_count() {
        std::unique_lock<std::mutex> lock(mutex);
        return queue.size() / sizeof(void*);
    }

    uint32_t get_free_space() {
        std::unique_lock<std::mutex> lock(mutex);
        return UINT32_MAX;
    }

    uint32_t get_msg_size() {
        return msg_size;
    }

    FuriStatus reset() {
        std::unique_lock<std::mutex> lock(mutex);
        while(!queue.empty()) {
            pop_and_free();
        }
        return FuriStatusOk;
    }
};

FuriMessageQueue* furi_message_queue_alloc(uint32_t msg_count, uint32_t msg_size) {
    (void)msg_count;
    return (FuriMessageQueue*)new QueueInstance(msg_size);
}

void furi_message_queue_free(FuriMessageQueue* instance) {
    delete(QueueInstance*)instance;
}

FuriStatus
    furi_message_queue_put(FuriMessageQueue* instance, const void* msg_ptr, uint32_t timeout) {
    (void)timeout;
    QueueInstance* queue = (QueueInstance*)instance;
    return queue->send((void*)msg_ptr);
}

FuriStatus furi_message_queue_get(FuriMessageQueue* instance, void* msg_ptr, uint32_t timeout) {
    QueueInstance* queue = (QueueInstance*)instance;
    if(timeout == FuriWaitForever) {
        return queue->receive(msg_ptr);
    } else {
        return queue->receive(msg_ptr, std::chrono::milliseconds(timeout));
    }
}

uint32_t furi_message_queue_get_capacity(FuriMessageQueue* instance) {
    QueueInstance* queue = (QueueInstance*)instance;
    return queue->get_free_space();
}

uint32_t furi_message_queue_get_message_size(FuriMessageQueue* instance) {
    QueueInstance* queue = (QueueInstance*)instance;
    return queue->get_msg_size();
}

uint32_t furi_message_queue_get_count(FuriMessageQueue* instance) {
    QueueInstance* queue = (QueueInstance*)instance;
    return queue->get_msg_count();
}

uint32_t furi_message_queue_get_space(FuriMessageQueue* instance) {
    QueueInstance* queue = (QueueInstance*)instance;
    return queue->get_free_space();
}

FuriStatus furi_message_queue_reset(FuriMessageQueue* instance) {
    QueueInstance* queue = (QueueInstance*)instance;
    return queue->reset();
}