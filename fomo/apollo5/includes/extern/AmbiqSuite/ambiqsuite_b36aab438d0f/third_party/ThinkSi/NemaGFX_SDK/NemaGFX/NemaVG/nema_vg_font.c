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
#include "nema_vg_font.h"
#include "nema_vg_context.h"
#include "nema_vg_p.h"
#include "nema_vg_path.h"
#include "nema_vg_version.h"
#include "nema_programHW.h"

#define DPI          141.f  // Default DPI
#define UNITS_PER_EM 2048.f // em units of the initial font (ttf)

#define PIXEL_TO_POINT_RATIO    (bound_font->size*DPI/(72.f*UNITS_PER_EM))

#define NEMA_VG_CHAR_XMASK     (0x01U) /**< Character orientation mask on the x-axis */
#define NEMA_VG_CHAR_YMASK     (0x02U) /**< Character orientation mask on the y-axis */

static TLS_VAR float font_scale = 1.f;
static TLS_VAR nema_vg_font_t *bound_font;

static const uint32_t UTF8_ONE_BYTE_MASK     = 0x80;
static const uint32_t UTF8_ONE_BYTE_BITS     = 0;
static const uint32_t UTF8_TWO_BYTES_MASK    = 0xE0;
static const uint32_t UTF8_TWO_BYTES_BITS    = 0xC0;
static const uint32_t UTF8_THREE_BYTES_MASK  = 0xF0;
static const uint32_t UTF8_THREE_BYTES_BITS  = 0xE0;
static const uint32_t UTF8_FOUR_BYTES_MASK   = 0xF8;
static const uint32_t UTF8_FOUR_BYTES_BITS   = 0xF0;
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

inline static const nema_vg_font_range_t * nema_get_codepoint_range(const uint32_t codepoint) {
    int range = 0;

    const nema_vg_font_range_t *range_ptr = &bound_font->ranges[range];

    while ( range_ptr->glyphs != NULL ) {
        if ( codepoint >= range_ptr->first && codepoint <= range_ptr->last ) {
            return range_ptr;
        }

        ++range;
        range_ptr = &bound_font->ranges[range];
    }

    return NULL;
}

inline static const nema_vg_glyph_t * nema_vg_get_glyph(const char *ch) {
    const uint8_t *utf8 = (const uint8_t *)ch;
    uint32_t codepoint = get_codepoint(utf8);

    if(codepoint == 0x00000000U ){
        return NULL;
    }

    const nema_vg_font_range_t *range_ptr = nema_get_codepoint_range(codepoint);
    if (range_ptr == NULL) {
        return NULL;
    }

    const nema_vg_glyph_t *glyph = &range_ptr->glyphs[ codepoint - range_ptr->first ];

    return glyph;
}

void nema_vg_bind_font(nema_vg_font_t *font) {
    if ( font != NULL ) {
        const uint32_t api_font_version = NEMA_VG_FONT_VERSION;
        const uint32_t struct_version   = font->version;

        if ( api_font_version != struct_version ) {
            nema_vg_set_error(NEMA_VG_ERR_UNSUPPORTED_FONT);
            bound_font = NULL;
        }
        else{
            bound_font = font;
        }
    }
    else{
        bound_font = NULL;
        nema_vg_set_error(NEMA_VG_ERR_NO_BOUND_FONT);
    }
}

inline static float nema_vg_get_glyph_xadvance(const nema_vg_glyph_t *glyph) {
    float x_advance = 0.f;

    if ( glyph != NULL ) {
        x_advance = (float) glyph->xAdvance;
    } else {
        x_advance = (float) bound_font->xAdvance;
    }

    return font_scale*x_advance;
}

