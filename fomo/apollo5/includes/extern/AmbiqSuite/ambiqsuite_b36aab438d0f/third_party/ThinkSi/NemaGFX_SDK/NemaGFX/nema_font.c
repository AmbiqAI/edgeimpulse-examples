/* TSI 2023.gen */
// -----------------------------------------------------------------------------
// Copyright (c) 2008-23 Think Silicon Single Member PC
// Think Silicon Single Member PC Confidential Proprietary
// -----------------------------------------------------------------------------
//     All Rights reserved - Unpublished -rights reserved under
//         the Copyright laws of the European Union
//
//  This file includes the Confidential information of Think Silicon Single
//  Member PC The receiver of this Confidential Information shall not disclose
//  it to any third party and shall protect its confidentiality by using the
//  same degree of care, but not less then a reasonable degree of care, as the
//  receiver uses to protect receiver's own Confidential Information. The entire
//  notice must be reproduced on all authorized copies and copies may only be
//  made to the extent permitted by a licensing agreement from Think Silicon
//  Single Member PC.
//
//  The software/data is provided 'as is', without warranty of any kind,
//  expressed or implied, including but not limited to the warranties of
//  merchantability, fitness for a particular purpose and noninfringement. In no
//  event shall Think Silicon Single Member PC be liable for any claim, damages
//  or other liability, whether in an action of contract, tort or otherwise,
//  arising from, out of or in connection with the software.
//
//  For further information please contact:
//
//                    Think Silicon Single Member PC
//                    http://www.think-silicon.com
//                    Patras Science Park
//                    Rion Achaias 26504
//                    Greece
// -----------------------------------------------------------------------------
#include "nema_core.h"
#include "nema_programHW.h"
#include "nema_regs.h"
#include "nema_ringbuffer.h"
#include "nema_rasterizer.h"
#include "nema_font.h"
#include "nema_raster.h"
#include "nema_error.h"
#include "nema_error_intern.h"

static TLS_VAR nema_font_t *bound_font;

void
nema_bind_font(nema_font_t *font) {
    bound_font = font;
}

static const uint32_t UTF8_ONE_BYTE_MASK = 0x80;
static const uint32_t UTF8_ONE_BYTE_BITS = 0;
static const uint32_t UTF8_TWO_BYTES_MASK = 0xE0;
static const uint32_t UTF8_TWO_BYTES_BITS = 0xC0;
static const uint32_t UTF8_THREE_BYTES_MASK = 0xF0;
static const uint32_t UTF8_THREE_BYTES_BITS = 0xE0;
static const uint32_t UTF8_FOUR_BYTES_MASK = 0xF8;
static const uint32_t UTF8_FOUR_BYTES_BITS = 0xF0;
static const uint32_t UTF8_CONTINUATION_MASK = 0xC0;
// static const uint32_t UTF8_CONTINUATION_BITS = 0x80;

static int utf8_codepoint_size(const uint8_t *byte)
{
    if ((*byte & UTF8_ONE_BYTE_MASK) == UTF8_ONE_BYTE_BITS) {
        return 1;
    }

    if ((*byte & UTF8_TWO_BYTES_MASK) == UTF8_TWO_BYTES_BITS) {
        return 2;
    }

    if ((*byte & UTF8_THREE_BYTES_MASK) == UTF8_THREE_BYTES_BITS) {
        return 3;
    }

    if ((*byte & UTF8_FOUR_BYTES_MASK) == UTF8_FOUR_BYTES_BITS) {
        return 4;
    }

    return 1;
}

static uint32_t get_codepoint(const uint8_t *utf8) {
    int cp_size = utf8_codepoint_size(utf8);
    uint32_t utf32;

    switch (cp_size) {
    case 1:
        utf32 = ((uint32_t) utf8[0] & ~UTF8_ONE_BYTE_MASK);
        break;
    case 2:
        utf32 =
            ((uint32_t) utf8[0] & ~UTF8_TWO_BYTES_MASK   ) << 6 |
            ((uint32_t) utf8[1] & ~UTF8_CONTINUATION_MASK)
        ;
        break;
    case 3:
        utf32 =
            ((uint32_t) utf8[0] & ~UTF8_THREE_BYTES_MASK ) << 12 |
            ((uint32_t) utf8[1] & ~UTF8_CONTINUATION_MASK) << 6  |
            ((uint32_t) utf8[2] & ~UTF8_CONTINUATION_MASK)
        ;

        break;
    case 4:
        utf32 =
            ((uint32_t) utf8[0] & ~UTF8_FOUR_BYTES_MASK  ) << 18 |
            ((uint32_t) utf8[1] & ~UTF8_CONTINUATION_MASK) << 12 |
            ((uint32_t) utf8[2] & ~UTF8_CONTINUATION_MASK) << 6  |
            ((uint32_t) utf8[3] & ~UTF8_CONTINUATION_MASK)
        ;
        break;
    default:
        utf32 = 0;
        break;
    }

    return utf32;
}

