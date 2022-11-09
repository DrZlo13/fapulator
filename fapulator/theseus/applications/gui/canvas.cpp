#include <gui/canvas.h>
#include <gui/canvas_i.h>
#include <bitset>
#include <hal/display.h>
#include "font/fonts.h"
#include "font/u8g2_font_render.h"

#define U8G2_DRAW_UPPER_RIGHT 0x01
#define U8G2_DRAW_UPPER_LEFT 0x02
#define U8G2_DRAW_LOWER_LEFT 0x04
#define U8G2_DRAW_LOWER_RIGHT 0x08
#define U8G2_DRAW_ALL \
    (U8G2_DRAW_UPPER_RIGHT | U8G2_DRAW_UPPER_LEFT | U8G2_DRAW_LOWER_RIGHT | U8G2_DRAW_LOWER_LEFT)

class CanvasInstance {
private:
    Canvas* canvas;
    std::bitset<DISPLAY_WIDTH * DISPLAY_HEIGHT> display_buffer;
    Color color = ColorBlack;
    CanvasDirection font_direction = CanvasDirectionLeftToRight;
    const uint8_t* font = u8g2_font_haxrcorp4089_tr;

public:
    CanvasInstance(Canvas* _canvas) {
        canvas = _canvas;
    }

    ~CanvasInstance() {
    }

    void fill(bool value) {
        if(value) {
            display_buffer.set();
        } else {
            display_buffer.reset();
        }
    }

    void set_pixel(size_t x, size_t y) {
        if(x < DISPLAY_WIDTH && y < DISPLAY_HEIGHT) {
            display_buffer.set(x + y * DISPLAY_WIDTH, color);
        }
    }

    void clear_pixel(size_t x, size_t y) {
        if(x < DISPLAY_WIDTH && y < DISPLAY_HEIGHT) {
            display_buffer.set(x + y * DISPLAY_WIDTH, !color);
        }
    }

    bool get_pixel(size_t x, size_t y) {
        bool pixel = false;
        if(x < DISPLAY_WIDTH && y < DISPLAY_HEIGHT) {
            pixel = display_buffer.test(x + y * DISPLAY_WIDTH);
        }
        return pixel;
    }

    void set_color(Color _color) {
        color = _color;
    }

    Color get_color() {
        return color;
    }

    void set_font(const uint8_t* font) {
        this->font = font;
    }

    void set_font_direction(CanvasDirection direction) {
        font_direction = direction;
    }

    CanvasDirection get_font_direction() {
        return font_direction;
    }

    static void draw_pixel_fg(uint8_t x, uint8_t y, void* context) {
        CanvasInstance* canvas = (CanvasInstance*)context;
        canvas->set_pixel(x, y);
    }

    static void draw_pixel_bg(uint8_t x, uint8_t y, void* context) {
        CanvasInstance* canvas = (CanvasInstance*)context;
        canvas->clear_pixel(x, y);
    }

    void draw_string(uint16_t x, uint16_t y, const char* text) {
        U8G2FontRender_t render = U8G2FontRender(this->font, draw_pixel_fg, draw_pixel_bg, this);
        U8G2FontRender_Print(&render, x, y, text);
    }

    void draw_vertical_line(uint16_t x, uint16_t y, uint16_t length) {
        for(uint16_t i = 0; i < length; i++) {
            set_pixel(x, y + i);
        }
    }

    void draw_horizontal_line(uint16_t x, uint16_t y, uint16_t length) {
        for(uint16_t i = 0; i < length; i++) {
            set_pixel(x + i, y);
        }
    }

    void draw_circle_section(uint8_t x, uint8_t y, uint8_t x0, uint8_t y0, uint8_t option) {
        /* upper right */
        if(option & U8G2_DRAW_UPPER_RIGHT) {
            set_pixel(x0 + x, y0 - y);
            set_pixel(x0 + y, y0 - x);
        }

        /* upper left */
        if(option & U8G2_DRAW_UPPER_LEFT) {
            set_pixel(x0 - x, y0 - y);
            set_pixel(x0 - y, y0 - x);
        }

        /* lower right */
        if(option & U8G2_DRAW_LOWER_RIGHT) {
            set_pixel(x0 + x, y0 + y);
            set_pixel(x0 + y, y0 + x);
        }

        /* lower left */
        if(option & U8G2_DRAW_LOWER_LEFT) {
            set_pixel(x0 - x, y0 + y);
            set_pixel(x0 - y, y0 + x);
        }
    }

