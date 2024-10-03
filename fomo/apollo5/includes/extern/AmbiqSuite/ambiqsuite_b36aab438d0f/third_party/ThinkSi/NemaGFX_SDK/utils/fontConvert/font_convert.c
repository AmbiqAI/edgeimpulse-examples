/* TSI 2023.xmp */
/*******************************************************************************
 * Copyright (c) 2023 Think Silicon Single Member PC
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this file and/or associated documentation files to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies of the
 * Materials, and to permit persons to whom the Materials are furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Materials.
 *
 * The software is provided 'as is', without warranty of any kind, express or
 * implied, including but not limited to the warranties of merchantability,
 * fitness for a particular purpose and noninfringement. In no event shall Think
 * Silicon Single Member PC be liable for any claim, damages or other liability,
 * whether in an action of contract, tort or otherwise, arising from, out of or
 * in connection with the software or the use or other dealings in the software.
 ******************************************************************************/

/*

Derived from "TrueType to Adafruit_GFX font converter".
*/

#include <stdio.h>
#include <ctype.h>
#include <stdint.h>
#include <ft2build.h>
#include <math.h>

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

#include FT_GLYPH_H

#define DPI 141

#define CLAMP(x, low, high) ( (x) < (low) ? (low) : ( (x) > (high) ? (high) : (x) ) )

FILE *dot_c_file;

typedef struct {
    uint32_t bitmapOffset;
    uint8_t  width, height;
    uint8_t  xAdvance;
    int8_t   xOffset, yOffset;
    uint32_t kern_offset;
    uint8_t  kern_length;
} nema_glyph_t;

typedef struct {
    int      bitmapOffset;
    uint8_t  width;
    uint8_t  xAdvance;
    int8_t   xOffset;
    int8_t   yOffset;
    int      id;
} nema_glyph_indexed_t;

typedef struct kern_pair_t_{
   uint32_t left;
   uint32_t right;
   int x_offset;
   struct kern_pair_t_ *next;
}kern_pair_t;

static kern_pair_t *kern_list_head = NULL;
static kern_pair_t *kern_list_cur  = NULL;

void insert_kern_pair(uint32_t left, uint32_t right, int x_offset) {
    kern_pair_t *pair = (kern_pair_t*) malloc(sizeof(kern_pair_t));
    pair->left     = left;
    pair->right    = right;
    pair->x_offset = x_offset;
    pair->next     = NULL;

    if ( kern_list_head == NULL ) {
        kern_list_head = pair;
        kern_list_cur  = pair;
    } else {
        kern_list_cur->next = pair;
        kern_list_cur       = pair;
    }
}

void kern_list_clear(){
    kern_pair_t *pair = kern_list_head;
    while (kern_list_head != NULL){
        kern_list_head = kern_list_head->next;
        free(pair);
        pair = kern_list_head;
    }
}

static uint8_t firstCall = 1;

static int GPUmem_count         = 0;
static int CPUmem_count32       = 0;
static int CPUmem_text_count32  = 0;
static int CPUmem_data_count32  = 0;
static int CPUmem_count64       = 0;
static int CPUmem_text_count64  = 0;
static int CPUmem_data_count64  = 0;
static int KernMem              = 0;

static int nema_kern_pair_t_size    = 8;
static int nema_glyph_t_size        = 16;
static int nema_glyph_indexed_t_size= 12;
static int nema_font_range_t_size32 = 12;
static int nema_font_range_t_size64 = 16;
static int nema_font_t_size32       = 44;
static int nema_font_t_size64       = 72;


static void write_pix(uint8_t value) {
    static uint8_t row = 0;

    if (firstCall != 0) {
        row = 0;
    }

    if(!firstCall) { // Format output table nicely
        if(++row >= 12) {        // Last entry on line?
            fprintf(dot_c_file, ",\n  "); //   Newline format output
            row = 0;         //   Reset row counter
        } else {                 // Not end of line
            fprintf(dot_c_file, ", ");    //   Simple comma delim
        }
    }
    fprintf(dot_c_file, "0x%02X", value); // Write byte value
    GPUmem_count++;
    firstCall = 0;         // Formatting flag
}