inline static const nema_font_range_t *
nema_get_codepoint_range(const uint32_t codepoint) {
    int range = 0;

    const nema_font_range_t *range_ptr = &bound_font->ranges[range];

    while ( range_ptr->glyphs != NULL ) {
        if ( codepoint >= range_ptr->first && codepoint <= range_ptr->last ) {
            return range_ptr;
        }

        ++range;
        range_ptr = &bound_font->ranges[range];
    }

    return NULL;
}

inline static uint32_t
nema_get_glyph_height(const nema_glyph_t *glyph) {
    uint32_t cur_off = glyph[0].bitmapOffset;
    uint32_t nxt_off = glyph[1].bitmapOffset;


    uint32_t diff = nxt_off-cur_off;
    uint32_t width_bits = (uint32_t)glyph->width*bound_font->bpp;
    uint32_t width_bytes = (width_bits+7U)/8U;
    uint32_t height = diff/width_bytes;

    return height;
}

inline static uint32_t
nema_get_indexed_glyph_height(const nema_glyph_indexed_t *glyph) {
    uint32_t cur_off = (uint32_t) glyph[0].bitmapOffset;
    uint32_t nxt_off = (uint32_t) glyph[1].bitmapOffset;

    uint32_t diff = nxt_off-cur_off;
    uint32_t width_bits = (uint32_t)glyph->width*bound_font->bpp;
    uint32_t width_bytes = (width_bits+7U)/8U;
    uint32_t height = diff/width_bytes;

    return height;
}

inline static const nema_font_range_t *
nema_get_range(const char *ch){
    const uint8_t *utf8 = (const uint8_t *)ch;
    uint32_t codepoint  = get_codepoint(utf8);
    const nema_font_range_t *range_ptr = nema_get_codepoint_range(codepoint);
    return range_ptr;
}

inline static const nema_glyph_t *
nema_get_glyph(const char *ch) {
    const uint8_t *utf8 = (const uint8_t *)ch;
    uint32_t codepoint = get_codepoint(utf8);
    if(codepoint == 0x00000000U ){
        return NULL;
    }
    const nema_font_range_t *range_ptr = nema_get_codepoint_range(codepoint);
    if (range_ptr == NULL) {
        return NULL;
    }

    const nema_glyph_t *glyph = &range_ptr->glyphs[ codepoint - range_ptr->first ];

    return glyph;
}

inline static const nema_glyph_indexed_t *
nema_get_indexed_glyph(int id) {

    if ( (bound_font->indexed_glyphs) == NULL) {
        return NULL;
    }

    int i = 0;

    do {
        if ( bound_font->indexed_glyphs[i].id == id ) {
           return (&bound_font->indexed_glyphs[i]);
        }
        i++;
    } while ( bound_font->indexed_glyphs[i].id >= 0 ); //end of indices is marked with negative id

    return NULL;
}

inline static uint8_t
nema_get_glyph_xadvance(const nema_glyph_t *glyph) {
    if ( glyph != NULL ) {
        return glyph->xAdvance;
    }

    return bound_font->xAdvance;
}

inline static int
nema_get_kern_xoffset(const char *left, const char *right){

    if ( (left == NULL) || (right == NULL) || (bound_font == NULL) || (bound_font->kern_pairs == NULL) ) {
        return 0;
    }

    const uint8_t *utf8_left      = (const uint8_t *) left;
    uint32_t codepoint_left       = get_codepoint(utf8_left);
    const nema_glyph_t *cur_glyph = nema_get_glyph( (const char*)right );
    int x_offset                  = 0;

    //if kerning is available, return the horizontal offset
    if ( (cur_glyph != NULL) && ((int)cur_glyph->kern_length > 0) ) {
        uint8_t i;
        for ( i = 0; i < cur_glyph->kern_length; ++i ) {
            if ( codepoint_left == bound_font->kern_pairs[cur_glyph->kern_offset + i].left ) { // && (codepoint_right == cur_range->kern_pairs[cur_glyph->kern_offset + i].right )
                return ((int) bound_font->kern_pairs[cur_glyph->kern_offset + i].x_offset);
            }
        }
    }
    return x_offset;
}

