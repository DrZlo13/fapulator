#pragma once
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

static constexpr size_t DISPLAY_WIDTH = 128;
static constexpr size_t DISPLAY_HEIGHT = 64;

class DisplayBuffer {
public:
    void set_pixel(size_t x, size_t y, bool value);
    void fill(bool value);
};

DisplayBuffer* get_display_buffer();
void commit_display_buffer(bool redraw);