static FILE *create_file(const char *filename) {
    FILE * pFile;
    pFile = fopen (filename, "w+");
    if (pFile == NULL) {
        fprintf(dot_c_file, "Cannot open file %s\n", filename);
        return NULL;
    }

    return pFile;
}

static void append_file(FILE *dst, FILE *src) {
    char ch;

    if ( fseek(src, 0L, SEEK_SET) != 0 ) {

    }

    while((ch=fgetc(src))!=EOF)
    {
        // printf("%c", ch);
        fputc(ch, dst);
    }

    fflush(dst);
}

static void
str_to_upper(char *d_str, char *s_str, int str_len) {
    int idx = 0;
    for (idx = 0; idx < str_len; idx++)
    {
        d_str[idx] = toupper(s_str[idx]);
    }
    d_str[idx] = '\0';
}

static int write_bitmap(FILE *pFile, FT_Bitmap *bitmap, int bpp) {
    int offset = 0;

    #define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
    #define BYTE_TO_BINARY(byte)  \
    (byte & 0x80 ? '#' : '.'), \
    (byte & 0x40 ? '#' : '.'), \
    (byte & 0x20 ? '#' : '.'), \
    (byte & 0x10 ? '#' : '.'), \
    (byte & 0x08 ? '#' : '.'), \
    (byte & 0x04 ? '#' : '.'), \
    (byte & 0x02 ? '#' : '.'), \
    (byte & 0x01 ? '#' : '.')

    if (bpp == 1) {
        for(int y=0; y < bitmap->rows; y++) {
            for (int x = 0; x < (bitmap->width*bpp+7)/8; ++x) {
                // printf( BYTE_TO_BINARY_PATTERN, BYTE_TO_BINARY(bitmap->buffer[y * bitmap->pitch + x]) );
                write_pix(bitmap->buffer[y * bitmap->pitch + x]);
                fwrite (&bitmap->buffer[y * bitmap->pitch + x] , sizeof(char), 1, pFile);
                ++offset;
            }
            // printf("\n");
        }
        // printf("\n");
    } else {
        int mul = 8/bpp;
        for(int y=0; y < bitmap->rows; y++) {
            uint8_t c;
            for (int x = 0; x < (bitmap->width*bpp+7)/8; ++x) {
                c = bitmap->buffer[y * bitmap->pitch + x*mul];
                if (bpp == 4) {
                    uint8_t c0 = c;
                    uint8_t c1 = bitmap->buffer[y * bitmap->pitch + x*mul+1];
                    c = (c0 & 0xf0) | (c1 >> 4);
                }
                if (bpp == 2) {
                    uint8_t c0 = c;
                    uint8_t c1 = bitmap->buffer[y * bitmap->pitch + x*mul+1];
                    uint8_t c2 = bitmap->buffer[y * bitmap->pitch + x*mul+2];
                    uint8_t c3 = bitmap->buffer[y * bitmap->pitch + x*mul+3];
                    c  = (c0 & 0xc0)>>0;
                    c |= (c1 & 0xc0)>>2;
                    c |= (c2 & 0xc0)>>4;
                    c |= (c3 & 0xc0)>>6;
                }
                // printf( BYTE_TO_BINARY_PATTERN, BYTE_TO_BINARY(c) );
                write_pix(c);
                fwrite (&c, sizeof(char), 1, pFile);
                ++offset;
            }
            // printf("\n");
        }
        // printf("\n");
    }

    return offset;
}