    void draw_circle(uint8_t x0, uint8_t y0, uint8_t rad, uint8_t option) {
        int8_t f;
        int8_t ddF_x;
        int8_t ddF_y;
        uint8_t x;
        uint8_t y;

        f = 1;
        f -= rad;
        ddF_x = 1;
        ddF_y = 0;
        ddF_y -= rad;
        ddF_y *= 2;
        x = 0;
        y = rad;

        draw_circle_section(x, y, x0, y0, option);

        while(x < y) {
            if(f >= 0) {
                y--;
                ddF_y += 2;
                f += ddF_y;
            }
            x++;
            ddF_x += 2;
            f += ddF_x;

            draw_circle_section(x, y, x0, y0, option);
        }
    }

    void draw_disc_section(uint8_t x, uint8_t y, uint8_t x0, uint8_t y0, uint8_t option) {
        /* upper right */
        if(option & U8G2_DRAW_UPPER_RIGHT) {
            draw_vertical_line(x0 + x, y0 - y, y + 1);
            draw_vertical_line(x0 + y, y0 - x, x + 1);
        }

        /* upper left */
        if(option & U8G2_DRAW_UPPER_LEFT) {
            draw_vertical_line(x0 - x, y0 - y, y + 1);
            draw_vertical_line(x0 - y, y0 - x, x + 1);
        }

        /* lower right */
        if(option & U8G2_DRAW_LOWER_RIGHT) {
            draw_vertical_line(x0 + x, y0, y + 1);
            draw_vertical_line(x0 + y, y0, x + 1);
        }

        /* lower left */
        if(option & U8G2_DRAW_LOWER_LEFT) {
            draw_vertical_line(x0 - x, y0, y + 1);
            draw_vertical_line(x0 - y, y0, x + 1);
        }
    }

    void draw_disc(uint8_t x0, uint8_t y0, uint8_t rad, uint8_t option) {
        int8_t f;
        int8_t ddF_x;
        int8_t ddF_y;
        uint8_t x;
        uint8_t y;

        f = 1;
        f -= rad;
        ddF_x = 1;
        ddF_y = 0;
        ddF_y -= rad;
        ddF_y *= 2;
        x = 0;
        y = rad;

        draw_disc_section(x, y, x0, y0, option);

        while(x < y) {
            if(f >= 0) {
                y--;
                ddF_y += 2;
                f += ddF_y;
            }
            x++;
            ddF_x += 2;
            f += ddF_x;

            draw_disc_section(x, y, x0, y0, option);
        }
    }

    void draw_box(uint8_t x, uint8_t y, uint8_t width, uint8_t height) {
        for(uint8_t i = 0; i < height; i++) {
            draw_horizontal_line(x, y + i, width);
        }
    }

    void draw_frame(uint8_t x, uint8_t y, uint8_t width, uint8_t height) {
        draw_vertical_line(x, y, height);
        draw_vertical_line(x + width - 1, y, height);
        draw_horizontal_line(x, y, width);
        draw_horizontal_line(x, y + height - 1, width);
    }

    void draw_rounded_frame(uint8_t x, uint8_t y, uint8_t width, uint8_t height, uint8_t radius) {
        draw_horizontal_line(x + radius, y, width - 2 * radius);
        draw_horizontal_line(x + radius, y + height - 1, width - 2 * radius);
        draw_vertical_line(x, y + radius, height - 2 * radius);
        draw_vertical_line(x + width - 1, y + radius, height - 2 * radius);
        draw_circle(x + radius + 1, y + radius, radius, U8G2_DRAW_UPPER_RIGHT);
        draw_circle(x + width - radius - 2, y + radius, radius, U8G2_DRAW_UPPER_LEFT);
        draw_circle(x + radius + 1, y + height - radius - 1, radius, U8G2_DRAW_LOWER_RIGHT);
        draw_circle(x + width - radius - 2, y + height - radius - 1, radius, U8G2_DRAW_LOWER_LEFT);
    }
};

Canvas* canvas_init() {
    Canvas* canvas = new Canvas;
    canvas->orientation = CanvasOrientationHorizontal;
    canvas->fb = new CanvasInstance(canvas);

    // Clear buffer and send to device
    canvas_clear(canvas);
    canvas_commit(canvas);

    return canvas;
}

void canvas_free(Canvas* canvas) {
    delete static_cast<CanvasInstance*>(canvas->fb);
    delete canvas;
}

void canvas_clear(Canvas* canvas) {
    static_cast<CanvasInstance*>(canvas->fb)->fill(false);
}

