#pragma once
#include <input/input.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*InputCallback)(InputEvent* input_event, void* context);

void hal_input_add_callback(InputCallback callback, void* context);

#ifdef __cplusplus
}
#endif