int font_convert(char *fontfile, int size, int bpp, int *range_start, int *range_end, int range_count, int kerning, int *glyph_id, int glyph_id_count) {
    int                i, j, err, bitmapOffset = 0;
    char               c;
    FT_Library         library;
    FT_Face            face;
    FT_Glyph           glyph;
    FT_Bitmap         *bitmap;
    FT_BitmapGlyphRec *g;

    firstCall = 1;

    char *sub_str = strrchr(fontfile, '/'); // Find last slash in filename
    if(sub_str) sub_str++;         // First character of filename (path stripped)
    else        sub_str = fontfile; // No path; font in local dir.

    char fontName[strlen(sub_str)+20];

    if (glyph_id_count != 0) {
        // Indexed fonts - ignoring range arguments
        range_count = 0;
    }

    // Derive font table names from filename.  Period (filename
    // extension) is truncated and replaced with the font size & bits.
    strcpy(fontName, sub_str);
    sub_str = strrchr(fontName, '.'); // Find last period (file ext)
    if(!sub_str) sub_str = &fontName[strlen(fontName)]; // If none, append
    // Insert font size and 7/8 bit.  fontName was alloc'd w/extra
    // space to allow this, we're not sprintfing into Forbidden Zone.
    sprintf(sub_str, "%dpt%db", size, bpp);
    // Space and punctuation chars in name replaced w/ underscores.
    for(i=0; (c=fontName[i]); i++) {
        if(isspace(c) || ispunct(c)) fontName[i] = '_';
    }

    if ( kerning == 1 ) {
        strcat(fontName, "_kern");
    }

    if ( glyph_id_count != 0 ) {
        strcat(fontName, "_indexed");
    }

    int  fontName_len = strlen(fontName);
    char fontName_upper[strlen(sub_str)+20];

    str_to_upper(fontName_upper, fontName, fontName_len);

    //Create .bin file
    char *tmp_filename = malloc(strlen(fontName) + 20);
    strcpy(tmp_filename, fontName);
    tmp_filename[strlen(fontName)  ] = '.';
    tmp_filename[strlen(fontName)+1] = 'b';
    tmp_filename[strlen(fontName)+2] = 'i';
    tmp_filename[strlen(fontName)+3] = 'n';
    tmp_filename[strlen(fontName)+4] = '\0';
    FILE * pFile = create_file(tmp_filename);
    if (pFile == NULL) {
        free(tmp_filename);
        return -1;
    }

    //Create .c file
    tmp_filename[strlen(fontName)  ] = '.';
    tmp_filename[strlen(fontName)+1] = 'c';
    tmp_filename[strlen(fontName)+2] = '\0';
    dot_c_file = create_file(tmp_filename);
    if (dot_c_file == NULL) {
        fclose(pFile);
        free(tmp_filename);
        return -1;
    }

    //Create .h file
    tmp_filename[strlen(fontName)  ] = '.';
    tmp_filename[strlen(fontName)+1] = 'h';
    tmp_filename[strlen(fontName)+2] = '\0';
    FILE *dot_h_file = create_file(tmp_filename);
    if (dot_h_file == NULL) {
        fclose(pFile);
        free(tmp_filename);
        return -1;
    }

    FILE *p1_dot_h = create_file("1.h");
    if (p1_dot_h == NULL) {
        fclose(pFile);
        fclose(dot_c_file);
        free(tmp_filename);
        return -1;
    }

    free(tmp_filename);

    // Init FreeType lib, load font
    if((err = FT_Init_FreeType(&library))) {
        fprintf(stderr, "FreeType init error: %d", err);
        fclose(pFile);
        fclose(dot_c_file);
        remove("1.h");
        return err;
    }
    if((err = FT_New_Face(library, fontfile, 0, &face))) {
        fprintf(stderr, "Font load error: %d", err);
        fclose(pFile);
        fclose(dot_c_file);
        FT_Done_FreeType(library);
        remove("1.h");
        return err;
    }

    //STB_TRUETYPE
    //*********************************************************
    long file_size;
    unsigned char* fontBuffer;
    FILE* fontFile = fopen(fontfile, "rb");
    fseek(fontFile, 0, SEEK_END);
    file_size = ftell(fontFile); /* how long is the file ? */
    fseek(fontFile, 0, SEEK_SET); /* reset */
    fontBuffer = malloc(file_size);
    fread(fontBuffer, file_size, 1, fontFile);
    fclose(fontFile);

    /* prepare font */
    stbtt_fontinfo ttf_info;
    if (!stbtt_InitFont(&ttf_info, fontBuffer, 0)) {
        fprintf(stderr, "stb_truetype initialization failed\n");
    }

    /* calculate font scaling */
    float scale = stbtt_ScaleForPixelHeight(&ttf_info, size);
    int ascent, descent, lineGap;
    stbtt_GetFontVMetrics(&ttf_info, &ascent, &descent, &lineGap);

    ascent  *= scale;
    descent *= scale;
    //*********************************************************

    // << 6 because '26dot6' fixed-point format
    FT_Set_Char_Size(face, size << 6, 0, DPI, 0);

    int max_yOffset = 0;


    int first, last;

    fprintf(dot_c_file, "#ifndef %s_C\n", fontName_upper);
    fprintf(dot_c_file, "#define %s_C\n\n", fontName_upper);

    fprintf(dot_c_file, "#include \"%s.h\"\n\n", fontName);

    fprintf(dot_c_file, "#ifndef NEMA_GPU_MEM\n");
    fprintf(dot_c_file, "#define NEMA_GPU_MEM\n");
    fprintf(dot_c_file, "#endif // NEMA_GPU_MEM\n\n");

    fprintf(dot_c_file, "#ifndef NEMA_FONT_LOAD_FROM_BIN\n");
    fprintf(dot_c_file, "/* This will be read by the GPU only */\n");
    fprintf(dot_c_file, "static uint8_t NEMA_GPU_MEM %sBitmaps[] = {\n", fontName);

    char kern_str[50];
    int kern_list_size = 0;

    char indices_str[strlen(fontName) + 20];
    strcpy(indices_str, "NULL");

    //Handle glyph indices first
    for ( int i = 0; i < glyph_id_count; ++i ) {
        if ( i == 0 ) {
            firstCall = 1;
            fprintf(dot_c_file, "  // Indexed glyphs range\n  ");
            fprintf(p1_dot_h, "/* This struct will be read by the CPU only */\n");
            fprintf(p1_dot_h, "static const nema_glyph_indexed_t %sGlyphs[] = {\n", fontName);

            sprintf(indices_str, "%sGlyphs", fontName);
        }

        if (bpp == 1) {
            err =  FT_Load_Glyph(face, glyph_id[i], FT_LOAD_TARGET_MONO);
        } else {
            err =  FT_Load_Glyph(face, glyph_id[i], FT_LOAD_TARGET_NORMAL);
        }
        if(err) {
            fprintf(stderr, "Error %d loading glyph with index '%d'\n", err, glyph_id[i]);
            continue;
        }

        if (bpp == 1) {
            err = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_MONO);
        } else {
            err = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);
        }

        if(err) {
            fprintf(stderr, "Error %d rendering glyph with index '%d'\n", err, glyph_id[i]);
            continue;
        }

        if((err = FT_Get_Glyph(face->glyph, &glyph))) {
            fprintf(stderr, "Error %d getting glyph '%c'\n", err, i);
            continue;
        }

        bitmap = &face->glyph->bitmap;
        g      = (FT_BitmapGlyphRec *)glyph;

        // --------------------------------------------------------------
        // print nema_glyph_t table
        // --------------------------------------------------------------
        {
            fprintf(p1_dot_h, "  { %6u, %3d, %3d, %4d, %4d, %4d },   // Index %d\n",
            bitmapOffset,
            bitmap->width,
            (int)face->glyph->advance.x >> 6,
            g->left,
            1 - g->top,
            glyph_id[i],
            glyph_id[i]);

            CPUmem_text_count32+= nema_glyph_indexed_t_size;
            CPUmem_text_count64+= nema_glyph_indexed_t_size;
        }

        if (max_yOffset < g->top) {
            max_yOffset = g->top;
        }

        bitmapOffset += write_bitmap(pFile, bitmap, bpp);

        FT_Done_Glyph(glyph);

        if ( i == glyph_id_count - 1 ) {
            fprintf(p1_dot_h, "  { %6u, %3d, %3d, %4d, %4d, %4d }\n", bitmapOffset, 0, 0, 0, 0, -1);
            fprintf(p1_dot_h, "};\n\n");

            CPUmem_text_count32+= nema_glyph_indexed_t_size;
            CPUmem_text_count64+= nema_glyph_indexed_t_size;
        }
    }

    for (int r = 0; r < range_count; ++r) {
        first = range_start[r];
        last  = range_end[r];

        if (r != 0) {
            fprintf(dot_c_file, ",\n");
        }

        fprintf(dot_c_file, "  // 0x%x - 0x%x\n  ", first, last);
        firstCall = 1;

        fprintf(p1_dot_h, "/* This struct will be read by the CPU only */\n");
        fprintf(p1_dot_h, "static const nema_glyph_t %sGlyphs%d[] = {\n", fontName, r);

        // int kern_array_size = last - first + 1;
        // int8_t kerning_array[range_count][kern_array_size][kern_array_size];

        // Process glyphs and output huge bitmap data array
        for(i=first, j=0; i<=last; i++, j++) {
            // MONO renderer provides clean image with perfect crop
            // (no wasted pixels) via bitmap struct.
            if (bpp == 1)
                err = FT_Load_Char(face, i, FT_LOAD_TARGET_MONO);
            else
                err = FT_Load_Char(face, i, FT_LOAD_TARGET_NORMAL);
            if(err) {
                fprintf(stderr, "Error %d loading char '%c'\n",
                  err, i);
                continue;
            }

            if (bpp == 1)
                err = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_MONO);
            else
                err = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);
            if(err) {
                fprintf(stderr, "Error %d rendering char '%c'\n", err, i);
                continue;
            }

            if((err = FT_Get_Glyph(face->glyph, &glyph))) {
                fprintf(stderr, "Error %d getting glyph '%c'\n", err, i);
                continue;
            }

            int kern_char_length = 0;
            int kern_char_offset = 0;

            if ( kerning == 1 ) {
                if ( (FT_HAS_KERNING(face)) ) {

                    //Find pairs in all ranges
                    for (int cur_range_ = 0; cur_range_ < range_count; ++cur_range_) {
                        int first_ = range_start[cur_range_];
                        int last_  = range_end[cur_range_];

                        for ( int k = first_; k < last_; ++k ) {

                            int kern_offset = stbtt_GetCodepointKernAdvance(&ttf_info, k, i);

                            if ( kern_offset != 0 ) {
                                if ( kern_char_offset == 0 ) {
                                    kern_char_offset = kern_list_size;
                                }
                                //use a linked list to store the found pairs
                                insert_kern_pair((uint32_t) k, (uint32_t) i, kern_offset>>6);

                                kern_char_length++;
                                kern_list_size++;
                            }
                        }
                    }
                }
            }

            bitmap = &face->glyph->bitmap;
            g      = (FT_BitmapGlyphRec *)glyph;

            // --------------------------------------------------------------
            // print nema_glyph_t table
            // --------------------------------------------------------------
            {
                fprintf(p1_dot_h, "  { %6u, %3d, %3d, %4d, %4d, %5u, %3u },   // 0x%02X",
                bitmapOffset,
                bitmap->width,
                (int)face->glyph->advance.x >> 6,
                g->left,
                1 - g->top,
                kern_char_offset,
                kern_char_length,
                i);

                if((i >= ' ') && (i <= '~')) {
                    fprintf(p1_dot_h, " '%c'", i);
                }
                fprintf(p1_dot_h, "\n");

                CPUmem_text_count32+= nema_glyph_t_size;
                CPUmem_text_count64+= nema_glyph_t_size;
            }

            // --------------------------------------------------------------

            if (max_yOffset < g->top) {
                max_yOffset = g->top;
            }

            bitmapOffset += write_bitmap(pFile, bitmap, bpp);

            FT_Done_Glyph(glyph);

        } // for glyph first->last

        fprintf(p1_dot_h, "  { %6u, %3d, %3d, %4d, %4d, %5u, %3u }\n", bitmapOffset, 0, 0, 0, 0, 0, 0);
        fprintf(p1_dot_h, "};\n\n");


        //------------------------------------------------------------------------
        CPUmem_text_count32+= nema_glyph_t_size;
        CPUmem_text_count64+= nema_glyph_t_size;
        fflush(p1_dot_h);

    } //for range

    // --------------------------------------------------------------
    // print kerning table
    // --------------------------------------------------------------
    int kerning_mem_overhead = 0;
    strcpy(kern_str, "NULL");
    if ( (kerning == 1) && (kern_list_size > 0) ){
        fprintf(p1_dot_h, "nema_kern_pair_t kerning_%s[] = {\n", fontName);

        kern_pair_t *pair = kern_list_head;

        while ( pair != NULL ) {
            fprintf(p1_dot_h, "  { 0x%x, %4d}, //0x%x, 0x%x", pair->left, CLAMP(pair->x_offset, INT8_MIN, INT8_MAX), pair->left, pair->right);

            if((pair->left >= ' ') && (pair->left <= '~') && (pair->right >= ' ') && (pair->right <= '~')) {
                fprintf(p1_dot_h, " || %c, %c", pair->left, pair->right);
            }

            fprintf(p1_dot_h, "\n");

            kerning_mem_overhead += nema_kern_pair_t_size;
            pair = pair->next;
        }

        fprintf(p1_dot_h, "  { 0, 0} //end of array\n");
        fprintf(p1_dot_h, "};\n\n");
        sprintf(kern_str, "&kerning_%s[0]", fontName);

        kerning_mem_overhead += nema_kern_pair_t_size;
    }

    KernMem += kerning_mem_overhead;

    kern_list_clear();