inline static void set_blend(uint32_t color) {
    if ( (color & 0xFF000000U) == 0xFF000000U) {
        nema_set_blend_blit(NEMA_BL_SRC_OVER|NEMA_BLOP_SRC_PREMULT);
    } else {
        nema_set_blend_blit(NEMA_BL_SIMPLE|NEMA_BLOP_MODULATE_A);
        nema_set_const_color(color);
    }
}

inline static int
nema_print_char_internal(const char *ch, int x, int y, int min_x, int min_y, int max_x, int max_y) {
    const nema_glyph_t *glyph = nema_get_glyph(ch);

    if ( glyph == NULL ) {
        return 0;
    }

    int x_ = x + glyph->xOffset;
    int y_ = y + glyph->yOffset;

    uint32_t height = nema_get_glyph_height(glyph);

    int _x0 = x_ < min_x ? min_x : x_;
    int _y0 = y_ < min_y ? min_y : y_;
    int _x1 = x_ + (int)glyph->width;
    int _y1 = y_ + (int)height;

    if (_x1 > max_x) {
        _x1 = max_x;
    }
    if (_y1 > max_y) {
        _y1 = max_y;
    }

    int _w = _x1 - _x0;
    int _h = _y1 - _y0;

    if (_w > 0 && _h > 0) {
        nema_tex_format_t format;
        switch (bound_font->bpp) {
            case 1u:
                format = NEMA_BW1;
                break;
            case 2u:
                format = NEMA_A2;
                break;
            case 4u:
                format = NEMA_A4;
                break;
            default:
                format = NEMA_A8;
                break;
        }

        nema_bind_tex(NEMA_TEX1,
                      bound_font->bo.base_phys + glyph->bitmapOffset,
                      glyph->width,
                      height,
                      format, -1,
                      NEMA_FILTER_PS);

        nema_set_matrix_translate((float) x_, (float) y_);
        if ( nema_context.surface_tile != 0U ) {
            nema_raster_rect(_x0, _y0, _w, _h);
        }
        else {
            uint32_t *cmd_array = nema_cl_get_space(3);
            if (cmd_array == NULL) {
                return 0;
            }

            cmd_array[ 0] = NEMA_DRAW_STARTXY;
            cmd_array[ 1] = YX16TOREG32(_y0, _x0);
            cmd_array[ 2] = NEMA_DRAW_ENDXY  ;
            int _y2 = _y0+_h;
            int _x2 = _x0+_w;
            cmd_array[ 3] = YX16TOREG32(_y2, _x2);
            cmd_array[ 4] = NEMA_DRAW_CMD    ;
            cmd_array[ 5] = nema_context.surface_tile | nema_context.color_grad | DRAW_BOX;
        }
    }

    uint8_t xAdvance = glyph->xAdvance;

    return (int)xAdvance;
}

//return count of the last non-space characters
//update cur_w like end-of-line spaces didn't exist
static inline int
count_eol_spaces(const char *str, int idx, int *cur_w) {
    int spaces = 0;
    int idx_   = idx;
    while ( idx_ >= 0 && str[idx_] == ' ' ) {
        --idx_;
        ++spaces;
    }

    if (cur_w != NULL && spaces > 0) {
        *cur_w -= spaces*(int)bound_font->xAdvance;
    }

    return idx_;
}

// find end of word
// a word ends with '\0', ' ' or '\n'
// returns the index to that ending element
static inline int
find_eow(const char *str, int i, int *word_width) {
    int cur_w = 0;
    int i_    = i;
    const char *left_char_ptr = NULL;
    // if the word starts with an ending element
    // it counts as a word, return immediately
    // width should be zero in that case
    while ( str[i_] != '\0' && str[i_] != ' ' && str[i_] != '\n') {

        // update cur_w and i
        if ( str[i_] != ' ' ) {
            const nema_glyph_t *glyph = nema_get_glyph(&str[i_]);
            cur_w += (int)nema_get_glyph_xadvance(glyph);

            // int left_bytes = utf8_codepoint_size((const uint8_t *)left_char_ptr);
            // uint32_t left_addr;
            // for ( int k = 0; k < left_bytes; ++k ) {
            //     left_addr =
            // }

            cur_w += nema_get_kern_xoffset(left_char_ptr, &str[i_]);

            left_char_ptr = &str[i_];
            i_ += utf8_codepoint_size((const uint8_t *)&str[i_]);
        } else {
            // cur_w += (int)bound_font->xAdvance;
            // ++i;
        }


    }

    *word_width = cur_w;
    return i_;
}