inline static float nema_vg_get_kern_xoffset(const char *left, const char *right) {

    if ( (left == NULL) || (right == NULL) || (bound_font == NULL) || (bound_font->kern_pairs == NULL) ) {
        return 0.f;
    }

    const uint8_t *utf8_left         = (const uint8_t *) left;
    uint32_t codepoint_left          = get_codepoint(utf8_left);
    const nema_vg_glyph_t *cur_glyph = nema_vg_get_glyph( (const char*)right );

    //if kerning is available, return the horizontal offset
    if ( (cur_glyph != NULL) && ((int)cur_glyph->kern_length > 0) ) {
        uint8_t i;
        for ( i = 0; i < cur_glyph->kern_length; ++i ) {
            if ( codepoint_left == bound_font->kern_pairs[cur_glyph->kern_offset + i].left ) {
                return (font_scale*bound_font->kern_pairs[cur_glyph->kern_offset + i].x_offset  - 0.5f); //offset is negative, thus -0.5f
            }
        }
    }

    return 0.f;
}

// find end of word
// a word ends with '\0', ' ' or '\n'
// returns the index to that ending element
static inline int find_eow(const char *str, int i, float *word_width) {
    float cur_w = 0.f;
    int      i_ = i;

    const char *left_char_ptr = NULL;
    // if the word starts with an ending element
    // it counts as a word, return immediately
    // width should be zero in that case
    while ( str[i_] != '\0' && str[i_] != ' ' && str[i_] != '\n') {
        if ( str[i_] != ' ' ) {
            const nema_vg_glyph_t *glyph = nema_vg_get_glyph(&str[i_]);
            cur_w += nema_vg_get_glyph_xadvance(glyph);
            cur_w += nema_vg_get_kern_xoffset(left_char_ptr, &str[i_]);

            left_char_ptr = &str[i_];
            i_ += utf8_codepoint_size((const uint8_t *)&str[i_]);
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
static inline int find_eol(const char *str, int i, float max_w, float *line_width, int *const space_count, uint32_t wrap) {
    float width = 0.f;
    int i_ = i;
    //do wrapping only if both wrap is enabled and max_w is > 0
    bool do_wrap = wrap != 0U && max_w > 0.f;
    int spaces = 0;

    if ( str[i_] == ' ' ) {
        ++spaces;
    }

    // while not the end of the string
    while ( (str[i_] != '\0') ) {
        float word_width = 0.f;

        // eow_idx always points to '\0', ' ', '\n'
        int eow_idx = find_eow(str, i_, &word_width);

        // if wrapping is enabled
        // and at least one word has been written
        // and we've exceeded max_w
        // line has ended
        if ( do_wrap &&
             width > 0.f &&
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
            width += font_scale*bound_font->xAdvance;
        } else if ( str[i_] == '\n' ) {
            //carriage return
            //NEW LINE

            break;
        } else if ( str[i_] == '\0' ) {
            //end of string
            //NEW LINE

            break;
        } else {
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

int nema_vg_string_get_bbox(const char *str, float *w, float *h, float max_w, uint32_t wrap) {
    if ( bound_font == NULL ) {
        nema_vg_set_error(NEMA_VG_ERR_NO_BOUND_FONT);
        return -1;
    }
    float max_w_ = max_w;

    if ( str == NULL ) {
        return -1;
    }

    if (max_w_ <= 0.f) {
        max_w_ = 2147483647.f; //MAX_FLOAT
    }

    float h_f = font_scale*bound_font->size; //cast for MISRA
    *w = 0.f;
    *h = h_f; //size <=> height

    float bbox_w         = 0.f;
    int i                = 0;
    int carriage_returns = 0;


    while (str[i] != '\0') {
        float line_width = 0.f;
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

    *w  = bbox_w;
    *h *= (float)carriage_returns;

    return carriage_returns;
}

static void draw_glyph(const nema_vg_glyph_t *glyph, NEMA_VG_PATH_HANDLE path, NEMA_VG_PAINT_HANDLE paint, float x, float y, nema_matrix3x3_t m) {
    const nema_vg_float_t bbox[4] = { (nema_vg_float_t) glyph->bbox_xmin, (nema_vg_float_t) glyph->bbox_ymin,
                                      (nema_vg_float_t) glyph->bbox_xmax, (nema_vg_float_t) glyph->bbox_ymax };

    if ( glyph->segment_length == 0 ) {
        return;
    }

    nema_vg_path_set_shape_and_bbox( (void*)path, glyph->segment_length, &(bound_font->segment[glyph->segment_offset]),
                                        glyph->data_length, &(bound_font->data[glyph->data_offset]), bbox );

    nema_matrix3x3_t m_path;
    nema_mat3x3_load_identity(m_path);
    nema_mat3x3_scale(m_path, font_scale, -font_scale);
    nema_mat3x3_translate(m_path, x , y);

    if ( m != NULL ) {
        nema_matrix3x3_t buffer_mat;

        buffer_mat[0][0] = m[0][0];
        buffer_mat[0][1] = m[0][1];
        buffer_mat[0][2] = m[0][2];
        buffer_mat[1][0] = m[1][0];
        buffer_mat[1][1] = m[1][1];
        buffer_mat[1][2] = m[1][2];
        buffer_mat[2][0] = m[2][0];
        buffer_mat[2][1] = m[2][1];
        buffer_mat[2][2] = m[2][2];

        nema_mat3x3_mul(buffer_mat, m_path);
        nema_vg_path_set_matrix(path, buffer_mat);
    } else {
        nema_vg_path_set_matrix(path, m_path);
    }

    (void)nema_vg_draw_path((void*)path, (void*)paint);
}

static inline float nema_vg_print_char_internal(NEMA_VG_PATH_HANDLE path, NEMA_VG_PAINT_HANDLE paint, const char *ch, float x, float y, float x_max, float y_max, nema_matrix3x3_t m) {
    if ( (x >= x_max) || (y >= y_max)) {
        return 0.f;
    }

    if (*ch == ' ') {
        return (font_scale*bound_font->xAdvance);
    }

    const nema_vg_glyph_t *glyph = nema_vg_get_glyph(ch);
    if ( glyph != NULL ) {
        draw_glyph(glyph, path, paint, x, y, m);
        return (glyph->xAdvance*font_scale);
    } else {
        return (bound_font->xAdvance*font_scale);
    }
}

#define VG_CARRIAGE_RETURN                                               \
            new_line = (alignX != NEMA_VG_ALIGNX_LEFT) || (wrap != 0U);  \
            cur_x = (float)x_off;                                        \
            cur_y += yAdvance;                                           \
            if ( cur_y >= (float)max_y + (font_scale*bound_font->ascender)) { \
                break;                                                   \
            }


void nema_vg_print(NEMA_VG_PAINT_HANDLE paint, const char *str, float x, float y, float w, float h, uint32_t align, nema_matrix3x3_t m) {
    if ( bound_font == NULL ) {
        nema_vg_set_error(NEMA_VG_ERR_NO_BOUND_FONT);
        return;
    }

    if ( (str == NULL) || (nema_float_is_zero(font_scale) ) ) {
        return;
    }

    NEMA_VG_PATH_HANDLE path = nema_vg_path_create();

    float ascent = font_scale*bound_font->ascender;
    //just aliases
    float x_off = x;
    float y_off = y;

    float cur_x           = x_off;
    float cur_y           = y_off + ascent;
    float yAdvance        = font_scale*bound_font->size + 0.5f;
    const uint32_t wrap   = (align & NEMA_VG_TEXT_WRAP);
    const uint32_t alignX = (align & NEMA_VG_ALIGNX_MASK);
    const uint32_t alignY = (align & NEMA_VG_ALIGNY_MASK);

#ifndef NEMAPVG
#if (P_FORCE_EVEN_ODD != 0)
    uint8_t fill_rule_prev = get_fill_rule();
    if ( fill_rule_prev == NEMA_VG_FILL_NON_ZERO ) {
        nema_vg_set_fill_rule(NEMA_VG_FILL_EVEN_ODD);
    }
#endif //P_FORCE_EVEN_ODD
#endif //NEMAPVG

    enable_screen_space_stroking();
    enable_use_bevel_joins(1);

    if ( alignY != NEMA_VG_ALIGNY_TOP ) {
        float tmp_w, tmp_h;
        int carriage_returns = nema_vg_string_get_bbox(str, &tmp_w, &tmp_h, w, wrap);
        float _h = h - tmp_h;

        if(alignY == NEMA_VG_ALIGNY_BOTTOM) {
            cur_y += (float)_h;
        } else if (alignY == NEMA_VG_ALIGNY_CENTER) {
            cur_y += ((float)_h)*0.5f;
        } else if (alignY == NEMA_VG_ALIGNY_JUSTIFY) {
            if ( (carriage_returns > 0) && (h > tmp_h) ) {
                yAdvance += _h/(float)carriage_returns;
            }
        } else {
            // misra
        }
    }

    float min_x   = x_off;
    float min_y   = y_off;
    float max_x   = min_x+w;
    float max_y   = min_y+h;
    int i         = 0;
    bool new_line = (alignX != NEMA_VG_ALIGNX_LEFT) || (wrap != 0U);
    float spacing = (float)(font_scale*bound_font->xAdvance);
    int eol_idx   = -1;

    const char *left_char_ptr = NULL;

    while (str[i] != '\0') {

        if (new_line) {
            //from this code segment we get:
            // - eol_idx
            // - spacing
            // - cur_x

            int spaces       = 0;   //needed for X_Justify
            float line_width = 0.f; //needed for X_Align/Justify

            float rest_of_text_area_width = w-(cur_x-x_off);

            eol_idx = find_eol(str, i, rest_of_text_area_width, &line_width, &spaces, wrap);

            if ( !nema_floats_equal(rest_of_text_area_width, w) && (line_width > rest_of_text_area_width) ) {
                VG_CARRIAGE_RETURN;
                eol_idx = find_eol(str, i, w, &line_width, &spaces, wrap);
            }

            if ( alignX == NEMA_VG_ALIGNX_JUSTIFY) {
                if ( str[eol_idx] == ' ' ) {
                    --spaces;
                }

                if (spaces > 0) {

                    float _w = w-line_width;
                    float xAdvance_f = (float)(font_scale*bound_font->xAdvance);
                    spacing = (float)(_w)/(float)spaces + xAdvance_f;
                    if (spacing < xAdvance_f) {
                        spacing = xAdvance_f;
                    }
                }
                cur_x = (float)x_off;
            } else if ( alignX == NEMA_VG_ALIGNX_CENTER) {
                float w_ = w-line_width;
                cur_x = x_off + w_*0.5f;
            } else if ( alignX == NEMA_VG_ALIGNX_RIGHT) {
                cur_x = x_off + w - line_width;
            } else {
                // misra
            }

            new_line = false;
        }

        if (str[i] == '\n' || i == eol_idx) {
            if ( cur_y >= max_y) {
                //reached the bottom of bbox.
                //stop rendering
                break;
            }

            VG_CARRIAGE_RETURN;
        } else if (cur_x < max_x) {
                if (str[i] == ' ') {
                    cur_x += spacing;
                } else {
                    const nema_vg_glyph_t *glyph = nema_vg_get_glyph( &str[i] );
                    bool glyph_valid = glyph != NULL;

                    if ( (cur_x < (x_off+w)) && (glyph_valid) ) {

                        cur_x += nema_vg_get_kern_xoffset(left_char_ptr, &str[i]);
                        cur_x += nema_vg_print_char_internal(path, paint, &str[i], cur_x, cur_y, max_x, max_y, m);
                    }
                }
        } else {
            // misra
        }

        left_char_ptr = &str[i];
        i += utf8_codepoint_size((const uint8_t *)&str[i]);
    }

    disable_screen_space_stroking();
    enable_use_bevel_joins(0);
#ifndef NEMAPVG
#if (P_FORCE_EVEN_ODD != 0)
    nema_vg_set_fill_rule(fill_rule_prev);
#endif //P_FORCE_EVEN_ODD
#endif //NEMAPVG

    nema_vg_path_destroy(path);
}

void nema_vg_set_font_size(float size) {
    if ( bound_font == NULL ) {
        nema_vg_set_error(NEMA_VG_ERR_NO_BOUND_FONT);
        return;
    }

    font_scale = size*(16.f/12.f)/UNITS_PER_EM; //16px <=> 12pt <=> 1em	<=> 100%
}

int nema_vg_get_ascender_pt(void) {
    int ret = 0;

    if ( bound_font != NULL ) {
        float ascent = font_scale*bound_font->ascender;
        ret = (int) (ascent);
    }

    return ret;
}

float nema_vg_print_char(NEMA_VG_PAINT_HANDLE paint, char ch, float x, float y, nema_matrix3x3_t m, uint32_t orientation) {
    if (ch == ' ') {
        return (font_scale*bound_font->xAdvance);
    }

    const nema_vg_glyph_t *glyph = nema_vg_get_glyph(&ch);
    if ( glyph != NULL ) {
        NEMA_VG_PATH_HANDLE path = nema_vg_path_create();
        const uint32_t orientationX = (orientation & NEMA_VG_CHAR_XMASK);
        const uint32_t orientationY = (orientation & NEMA_VG_CHAR_YMASK);

        //aliases
        float x_off = x;
        float y_off = y;

        if ( orientationX == NEMA_VG_CHAR_RTL ) {
            x_off -= glyph->xAdvance*font_scale;
        }

        if ( orientationY == NEMA_VG_CHAR_BTT ) {
            y_off -= font_scale*bound_font->size;
        }

        draw_glyph(glyph, path, paint, x_off, y_off, m);
        nema_vg_path_destroy(path);
        return (glyph->xAdvance*font_scale);
    } else {
        return (bound_font->xAdvance*font_scale);
    }
}

nema_font_t* nema_vg_generate_raster_font(int size, int pool) {
    //initial values that need to be reset on function exit
    int32_t  clip_x = nema_context.prev_clip_xy[0];
    int32_t  clip_y = nema_context.prev_clip_xy[1];
    uint32_t clip_w = nema_context.prev_clip_wh[0];
    uint32_t clip_h = nema_context.prev_clip_wh[1];

    uintptr_t fb_base_phys = (uintptr_t) nema_context.texs[NEMA_TEX0].base;
    int32_t   fb_w         = nema_context.texs[NEMA_TEX0].w;
    int32_t   fb_h         = nema_context.texs[NEMA_TEX0].h;
    int32_t   fb_format    = nema_context.texs[NEMA_TEX0].format;
    int32_t   fb_stride    = nema_context.texs[NEMA_TEX0].fstride;

    nema_vg_set_font_size(size);

    //1.Kern pairs
    nema_vg_kern_pair_t *kern_pair_vg = (nema_vg_kern_pair_t*) bound_font->kern_pairs;
    int kern_pairs_count = 0;

    if ( kern_pair_vg != NULL ) {
        while ( kern_pair_vg->left != 0 ) {
            kern_pairs_count += 1;
            kern_pair_vg++;
        }
    }

    nema_kern_pair_t *kern_pairs_raster = NULL;

    if ( kern_pairs_count > 0 ) {
        kern_pairs_raster = (nema_kern_pair_t*) nema_host_malloc((kern_pairs_count+1)*sizeof(nema_kern_pair_t));

        if ( kern_pairs_raster == NULL ) {
            return NULL;
        }

        for ( int i = 0; i < kern_pairs_count; ++i ) {
            kern_pairs_raster[i].left     = bound_font->kern_pairs[i].left;
            kern_pairs_raster[i].x_offset = (int8_t) (font_scale*(bound_font->kern_pairs[i].x_offset) - 0.5f);
        }

        //Last pair contains explicit zero values
        kern_pairs_raster[kern_pairs_count].left     = 0;
        kern_pairs_raster[kern_pairs_count].x_offset = 0;
    }

    //2. Glyphs data
    nema_vg_font_range_t *range = (nema_vg_font_range_t*) bound_font->ranges;
    int range_count = 0;

    if ( range != NULL ) {
        while( range->glyphs != NULL ){
            range_count += 1;
            range++;
        }
    }

    const int ascent      = (const int) (font_scale*bound_font->ascender + 0.5f);
    const int w_margin    = 2; //2 pixels margin for AA

    int font_height = (const int) (font_scale*bound_font->size + 0.5f);
    font_height     = ((font_height+3)/4)*4; //make it multiple of 4

    if ( font_height > fb_h) {
        return NULL;
    }

    range = (nema_vg_font_range_t*) bound_font->ranges;
    int bitmaps_size = 0;
    int glyph_count  = 0;

    //Calculate the buffer size for the bitmaps
    while ( range->glyphs != NULL ) {
        int range_glyphs_ = range->last - range->first + 1;

        for ( int glyph_idx = 0; glyph_idx < range_glyphs_; ++glyph_idx ) {
            nema_vg_glyph_t *glyph = (nema_vg_glyph_t*) &range->glyphs[glyph_idx];
            int glyph_w = (int) (font_scale*(glyph->bbox_xmax - glyph->bbox_xmin)) + w_margin;
            bitmaps_size += glyph_w*font_height;
            glyph_count++;
        }
        range++;
    }

    int bitmaps_bo_width = bitmaps_size/font_height;
    bitmaps_bo_width     =  ((bitmaps_bo_width+3)/4)*4;

    bitmaps_size = bitmaps_bo_width*font_height;

    nema_buffer_t bitmaps_bo = nema_buffer_create_pool(pool, bitmaps_size);

    if ( bitmaps_bo.base_phys == 0U ) {
        return NULL;
    }

    nema_set_blend_fill(NEMA_BL_SRC);

    //If the buffer is very large, clear multiple chunks of (2048 x font_height)
    const int max_width = 2048;
    int clear_count     = bitmaps_bo_width/max_width;
    int clear_remainder = (int) nema_fmod((float) bitmaps_bo_width, (float) max_width);

    for ( int k = 0; k < clear_count; ++k) {
        uintptr_t base_addr = bitmaps_bo.base_phys + k*max_width*font_height;
        int width  = max_width/4;
        int height = font_height/4;
        nema_bind_dst_tex(base_addr, width, height, NEMA_RGBA8888, max_width);
        nema_set_clip (0, 0, max_width, font_height);
        nema_fill_rect(0, 0, max_width, font_height, 0);
    }

    if ( clear_remainder > 0 ) {
        uintptr_t base_addr = bitmaps_bo.base_phys + clear_count*max_width*font_height;
        int width  = clear_remainder/4;
        int height = font_height/4;
        nema_bind_dst_tex(base_addr, width, height, NEMA_RGBA8888, width);
        nema_set_clip (0, 0, clear_remainder, font_height);
        nema_fill_rect(0, 0, clear_remainder, font_height, 0);
    }

    NEMA_VG_PATH_HANDLE path   = nema_vg_path_create();
    NEMA_VG_PAINT_HANDLE paint = nema_vg_paint_create();

    nema_vg_paint_set_type(paint, NEMA_VG_PAINT_COLOR);
    nema_vg_paint_set_paint_color(paint, 0xffffffffU);

    uint8_t fill_rule_prev = get_fill_rule();
    nema_vg_set_fill_rule(NEMA_VG_FILL_NON_ZERO);

#ifndef NEMAPVG
#if (P_FORCE_EVEN_ODD != 0)
        if ( fill_rule_prev == NEMA_VG_FILL_NON_ZERO ) {
            nema_vg_set_fill_rule(NEMA_VG_FILL_EVEN_ODD);
        }
#endif //P_FORCE_EVEN_ODD
#endif //NEMAPVG

    nema_vg_set_quality(NEMA_VG_QUALITY_MAXIMUM);
    nema_vg_set_blend(NEMA_BL_SRC_OVER);

    range = (nema_vg_font_range_t*) bound_font->ranges;
    nema_font_range_t *ranges_raster = (nema_font_range_t*) nema_host_malloc((range_count+1)*sizeof(nema_font_range_t));

    if ( ranges_raster == NULL ) {
        return NULL;
    }

    uintptr_t glyph_addr = bitmaps_bo.base_phys;

    int range_idx = 0;
    while ( range->glyphs != NULL ) {
        int glyphs_in_range = range->last - range->first + 1;
        nema_glyph_t *glyphs_raster = (nema_glyph_t*) nema_host_malloc((glyphs_in_range + 1)*sizeof(nema_glyph_t));

        if ( glyphs_raster == NULL ) {
            return NULL;
        }

        for ( int glyph_idx = 0; glyph_idx < glyphs_in_range; ++glyph_idx ) {
            nema_vg_glyph_t *glyph = (nema_vg_glyph_t*) &range->glyphs[glyph_idx];
            int glyph_w = (int) (font_scale*(glyph->bbox_xmax - glyph->bbox_xmin) ) + w_margin;
            int glyph_h = font_height;

            //draw glyph bitmap
            nema_bind_dst_tex(glyph_addr, glyph_w, glyph_h, NEMA_A8, glyph_w);
            nema_set_clip(0, 0, glyph_w, glyph_h);
            float x_centered = -font_scale*glyph->bbox_xmin + 0.5f*w_margin; //glyph must be centered according to bbox
            draw_glyph(glyph, path, paint, x_centered, (float) ascent, NULL);

            glyphs_raster[glyph_idx].bitmapOffset = glyph_addr - bitmaps_bo.base_phys;
            glyphs_raster[glyph_idx].width        = glyph_w;
            glyphs_raster[glyph_idx].xAdvance     = (int) (font_scale*(glyph->xAdvance)  + 0.5f);
            glyphs_raster[glyph_idx].xOffset      = (int) (font_scale*(glyph->bbox_xmin) + 0.5f);
            glyphs_raster[glyph_idx].yOffset      = 0;
            glyphs_raster[glyph_idx].kern_offset  = (uint32_t) glyph->kern_offset;
            glyphs_raster[glyph_idx].kern_length  = (uint8_t)  glyph->kern_length;

            glyph_addr += glyph_w*glyph_h;
        }

        glyphs_raster[glyphs_in_range].bitmapOffset = glyph_addr - bitmaps_bo.base_phys;
        glyphs_raster[glyphs_in_range].width        = 0U;
        glyphs_raster[glyphs_in_range].xAdvance     = 0U;
        glyphs_raster[glyphs_in_range].xOffset      = 0;
        glyphs_raster[glyphs_in_range].yOffset      = 0;
        glyphs_raster[glyphs_in_range].kern_offset  = 0U;
        glyphs_raster[glyphs_in_range].kern_length  = 0U;

        ranges_raster[range_idx].first  = range->first;
        ranges_raster[range_idx].last   = range->last;
        ranges_raster[range_idx].glyphs = glyphs_raster;

        range_idx += 1;
        range++;
    }

    //Last pair contains explicit zero values
    ranges_raster[range_count].first  = 0U;
    ranges_raster[range_count].last   = 0U;
    ranges_raster[range_count].glyphs = NULL;

    nema_vg_paint_destroy(paint);
    nema_vg_path_destroy(path);

    nema_font_t *raster_font = (nema_font_t*) nema_host_malloc(sizeof(nema_font_t));

    if ( raster_font == NULL ) {
        return NULL;
    }

    raster_font->bo.base_phys   = bitmaps_bo.base_phys;
    raster_font->bo.base_virt   = bitmaps_bo.base_virt;
    raster_font->bo.size        = bitmaps_bo.size;
    raster_font->ranges         = ranges_raster;
    raster_font->flags          = 0U;
    raster_font->xAdvance       = font_scale*bound_font->xAdvance;
    raster_font->yAdvance       = font_height;
    raster_font->max_ascender   = 0U;
    raster_font->bpp            = 8U;
    raster_font->kern_pairs     = kern_pairs_raster;
    raster_font->indexed_glyphs = NULL;

    //reset framebuffer, clip, fill rule
    nema_bind_dst_tex(fb_base_phys, fb_w, fb_h, fb_format, fb_stride);
    nema_set_clip(clip_x, clip_y, clip_w, clip_h);
    nema_vg_set_fill_rule(fill_rule_prev);

    return raster_font;
}


void nema_vg_destroy_raster_font(nema_font_t *font) {
    if ( font != NULL ) {
        //bitmaps
        if ( font->bo.base_phys != 0U ) {
            nema_buffer_destroy(&(font->bo));
        }

        //glyphs
        nema_font_range_t *range = (nema_font_range_t*) font->ranges;
        if ( range != NULL ) {
            while( range->glyphs != NULL ) {
                nema_host_free( (void*) range->glyphs);
                range++;
            }
        }

        //ranges
        if ( font->ranges != NULL ) {
            nema_host_free((void*) font->ranges);
        }

        //kern pairs
        if ( font->kern_pairs != NULL ) {
            nema_host_free((void*) font->kern_pairs);
        }

        //font
        nema_host_free((void*) font);
    }
}