// --------------------------------------------------------------
    fprintf(dot_c_file, "\n};\n\n"); // End bitmap array
    fprintf(dot_c_file, "\n#endif // NEMA_FONT_LOAD_FROM_BIN\n\n"); // End bitmap array
    append_file(dot_c_file, p1_dot_h);
    fclose(p1_dot_h);

    fprintf(dot_c_file, "/* This struct will be read by the CPU only */\n");
    fprintf(dot_c_file, "static const nema_font_range_t %s_ranges[] = {\n", fontName);
    for (int r = 0; r < range_count; ++r) {
        fprintf(dot_c_file,
            "  {0x%08x, 0x%08x, %sGlyphs%d},\n",
            range_start[r], range_end[r], fontName, r);

        CPUmem_text_count32+=nema_font_range_t_size32;
        CPUmem_text_count64+=nema_font_range_t_size64;
    }
    fprintf(dot_c_file,
        "  {0, 0, NULL}\n");
    CPUmem_text_count32+=nema_font_range_t_size32;
    CPUmem_text_count64+=nema_font_range_t_size64;
    fprintf(dot_c_file, "};\n\n");

    if (bpp == 1)
        err = FT_Load_Char(face, ' ', FT_LOAD_TARGET_MONO);
    else
        err = FT_Load_Char(face, ' ', FT_LOAD_TARGET_NORMAL);
    if(err) {
        fprintf(stderr, "Error %d loading char '%c'\n",
          err, i);
        fclose(pFile);
        fclose(dot_c_file);
        remove("1.h");
        return err;
    }

    int face_advanceX = face->glyph->advance.x >> 6;

    // Output font structure

    fprintf(dot_c_file, "/* This struct will be read by the CPU only */\n");
    fprintf(dot_c_file, "nema_font_t %s = {\n", fontName);