// find end of line
// Multiple consequtive spaces count as words
// A line ends
//      - explicitely with '\n'
//      - implicitely '\0'
//      - implicitely with wrapping
//
// returns the index to that ending element
static inline int
find_eol(const char *str, int i, int max_w, int *line_width, int *const space_count, uint32_t wrap) {
    int width = 0;
    int i_    = i;

    //do wrapping only if both wrap is enabled and max_w is > 0
    bool do_wrap = wrap != 0U && max_w > 0;
    int spaces = 0;

    // if (str[i] == ' ') {
    //     ++spaces;
    // }

    // while not the end of the string
    while ( (str[i_] != '\0') ) {
        int word_width;

        // eow_idx always points to '\0', ' ', '\n'
        int eow_idx = find_eow(str, i_, &word_width);

        // if wrapping is enabled
        // and at least one word has been written
        // and we've exceeded max_w
        // line has ended
        if ( do_wrap &&
             width > 0 &&
             (width+word_width) > max_w ) {
            //Current word exceeds max_w
            //Discard current word
            --i_;
            break;
        }

        width += word_width;
        i_ = eow_idx;

        if (str[i_] == ' ') {
            ++spaces;
            width += (int)bound_font->xAdvance;
        }
        else if ( str[i_] == '\n' ) {
            //carriage return
            //NEW LINE

            break;
        }
        else if ( str[i_] == '\0' ) {
            //end of string
            //NEW LINE

            break;
        }
        else {
            //misra
        }

        ++i_;
    }

    if (line_width != NULL ) {
        *line_width = width;
    }

    if (space_count != NULL ) {
        *space_count = spaces;
    }

    return i_;
}


int
nema_string_get_bbox(const char *str, int *w, int *h, int max_w, uint32_t wrap) {
    int max_w_ = max_w; //linting alias
    if ( bound_font == NULL ) {
        nema_set_error(NEMA_ERR_NO_BOUND_FONT);
        return -1;
    }

    if ( str == NULL ) {
        return -1;
    }

    if (max_w_ <= 0) {
        max_w_ = 0xffffff; //MAX_INT
    }

    *w = 0;
    *h = (int)bound_font->yAdvance;

    int bbox_w = 0;
    int i = 0;
    int carriage_returns = 0;

    while (str[i] != '\0') {
        int line_width;
        i = find_eol(str, i, max_w_, &line_width, NULL, wrap);

        if (str[i] != '\0') {
            //since last character is ' ' or '\n', not '\0'
            ++i;
        }

        ++carriage_returns;

        if (line_width > bbox_w) {
            bbox_w = line_width;
        }
    }

    if ( bbox_w > max_w_ ) {
        bbox_w = max_w_;
    }

    *w = bbox_w;
    *h = carriage_returns*(int)bound_font->yAdvance;

    return carriage_returns;
}

#define CARRIAGE_RETURN                                                 \
            new_line = (alignX != NEMA_ALIGNX_LEFT) || (wrap != 0U);    \
            cur_x = (float)x_off;                                       \
            cur_y += yAdvance;                                          \
            if ((int)cur_y >= max_y + (int)bound_font->max_ascender) {  \
                break;                                                  \
            }                                                           \