void canvas_commit(Canvas* canvas) {
    CanvasInstance* canvas_instance = static_cast<CanvasInstance*>(canvas->fb);
    DisplayBuffer* display_buffer = get_display_buffer();

    for(size_t x = 0; x < DISPLAY_WIDTH; x++) {
        for(size_t y = 0; y < DISPLAY_HEIGHT; y++) {
            display_buffer->set_pixel(x, y, canvas_instance->get_pixel(x, y));
        }
    }
    commit_display_buffer(true);
}

void canvas_set_font(Canvas* canvas, Font font) {
    CanvasInstance* canvas_instance = static_cast<CanvasInstance*>(canvas->fb);

    if(font == FontPrimary) {
        canvas_instance->set_font(u8g2_font_helvB08_tr);
    } else if(font == FontSecondary) {
        canvas_instance->set_font(u8g2_font_haxrcorp4089_tr);
    } else if(font == FontKeyboard) {
        canvas_instance->set_font(u8g2_font_profont11_mr);
    } else if(font == FontBigNumbers) {
        canvas_instance->set_font(u8g2_font_profont22_tn);
    } else {
        abort();
    }
}

void canvas_draw_str(Canvas* canvas, uint8_t x, uint8_t y, const char* str) {
    CanvasInstance* canvas_instance = static_cast<CanvasInstance*>(canvas->fb);
    if(!str) return;
    x += canvas->offset_x;
    y += canvas->offset_y;
    canvas_instance->draw_string(x, y, str);
}

void canvas_draw_circle(Canvas* canvas, uint8_t x, uint8_t y, uint8_t radius) {
    CanvasInstance* canvas_instance = static_cast<CanvasInstance*>(canvas->fb);
    x += canvas->offset_x;
    y += canvas->offset_y;
    canvas_instance->draw_circle(x, y, radius, U8G2_DRAW_ALL);
}

void canvas_draw_disc(Canvas* canvas, uint8_t x, uint8_t y, uint8_t radius) {
    CanvasInstance* canvas_instance = static_cast<CanvasInstance*>(canvas->fb);
    x += canvas->offset_x;
    y += canvas->offset_y;
    canvas_instance->draw_disc(x, y, radius, U8G2_DRAW_ALL);
}

void canvas_set_orientation(Canvas* canvas, CanvasOrientation orientation) {
    // TODO: Implement
}

void canvas_frame_set(
    Canvas* canvas,
    uint8_t offset_x,
    uint8_t offset_y,
    uint8_t width,
    uint8_t height) {
    canvas->offset_x = offset_x;
    canvas->offset_y = offset_y;
    canvas->width = width;
    canvas->height = height;
}

void canvas_reset(Canvas* canvas) {
    canvas_clear(canvas);
    canvas_set_color(canvas, ColorBlack);
    canvas_set_font(canvas, FontSecondary);
    canvas_set_font_direction(canvas, CanvasDirectionLeftToRight);
}

void canvas_set_color(Canvas* canvas, Color color) {
    CanvasInstance* canvas_instance = static_cast<CanvasInstance*>(canvas->fb);
    canvas_instance->set_color(color);
}

void canvas_set_font_direction(Canvas* canvas, CanvasDirection dir) {
    CanvasInstance* canvas_instance = static_cast<CanvasInstance*>(canvas->fb);
    canvas_instance->set_font_direction(dir);
}

void canvas_draw_box(Canvas* canvas, uint8_t x, uint8_t y, uint8_t width, uint8_t height) {
    CanvasInstance* canvas_instance = static_cast<CanvasInstance*>(canvas->fb);
    x += canvas->offset_x;
    y += canvas->offset_y;
    canvas_instance->draw_box(x, y, width, height);
}

void canvas_draw_frame(Canvas* canvas, uint8_t x, uint8_t y, uint8_t width, uint8_t height) {
    CanvasInstance* canvas_instance = static_cast<CanvasInstance*>(canvas->fb);
    x += canvas->offset_x;
    y += canvas->offset_y;
    canvas_instance->draw_frame(x, y, width, height);
}

void canvas_draw_rframe(
    Canvas* canvas,
    uint8_t x,
    uint8_t y,
    uint8_t width,
    uint8_t height,
    uint8_t radius) {
    CanvasInstance* canvas_instance = static_cast<CanvasInstance*>(canvas->fb);
    x += canvas->offset_x;
    y += canvas->offset_y;
    canvas_instance->draw_rounded_frame(x, y, width, height, radius);
}