//  fprintf(dot_c_file, "  (uint8_t  *)%sBitmaps,\n", fontName);
    fprintf(dot_c_file, "  {\n");
    fprintf(dot_c_file, "#ifdef NEMA_FONT_LOAD_FROM_BIN\n");
    fprintf(dot_c_file, "    .base_virt = (void *) 0,\n");
    fprintf(dot_c_file, "    .base_phys = (uintptr_t) 0,\n");
    fprintf(dot_c_file, "#else // NEMA_FONT_LOAD_FROM_BIN\n");
    fprintf(dot_c_file, "    .base_virt = (void *) %sBitmaps,\n", fontName);
    fprintf(dot_c_file, "    .base_phys = (uintptr_t) %sBitmaps,\n", fontName);
    fprintf(dot_c_file, "#endif // NEMA_FONT_LOAD_FROM_BIN\n");
    fprintf(dot_c_file, "    .size      = %d\n", GPUmem_count);
    fprintf(dot_c_file, "  },\n");
    fprintf(dot_c_file, "  %s_ranges,\n", fontName);
    fprintf(dot_c_file, "  %d,\n", GPUmem_count);
    fprintf(dot_c_file, "#ifdef NEMA_FONT_LOAD_FROM_BIN\n");
    fprintf(dot_c_file, "  (uint8_t *)0,\n");
    fprintf(dot_c_file, "#else // NEMA_FONT_LOAD_FROM_BIN\n");
    fprintf(dot_c_file, "  %sBitmaps,\n", fontName);
    fprintf(dot_c_file, "#endif // NEMA_FONT_LOAD_FROM_BIN\n");
    fprintf(dot_c_file, "  0,\n");
    fprintf(dot_c_file, "  %d, %ld, %d, %d, %s, %s\n",
      face_advanceX, face->size->metrics.height >> 6, max_yOffset, bpp, kern_str, indices_str);
    fprintf(dot_c_file, "};");

    CPUmem_data_count32+=nema_font_t_size32;
    CPUmem_data_count64+=nema_font_t_size64;

    FT_Done_FreeType(library);

    fprintf(dot_c_file, "\n#endif // %s_C\n\n", fontName_upper);

    // -------------------- Header File ------------------------------
    {
        // print guards
        fprintf(dot_h_file, "#ifndef %s_H\n", fontName_upper);
        fprintf(dot_h_file, "#define %s_H\n\n", fontName_upper);

//        fprintf(dot_h_file, "#define NEMA_FONT_LOAD_FROM_BIN\n");


        // print comment

        fprintf(dot_h_file,
"/*\n\
    Do this:\n\
        #define NEMA_FONT_IMPLEMENTATION\n\
    before you include this file in *one* C file to create the implementation\n\
\n\
    OR\n\
\n\
    add:\n\
        %s.c\n\
    to your C_SRCS to be compiled\n\
*/\n\n", fontName
            );

        fprintf(dot_h_file, "#include \"nema_font.h\"\n\n");

        fprintf(dot_h_file, "extern nema_font_t %s;\n\n", fontName);

        fprintf(dot_h_file, "#ifdef NEMA_FONT_IMPLEMENTATION\n");
        fprintf(dot_h_file, "#include \"%s.c\"\n", fontName);
        fprintf(dot_h_file, "#endif // NEMA_FONT_IMPLEMENTATION\n");

        fprintf(dot_h_file, "\n#endif // %s_H\n\n", fontName_upper);

        fflush(dot_h_file);
        fclose(dot_h_file);
    }
    // ---------------------------------------------------------------

    fflush(dot_c_file);
    fflush(pFile);

    fclose(pFile);
    fclose(dot_c_file);

    remove("1.h");


    /* Calculate host and graphic memory size */
    CPUmem_data_count32 += KernMem;
    CPUmem_data_count64 += KernMem;

    CPUmem_count32 = CPUmem_text_count32 + CPUmem_data_count32;
    CPUmem_count64 = CPUmem_text_count64 + CPUmem_data_count64;

    printf("\n");
    printf("---------------------------\n");
    printf("Memory Footprint Estimation\n");
    printf("---------------------------\n");
    printf("\n32-bit system:\n");
    printf("\tTotal Memory    : ~%.0f Kb (%d bytes)\n", round((float)(CPUmem_count32 + GPUmem_count)/1000.f), CPUmem_count32 + GPUmem_count);
    printf("\tGraphics Memory : ~%.0f Kb (%d bytes)\n", round((float)GPUmem_count/1000.f), GPUmem_count);
    printf("\tHost Memory     : ~%.0f Kb (%d bytes)\n", round((float)CPUmem_count32/1000.f), CPUmem_count32);

    printf("\t\t(.text) : %d bytes\n", CPUmem_text_count32);
    if ( kerning == 1 ) {
        printf("\t\t(.data) : %d bytes (%d bytes for kerning)\n", CPUmem_data_count32, KernMem);
    } else {
        printf("\t\t(.data) : %d bytes\n", CPUmem_data_count32);
    }
    printf("\n64-bit system:\n");
    printf("\tTotal Memory    : ~%.0f Kb (%d bytes)\n", round((float)(CPUmem_count64 + GPUmem_count)/1000.f), CPUmem_count64 + GPUmem_count);
    printf("\tGraphics Memory : ~%.0f Kb (%d bytes)\n", round((float)GPUmem_count/1000.f), GPUmem_count);
    printf("\tHost Memory     : ~%.0f Kb (%d bytes)\n", round((float)CPUmem_count64/1000.f), CPUmem_count64);
    printf("\t\t(.text) : %d bytes\n", CPUmem_text_count64);
    if ( kerning == 1 ) {
        printf("\t\t(.data) : %d bytes (%d bytes for kerning)\n", CPUmem_data_count64, KernMem);
    } else {
        printf("\t\t(.data) : %d bytes\n", CPUmem_data_count64);
    }

    return 0;
}