void
nema_print_to_position(const char *str, int *pos_x, int *pos_y, int x, int y, int w, int h, uint32_t fg_col, uint32_t align) {
    if ( bound_font == NULL ) {
        nema_set_error(NEMA_ERR_NO_BOUND_FONT);
        return;
    }

    if ( str == NULL ) {
        return;
    }

    //just aliases
    int x_off = x;
    int y_off = y;

    //pos_x/y are relative coordinates (inside text area)
    int pos_x_ = pos_x == NULL ? 0 : *pos_x;
    int pos_y_ = pos_y == NULL ? 0 : *pos_y;

    if ( pos_y_ >= h ) {
        //already below the bottom of the text area
        return;
    }

    //doing int to float this way for misra
    int   cur_x_ = (pos_x_ + x_off);
    float cur_x  = (float)cur_x_;
    int   cur_y_ = pos_y_ + y_off + (int)bound_font->max_ascender;
    float cur_y = (float) cur_y_;
    float yAdvance = (float)bound_font->yAdvance;
    const uint32_t wrap   = (align & NEMA_TEXT_WRAP);
    const uint32_t alignX = (align & NEMA_ALIGNX_MASK);
    const uint32_t alignY = (align & NEMA_ALIGNY_MASK);

    if ( alignY != NEMA_ALIGNY_TOP ) {
        int tmp_w, tmp_h, carriage_returns;
        carriage_returns = nema_string_get_bbox(str, &tmp_w, &tmp_h, w, wrap);
        int _h = h - tmp_h;

        if      (alignY == NEMA_ALIGNY_BOTTOM) {
            cur_y += (float)_h;
        }
        else if (alignY == NEMA_ALIGNY_CENTER) {
            cur_y += ((float)_h)*0.5f;
        }
        else if (alignY == NEMA_ALIGNY_JUSTIFY) {
            if ( (carriage_returns > 0) && (h > tmp_h) ) {
                yAdvance += (float)(_h)/(float)carriage_returns;
            }
        } else {
            // misra
        }
    }

    int min_x = x_off;
    int min_y = y_off;
    int max_x = min_x+w;
    int max_y = min_y+h;

    // nema_set_blend_blit(NEMA_BL_SIMPLE);
    // fg_col = nema_premultiply_rgba(fg_col);
    set_blend(fg_col);
    nema_set_tex_color(fg_col);

    int i = 0;

    bool new_line = (alignX != NEMA_ALIGNX_LEFT) || (wrap != 0U);
    float spacing = (float)bound_font->xAdvance;
    int eol_idx = -1;

    const char *left_char_ptr = NULL;

    while (str[i] != '\0') {
        if (new_line) {
            //from this code segment we get:
            // - eol_idx
            // - spacing
            // - cur_x

            int spaces = 0; //needed for X_Justify
            int line_width; //needed for X_Align/Justify

            int rest_of_text_area_width = w-((int)cur_x-x_off);

            eol_idx = find_eol(str, i, rest_of_text_area_width, &line_width, &spaces, wrap);

            if (rest_of_text_area_width != w && line_width > rest_of_text_area_width) {
                CARRIAGE_RETURN;
                eol_idx = find_eol(str, i, w, &line_width, &spaces, wrap);
            }

            if ( alignX == NEMA_ALIGNX_JUSTIFY) {
                if ( str[eol_idx] == ' ' ) {
                    --spaces;
                }

                if (spaces > 0) {

                    int _w = w-line_width;
                    float xAdvance_f = (float)bound_font->xAdvance;
                    spacing = (float)(_w)/(float)spaces + xAdvance_f;
                    if (spacing < xAdvance_f) {
                        spacing = xAdvance_f;
                    }
                }
                cur_x = (float)x_off;
            }
            else if ( alignX == NEMA_ALIGNX_CENTER) {
                int w_ = w-line_width;
                cur_x = (float)x_off + (float)w_*0.5f;
            }
            else if ( alignX == NEMA_ALIGNX_RIGHT) {
                int cur_x_i = x_off + w - line_width;
                cur_x = (float)cur_x_i;
            }
            else {
                // misra
            }

            new_line = false;
        }

        if (str[i] == '\n' || i == eol_idx) {
            if ((int)cur_y >= max_y) {
                //reached the bottom of bbox.
                //stop rendering
                break;
            }

            CARRIAGE_RETURN;
        }
        else if ((int)cur_x < max_x) {
            //render str[i] to (cur_x, cur_y)
            if (str[i] == ' ') {
                cur_x += spacing;
            }
            else {
                bool glyph_valid = nema_get_glyph( &str[i] ) != NULL;

                if ( ((int)cur_x < (x_off+w)) && (glyph_valid) ) {
                    cur_x += (float)nema_get_kern_xoffset(left_char_ptr, &str[i]);
                    cur_x += (float)nema_print_char_internal(&str[i], (int)cur_x, (int)cur_y, min_x, min_y, max_x, max_y);
                }
            }
        }
        else {
            // misra
        }

        left_char_ptr = &str[i];
        i += utf8_codepoint_size((const uint8_t *)&str[i]);
    }

    nema_cl_add_cmd(NEMA_HOLDCMD | CL_NOP, 0);

    if (pos_x != NULL) {
        *pos_x = (int)cur_x - x_off;
    }
    if (pos_y != NULL) {
        *pos_y = (int)cur_y - y_off - (int)bound_font->max_ascender;
    }
}

