//*****************************************************************************
//
//! @file lv_ambiq_font_align.c
//!
//! @brief Ambiq font realignment.
//!
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#include "lv_ambiq_font_align.h"

//*****************************************************************************
//
// Global Variables
//
//*****************************************************************************
//! A structure to process A4 format font.
union low_high_uint8{
    struct{
        uint8_t low : 4;
        uint8_t high: 4;
    } half;
    uint8_t data;
};

//*****************************************************************************
//
//! @brief Convert each letter in a font.
//!
//! @param width - glyph width in pixel.
//! @param hight - glyph hight in pixel.
//! @param src - glyph source address.
//! @param des - glyph destination address.
//!
//! @return the bitmap advance in byte.
//
//*****************************************************************************
static int convert_letter(uint32_t width, uint32_t hight, uint8_t* src, uint8_t* des)
{
    uint32_t count_src = 0;
    uint32_t count_des = 0;
    uint32_t i;
    uint32_t j;
    uint32_t value;

    uint32_t byte_offset;
    uint32_t bit_offset;
    union low_high_uint8 value_byte;

    for(i=0; i<hight; i++)
    {
        for(j=0; j<width; j++)
        {
            byte_offset = count_src/2;
            bit_offset = count_src%2;
            value_byte.data = src[byte_offset];

            value = (bit_offset == 0) ? (value_byte.half.high) : (value_byte.half.low);

            byte_offset = count_des/2;
            bit_offset = count_des%2;
            value_byte.data = des[byte_offset];

            if(bit_offset)
            {
                value_byte.half.low = value;
            }
            else
            {
                value_byte.half.high = value;
            }

            des[byte_offset] = value_byte.data;

            count_des++;
            count_src++;

        }
        if(width%2)
            count_des++;
    }

    return (count_des + 1)/2;
}

//*****************************************************************************
//
//! @brief Get the font bitmap length after alignment.
//!
//! @param font - Font to be align.
//!
//! @return the bitmap length after alignment.
//
//*****************************************************************************
uint32_t lv_ambiq_bitmap_length_get(const lv_font_fmt_txt_dsc_t* font)
{
    uint32_t i;
    uint32_t j;
    const lv_font_fmt_txt_cmap_t * cmaps_array = font->cmaps;
    uint32_t offset_des = 0;

    if(font->bpp != 4)
        return 0;

    for(i=0; i<font->cmap_num; i++)
    {
        uint32_t camp_letter_total;
        uint32_t camp_start;

        if((cmaps_array[i].type == LV_FONT_FMT_TXT_CMAP_FORMAT0_FULL) || 
        (cmaps_array[i].type == LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY))
        {
            camp_letter_total = cmaps_array[i].range_length;
        }
        else
        {
            camp_letter_total = cmaps_array[i].list_length;
        }

        camp_start = cmaps_array[i].glyph_id_start;

        for(j=0; j<camp_letter_total; j++)
        {
            lv_font_fmt_txt_glyph_dsc_t* glyph = &font->glyph_dsc[camp_start + j];

            uint32_t bitmap_advance_des = ((glyph->box_w * font->bpp + 7)/8)*glyph->box_h;
            offset_des += bitmap_advance_des;
        }
    }

    return offset_des;
}

//*****************************************************************************
//
//! @brief Align the A4 format font bitmap
//!
//! @param font - Font to be align.
//! @param new_bitmap_addr - a buffer to hold the aligned bitmap, 
//!                         call lv_ambiq_bitmap_length_get to get the its length .
//!
//! @note This function will modify the glyph_bitmap pointer to the input new_bitmap_addr.
//!
//! @return None.
//
//*****************************************************************************
void lv_ambiq_A4_font_align(lv_font_fmt_txt_dsc_t* font, uint8_t* new_bitmap_addr)
{
    uint32_t i;
    uint32_t j;
    const lv_font_fmt_txt_cmap_t * cmaps_array = font->cmaps;
    uint32_t offset_des = 0;
    uint32_t offset_src;
    const uint8_t* src = font->glyph_bitmap;
    uint8_t* des = new_bitmap_addr;
    lv_font_fmt_txt_glyph_dsc_t* glyph;

    if(font->bpp != 4)
        return;

    for(i=0; i<font->cmap_num; i++)
    {
        uint32_t camp_letter_total;
        uint32_t camp_start;

        if((cmaps_array[i].type == LV_FONT_FMT_TXT_CMAP_FORMAT0_FULL) || 
        (cmaps_array[i].type == LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY))
        {
            camp_letter_total = cmaps_array[i].range_length;
        }
        else
        {
            camp_letter_total = cmaps_array[i].list_length;
        }

        camp_start = cmaps_array[i].glyph_id_start;

        for(j=0; j<camp_letter_total; j++)
        {
            glyph = &font->glyph_dsc[camp_start + j];
            offset_src = glyph->bitmap_index;
            uint32_t bitmap_advance_des;

            glyph->bitmap_index = offset_des;
            bitmap_advance_des = convert_letter(glyph->box_w, glyph->box_h, (uint8_t*)&src[offset_src], &des[offset_des]);
            offset_des += bitmap_advance_des;
        }
    }
    font->glyph_bitmap = des;
}
