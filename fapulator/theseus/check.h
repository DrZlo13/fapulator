#pragma once
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

#define furi_crash(message)
#define furi_halt(message)

#define furi_check(__e) \
    do {                \
        if(!(__e)) {    \
            abort();    \
        }               \
    } while(0)

#define furi_assert(__e)

#ifdef __cplusplus
}
#endif
