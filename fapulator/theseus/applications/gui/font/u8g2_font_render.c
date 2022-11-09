/*
 * u8g2_font_render.c
 *
 *  Created on: Nov 27, 2020
 *      Author: quard
 */
#include "u8g2_font_render.h"

uint8_t font_get_unsigned_bits(U8G2FontGlyph_t* glyph, uint8_t count);
int8_t font_get_signed_bits(U8G2FontGlyph_t* glyph, uint8_t count);
uint16_t font_get_start_symbol_search_postition(U8G2FontRender_t* font, char chr);
int8_t font_get_glyph(
    U8G2FontRender_t* font,
    U8G2FontGlyph_t* glyph,
    uint16_t search_position,
    char chr);
void font_parse_glyph_header(U8G2FontRender_t* font, U8G2FontGlyph_t* glyph);
uint8_t font_draw_start_x_position(U8G2FontRender_t* font, U8G2FontGlyph_t* glyph);
uint8_t font_draw_start_y_position(U8G2FontRender_t* font, U8G2FontGlyph_t* glyph);
void font_render_glyph(U8G2FontRender_t* font, U8G2FontGlyph_t* glyph, uint8_t x, uint8_t y);

U8G2FontRender_t U8G2FontRender(
    const uint8_t* data,
    fnDrawPixel drawFgPixel,
    fnDrawPixel drawBgPixel,
    void* context) {
    U8G2FontRender_t font = {
        .data = data,
        .drawFgPixel = drawFgPixel,
        .drawBgPixel = drawBgPixel,
        .context = context,
    };

    font.header = U8G2FontRender_ParseHeader(&font);

    return font;
}

U8G2FontHeader_t U8G2FontRender_ParseHeader(U8G2FontRender_t* font) {
    U8G2FontHeader_t header;

    memcpy(&header, font->data, U8G2_FONT_HEADER_SIZE);
    header.offset_A = U8G2_FONT_HEADER_SIZE + (font->data[17] << 8 | font->data[18]);
    header.offset_a = U8G2_FONT_HEADER_SIZE + (font->data[19] << 8 | font->data[20]);
    header.offset_0x100 = U8G2_FONT_HEADER_SIZE + (font->data[21] << 8 | font->data[22]);

    return header;
}

void U8G2FontRender_PrintChar(U8G2FontRender_t* font, uint8_t* x, uint8_t y, char chr) {
    uint16_t search_position = font_get_start_symbol_search_postition(font, chr);

    U8G2FontGlyph_t glyph;
    if(font_get_glyph(font, &glyph, search_position, chr) != U8G2FontRender_OK) {
        return;
    }
    font_parse_glyph_header(font, &glyph);

    font_render_glyph(font, &glyph, *x, y);

    *x += glyph.pitch;
}

void U8G2FontRender_Print(U8G2FontRender_t* font, uint8_t x, uint8_t y, const char* str) {
    while(*str) {
        const char* chr = str++;
        // if(*chr < 0x100) {
        U8G2FontRender_PrintChar(font, &x, y, *chr);
        // }
    }
}

uint8_t font_get_unsigned_bits(U8G2FontGlyph_t* glyph, uint8_t count) {
    uint8_t val;
    uint8_t start = glyph->bit_pos;
    uint8_t end = start + count;

    val = pgm_read(glyph->data);
    val >>= start;

    if(end >= 8) {
        uint8_t cnt = 8 - start;
        glyph->data++;

        val |= pgm_read(glyph->data) << (cnt);

        end -= 8;
    }

    glyph->bit_pos = end;

    val &= (1U << count) - 1;

    return val;
}

int8_t font_get_signed_bits(U8G2FontGlyph_t* glyph, uint8_t count) {
    int8_t val = (int8_t)font_get_unsigned_bits(glyph, count);
    val -= 1 << (count - 1);

    return val;
}

uint16_t font_get_start_symbol_search_postition(U8G2FontRender_t* font, char chr) {
    uint16_t search_position = U8G2_FONT_HEADER_SIZE;
    if(chr >= 65 && chr <= 90) {
        search_position = font->header.offset_A;
    } else if(chr >= 97 && chr <= 122) {
        search_position = font->header.offset_a;
    }

    return search_position;
}

int8_t font_get_glyph(
    U8G2FontRender_t* font,
    U8G2FontGlyph_t* glyph,
    uint16_t search_position,
    char chr) {
    while(1) {
        memcpy(glyph, font->data + search_position, 2);
        if(glyph->character == chr) {
            glyph->data = font->data + search_position + 2;
            glyph->bit_pos = 0;

            return U8G2FontRender_OK;
        }

        search_position += glyph->next_glypth;
        if(glyph->next_glypth == 0) {
            break;
        }
    }

    return U8G2FontRender_ERR;
}

void font_parse_glyph_header(U8G2FontRender_t* font, U8G2FontGlyph_t* glyph) {
    glyph->width = font_get_unsigned_bits(glyph, font->header.glyph_width);
    glyph->height = font_get_unsigned_bits(glyph, font->header.glyph_height);
    glyph->x_offset = font_get_signed_bits(glyph, font->header.glyph_x_offset);
    glyph->y_offset = font_get_signed_bits(glyph, font->header.glyph_y_offset);
    glyph->pitch = font_get_signed_bits(glyph, font->header.glyph_pitch);
}

uint8_t font_draw_start_x_position(U8G2FontRender_t* font, U8G2FontGlyph_t* glyph) {
    return glyph->x_offset;
}

uint8_t font_draw_start_y_position(U8G2FontRender_t* font, U8G2FontGlyph_t* glyph) {
    return -glyph->height - glyph->y_offset;
}

void font_render_glyph(U8G2FontRender_t* font, U8G2FontGlyph_t* glyph, uint8_t x, uint8_t y) {
    uint32_t pixels = 0;
    uint8_t y_pos = y + font_draw_start_y_position(font, glyph);
    uint8_t x_pos = x + font_draw_start_x_position(font, glyph);
    while(1) {
        uint8_t zeros = font_get_unsigned_bits(glyph, font->header.zero_bit_width);
        uint8_t ones = font_get_unsigned_bits(glyph, font->header.one_bit_width);
        int8_t repeat = 0;

        while(font_get_unsigned_bits(glyph, 1) == 1) {
            repeat++;
        }

        for(; repeat >= 0; repeat--) {
            for(uint8_t i = 0; i < zeros + ones; i++) {
                if(i <= zeros - 1) {
                    font->drawBgPixel(x_pos, y_pos, font->context);
                } else {
                    font->drawFgPixel(x_pos, y_pos, font->context);
                }
                x_pos++;

                pixels++;
                if(pixels % glyph->width == 0) {
                    y_pos++;
                    x_pos = x + font_draw_start_x_position(font, glyph);
                }
            }
        }

        if(pixels >= glyph->width * glyph->height) {
            break;
        }
    }
}