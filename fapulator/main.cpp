#include <stdint.h>
#include <stddef.h>
#include <furi.h>

#define TAG "LoaderSrv"

extern "C" int32_t gui_srv(void* p);
extern "C" int32_t input_srv(void* p);
extern void hal_pre_init(void);
extern void hal_post_init(void);

typedef struct {
    const FuriThreadCallback app;
    const char* name;
    const size_t stack_size;
} FlipperApplication;

static bool start_application(const FlipperApplication* application, const char* arguments) {
    FURI_LOG_I(TAG, "Starting: %s", application->name);

    FuriThread* thread = furi_thread_alloc();
    furi_thread_set_name(thread, application->name);
    furi_thread_set_stack_size(thread, application->stack_size);
    furi_thread_set_callback(thread, application->app);
    furi_thread_start(thread);

    return true;
}

extern "C" int32_t snake_game_app(void* p);
extern "C" int32_t keypad_test_app(void* p);

static FlipperApplication applications[] = {
    {input_srv, "InputService", 1024 * 4},
    {gui_srv, "GuiService", 1024 * 4},
    // {snake_game_app, "Snake Game", 1024 * 4},
    {keypad_test_app, "Keypad Test", 1024 * 4},
};

int main(int argc, char** argv) {
    hal_pre_init();

    for(size_t i = 0; i < sizeof(applications) / sizeof(FlipperApplication); i++) {
        start_application(&applications[i], NULL);
    }

    hal_post_init();

    return 0;
}