void
nema_print(const char *str, int x, int y, int w, int h, uint32_t fg_col, uint32_t align) {
    nema_print_to_position(str, NULL, NULL, x, y, w, h, fg_col, align);
}

inline static int
nema_print_indexed_glyph(int id, int x, int y) {
    const nema_glyph_indexed_t *glyph = nema_get_indexed_glyph(id);

    if ( glyph == NULL ) {
        return 0;
    }

    int x_ = x + glyph->xOffset;
    int y_ = y + glyph->yOffset + (int)bound_font->max_ascender;

    uint32_t height = nema_get_indexed_glyph_height(glyph);

    int _x0 = x_;
    int _y0 = y_;
    int _x1 = x_+(int)glyph->width;
    int _y1 = y_+(int)height;

    int _w = _x1 - _x0;
    int _h = _y1 - _y0;

    if (_w > 0 && _h > 0) {
        nema_tex_format_t format;
        switch (bound_font->bpp) {
            case 1u:
                format = NEMA_BW1;
                break;
            case 2u:
                format = NEMA_A2;
                break;
            case 4u:
                format = NEMA_A4;
                break;
            default:
                format = NEMA_A8;
                break;
        }
        nema_bind_tex(NEMA_TEX1,
                      bound_font->bo.base_phys + (uint32_t) glyph->bitmapOffset,
                      glyph->width,
                      height,
                      format, -1,
                      NEMA_FILTER_PS);

        nema_set_matrix_translate((float)x_, (float)y_);
        if ( nema_context.surface_tile != 0U ) {
            nema_raster_rect(_x0, _y0, _w, _h);
        } else {
            uint32_t *cmd_array = nema_cl_get_space(3);
            if (cmd_array == NULL) {
                return 0;
            }

            cmd_array[ 0] = NEMA_DRAW_STARTXY;
            cmd_array[ 1] = YX16TOREG32(_y0, _x0);
            cmd_array[ 2] = NEMA_DRAW_ENDXY  ;
            int _y2 = _y0+_h;
            int _x2 = _x0+_w;
            cmd_array[ 3] = YX16TOREG32(_y2, _x2);
            cmd_array[ 4] = NEMA_DRAW_CMD    ;
            cmd_array[ 5] = nema_context.surface_tile | nema_context.color_grad | DRAW_BOX;
        }
    }

    uint8_t xAdvance = glyph->xAdvance;

    return (int)xAdvance;
}

void nema_print_indexed(const int *ids, int id_count, int x, int y, uint32_t fg_col) {
    int   cur_x = x;

    set_blend(fg_col);
    nema_set_tex_color(fg_col);

    for ( int i = 0; i < id_count; ++i ) {
        cur_x += nema_print_indexed_glyph(ids[i], cur_x, y);
    }
}

void nema_print_char_indexed(const int id, int x, int y, uint32_t fg_col) {
    set_blend(fg_col);
    nema_set_tex_color(fg_col);
    (void) nema_print_indexed_glyph(id, x, y);
}

void  nema_string_indexed_get_bbox(const int *ids, int id_count, int *w, int *h, int max_w){
    int max_w_ = max_w; //linting alias
    if ( bound_font == NULL ) {
        nema_set_error(NEMA_ERR_NO_BOUND_FONT);
        return;
    }

    if ( ids == NULL ) {
        return;
    }

    if (max_w_ <= 0) {
        max_w_ = 0xffffff; //MAX_INT
    }

    int w_ = 0;
    int h_ = (int)bound_font->yAdvance;

    for ( int i = 0; i < id_count; ++i ) {
        const nema_glyph_indexed_t *glyph = nema_get_indexed_glyph(ids[i]);

        if ( glyph == NULL ) {
            continue;
        }

        w_ += (int) (glyph->xOffset);
        w_ += (int) (glyph->xAdvance);


        if ( w_ > max_w_ ) {
            w_ = max_w_;
            break;
        }
    }

    *w = w_;
    *h = h_;
}

int nema_font_get_x_advance(void) {
    if ( bound_font == NULL ) {
        nema_set_error(NEMA_ERR_NO_BOUND_FONT);
        return 0;
    }

    int advance = (int) bound_font->xAdvance;

    return advance;
}
