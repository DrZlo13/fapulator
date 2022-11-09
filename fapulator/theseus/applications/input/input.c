#include <furi.h>
#include <hal/input.h>
#include <input/input.h>

const char* input_get_key_name(InputKey key) {
    switch(key) {
    case InputKeyUp:
        return "Up";
    case InputKeyDown:
        return "Down";
    case InputKeyRight:
        return "Right";
    case InputKeyLeft:
        return "Left";
    case InputKeyOk:
        return "Ok";
    case InputKeyBack:
        return "Back";
    default:
        return "Unknown";
    }
}

const char* input_get_type_name(InputType type) {
    switch(type) {
    case InputTypePress:
        return "Press";
    case InputTypeRelease:
        return "Release";
    case InputTypeShort:
        return "Short";
    case InputTypeLong:
        return "Long";
    case InputTypeRepeat:
        return "Repeat";
    default:
        return "Unknown";
    }
}

typedef struct {
    FuriPubSub* event_pubsub;
} Input;

static Input* input = NULL;
#define INPUT_THREAD_FLAG_ISR 0x00000001

static void input_callback(InputEvent* input_event, void* context) {
    Input* input = context;
    furi_pubsub_publish(input->event_pubsub, input_event);
}

int32_t input_srv(void* p) {
    input = malloc(sizeof(Input));
    input->event_pubsub = furi_pubsub_alloc();
    furi_record_create(RECORD_INPUT_EVENTS, input->event_pubsub);
    hal_input_add_callback(input_callback, input);

    while(1) {
        furi_thread_flags_wait(INPUT_THREAD_FLAG_ISR, FuriFlagWaitAny, FuriWaitForever);
    }
}