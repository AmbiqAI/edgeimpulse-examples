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

/*

Derived from "TrueType to Adafruit_GFX font converter".
*/

#include <stdio.h>
#include <ctype.h>
#include <stdint.h>
#include <ft2build.h>
#include <freetype/ftoutln.h>
#include <freetype/ftbbox.h>
#include <math.h>
#include "nema_vg_version.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"



#include FT_GLYPH_H

#define DPI 141

#define CLAMP(x, low, high) ( (x) < (low) ? (low) : ( (x) > (high) ? (high) : (x) ) )

FILE *dot_c_file;
FILE *p1_dot_h;
FILE *p2_dot_h;


#define NEMA_VG_PRIM_CLOSE              (0x00U)
#define NEMA_VG_PRIM_MOVE               (0x01U)
#define NEMA_VG_PRIM_LINE               (0x02U)
#define NEMA_VG_PRIM_HLINE              (0x03U)
#define NEMA_VG_PRIM_VLINE              (0x04U)
#define NEMA_VG_PRIM_BEZIER_QUAD        (0x05U)
#define NEMA_VG_PRIM_BEZIER_CUBIC       (0x06U)
#define NEMA_VG_PRIM_BEZIER_SQUAD       (0x07U)
#define NEMA_VG_PRIM_BEZIER_SCUBIC      (0x08U)

typedef struct {
    uint32_t data_offset;    /**< Offset value for the data of the glyph in the respective data array */
    size_t   data_length;    /**< Length of the data in the respective data array */
    uint32_t segment_offset; /**< Offset value for the segments of the glyph in the respective segment array */
    size_t   segment_length; /**< Length of the segments in the respective segment array */
    float    xAdvance;       /**< Advance width*/
    uint32_t kern_offset;    /**< Kerning offset  of the glyph in the respective kerning array */
    uint8_t  kern_length;    /**< Length of the kerning information of the glyph */
    int16_t  bbox_xmin;      /**< Minimum x of the glyph's bounding box*/
    int16_t  bbox_ymin;      /**< Minimum y of the glyph's bounding box*/
    int16_t  bbox_xmax;      /**< Maximum x of the glyph's bounding box*/
    int16_t  bbox_ymax;      /**< Maximum y of the glyph's bounding box*/
} nema_vg_glyph_t;

typedef struct kern_pair_t_{
   uint32_t left;
   uint32_t right;
   float    x_offset;
   struct kern_pair_t_ *next;
} kern_pair_t;

static kern_pair_t *kern_list_head = NULL;
static kern_pair_t *kern_list_cur  = NULL;

void insert_kern_pair(uint32_t left, uint32_t right, float x_offset) {
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

static uint8_t firstCallByte           = 1;
static uint8_t firstCallFloat          = 1;

static int data_count                  = 0;
static int seg_count                   = 0;

static int CPUmem_count32              = 0;
static int CPUmem_text_count32         = 0;
static int CPUmem_data_count32         = 0;
static int CPUmem_count64              = 0;
static int CPUmem_text_count64         = 0;
static int CPUmem_data_count64         = 0;
static int KernMem                     = 0;

static int nema_vg_kern_pair_t_size    = 8;
static int nema_vg_glyph_t_size        = 36; //28 + 8 bbox
static int nema_vg_font_range_t_size32 = 12;
static int nema_vg_font_range_t_size64 = 16;
static int nema_vg_font_t_size32       = 48;
static int nema_vg_font_t_size64       = 64;


static void write_float(float value, FILE *output_file) {
    static uint8_t row = 0;

    if (firstCallByte != 0) {
        row = 0;
    }

    if( firstCallByte == 0 ) {             // Format output table nicely
        if(++row >= 12) {                  // Last entry on line?
            fprintf(output_file, ",\n  "); // Newline format output
            row = 0;                       // Reset row counter
        } else {                           // Not end of line
            fprintf(output_file, ", ");    // Simple comma delim
        }
    }

    fprintf(output_file, "%.2ff", value);
    firstCallByte = 0;
    CPUmem_data_count32 += 4;
    CPUmem_data_count64 += 4;
}

static void write_byte(uint8_t value, FILE *output_file) {
    static uint8_t row = 0;

    if (firstCallFloat != 0) {
        row = 0;
    }

    if( firstCallFloat == 0 ) {            // Format output table nicely
        if(++row >= 12) {                  // Last entry on line?
            fprintf(output_file, ",\n  "); // Newline format output
            row = 0;                       // Reset row counter
        } else {                           // Not end of line
            fprintf(output_file, ", ");    // Simple comma delim
        }
    }

    fprintf(output_file, "%d", value); // Write byte value
    firstCallFloat = 0;

    CPUmem_data_count32 += 1;
    CPUmem_data_count64 += 1;
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

int get_outline(FT_Glyph glyph, FT_OutlineGlyph* outline) {
  int err = 0;

  switch ( glyph->format ) {
    case FT_GLYPH_FORMAT_BITMAP:
      err = 1;
      break;

    case FT_GLYPH_FORMAT_OUTLINE:
      *outline = (FT_OutlineGlyph)glyph;
      break;

    default:
      break;
  }

  return err;
}


// int PrintOutLine(FT_OutlineGlyph Outg)
// {
//     int Err = 0;
//     FT_Outline* outline = &Outg->outline;
//     FT_Vector* pPoint = outline->points;  //pointer to outline points

//     for ( int i = 0; i < outline->n_points; ++i) {
//         write_float(pPoint->x, dot_c_file);
//         write_float(pPoint->y, dot_c_file);
//         data_count += 2;
//         pPoint++;
//     }

//     return Err;
// }

int move_to( const FT_Vector*  to, void* user ) {
    write_byte(NEMA_VG_PRIM_MOVE, p1_dot_h);
    seg_count++;

    write_float((double)to->x, dot_c_file);
    write_float((double)to->y, dot_c_file);
    data_count += 2;
    return 0;
}


int line_to( const FT_Vector*  to, void* user ) {
    write_byte(NEMA_VG_PRIM_LINE, p1_dot_h);
    seg_count++;

    write_float((double)to->x, dot_c_file);
    write_float((double)to->y, dot_c_file);
    data_count += 2;
    return 0;
}

int quad_to( const FT_Vector*  control, const FT_Vector*  to, void* user ) {
    write_byte(NEMA_VG_PRIM_BEZIER_QUAD, p1_dot_h);
    seg_count++;

    write_float((double)control->x, dot_c_file);
    write_float((double)control->y, dot_c_file);
    write_float((double)to->x, dot_c_file);
    write_float((double)to->y, dot_c_file);
    data_count += 4;
    return 0;
}

int cubic_to( const FT_Vector*  control1, const FT_Vector*  control2, const FT_Vector*  to, void* user ) {
    write_byte(NEMA_VG_PRIM_BEZIER_CUBIC, p1_dot_h);
    seg_count++;

    write_float((double)control1->x, dot_c_file);
    write_float((double)control1->y, dot_c_file);
    write_float((double)control2->x, dot_c_file);
    write_float((double)control2->y, dot_c_file);
    write_float((double)to->x, dot_c_file);
    write_float((double)to->y, dot_c_file);
    data_count += 6;
    return 0;
}

int vg_font_convert(char *fontfile, int *range_start, int *range_end, int range_count, int kerning) {
    int i, err;
    char c;
    FT_Library         library;
    FT_Face            face;
    FT_Glyph           glyph;

    int data_length = 0;
    int seg_length  = 0;

    firstCallByte  = 1;
    firstCallFloat = 1;

    char *sub_str = strrchr(fontfile, '/'); // Find last slash in filename
    if(sub_str) sub_str++;         // First character of filename (path stripped)
    else        sub_str = fontfile; // No path; font in local dir.

    char fontName[strlen(sub_str)+20];

    // Derive font table names from filename.  Period (filename
    // extension) is truncated and replaced with the font size & bits.
    strcpy(fontName, sub_str);
    sub_str = strrchr(fontName, '.'); // Find last period (file ext)
    if(!sub_str) sub_str = &fontName[strlen(fontName)]; // If none, append
    // Insert font size and 7/8 bit.  fontName was alloc'd w/extra
    // space to allow this, we're not sprintfing into Forbidden Zone.
    // sprintf(sub_str, "%dpt%db", size, bpp);
    // Space and punctuation chars in name replaced w/ underscores.
    for(i=0; (c=fontName[i]); i++) {
        if(isspace(c) || ispunct(c)) fontName[i] = '_';
    }

    if ( kerning == 1 ) {
        strcat(fontName, "_kern");
    }

    int  fontName_len = strlen(fontName);
    char fontName_upper[strlen(sub_str)+20];

    str_to_upper(fontName_upper, fontName, fontName_len);


    //Create .c file
    char *tmp_filename = malloc(strlen(fontName) + 20);
    strcpy(tmp_filename, fontName);
    tmp_filename[strlen(fontName)  ] = '.';
    tmp_filename[strlen(fontName)+1] = 'c';
    tmp_filename[strlen(fontName)+2] = '\0';
    dot_c_file = create_file(tmp_filename);
    if (dot_c_file == NULL) {
        free(tmp_filename);
        return -1;
    }

    //Create .h file
    tmp_filename[strlen(fontName)  ] = '.';
    tmp_filename[strlen(fontName)+1] = 'h';
    tmp_filename[strlen(fontName)+2] = '\0';
    FILE *dot_h_file = create_file(tmp_filename);
    if (dot_h_file == NULL) {
        free(tmp_filename);
        return -1;
    }


    //helper files
    p1_dot_h = create_file("1.h");
    if (p1_dot_h == NULL) {
        fclose(dot_c_file);
        free(tmp_filename);
        return -1;
    }

    p2_dot_h = create_file("2.h");
    if (p2_dot_h == NULL) {
        fclose(dot_c_file);
        free(tmp_filename);
        return -1;
    }

    // Init FreeType lib, load font
    if((err = FT_Init_FreeType(&library))) {
        fprintf(stderr, "FreeType init error: %d", err);
        fclose(dot_c_file);
        fclose(p1_dot_h);
        fclose(p2_dot_h);
        remove("1.h");

        remove("2.h");
        return err;
    }

    if((err = FT_New_Face(library, fontfile, 0, &face))) {
        fprintf(stderr, "Font load error: %d", err);
        fclose(dot_c_file);
        fclose(p1_dot_h);
        fclose(p2_dot_h);
        FT_Done_FreeType(library);
        remove("1.h");
        remove("2.h");

        return err;
    }

    free(tmp_filename);

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
    if (!stbtt_InitFont(&ttf_info, fontBuffer, stbtt_GetFontOffsetForIndex(fontBuffer,0))) {
        fprintf(stderr, "stb_truetype initialization failed\n");
    }

    int first;
    int last;

    fprintf(dot_c_file, "#ifndef %s_C\n", fontName_upper);
    fprintf(dot_c_file, "#define %s_C\n\n", fontName_upper);

    fprintf(dot_c_file, "#include \"%s.h\"\n", fontName);
    fprintf(dot_c_file, "#include \"nema_vg_context.h\"\n\n");
    fprintf(dot_c_file, "static const nema_vg_float_t %s_data[] = {\n", fontName);
    fprintf(p1_dot_h, "static const uint8_t %s_segments[] = {\n", fontName);

    //segment functions
    FT_Outline_Funcs funcs;
    funcs.move_to  = &move_to;
    funcs.line_to  = &line_to;
    funcs.conic_to = &quad_to;
    funcs.cubic_to = &cubic_to;
    funcs.shift    = 0;
    funcs.delta    = 0;

    char kern_str[50];
    int kern_list_size = 0;

     // Process glyphs and output the data for each vertex
    int data_offset = 0;
    int seg_offset  = 0;

    uint8_t x_advance_overwrite = 0; //bool
    float x_advance_font = -1.f;

    for ( int r = 0; r < range_count; ++r ) {
        first = range_start[r];
        last  = range_end[r];

        firstCallByte  = 1;
        firstCallFloat = 1;

        if (r != 0) {
            fprintf(dot_c_file, ",\n");
        }

        if (r != 0) {
            fprintf(p1_dot_h, ",\n");
        }

         if (r != 0) {
            fprintf(p2_dot_h, "};\n\n");
        }

        fprintf(dot_c_file, "  // 0x%x - 0x%x\n  ", first, last);
        fprintf(p1_dot_h,   "  // 0x%x - 0x%x\n  ", first, last);

        fprintf(p2_dot_h, "static const nema_vg_glyph_t %sGlyphs%d[] = {\n", fontName, r);

        for ( i=first; i<=last; ++i ) {

            err = FT_Load_Char(face, i, FT_LOAD_NO_BITMAP | FT_LOAD_NO_SCALE);
            if(err) {
                fprintf(stderr, "Error %d loading char '%c'\n", err, i);
                continue;
            }

            if((err = FT_Get_Glyph(face->glyph, &glyph))) {
                fprintf(stderr, "Error %d getting glyph '%c'\n", err, i);
                continue;
            }

            data_count = 0;
            seg_count  = 0;

            FT_Outline  outline = face->glyph->outline;

            FT_BBox  bbox;
            FT_Outline_Get_BBox( &outline, &bbox);

            //Forbid negative xmin
            if ( bbox.xMin < 0 ) {
                FT_Outline_Translate( &outline, -bbox.xMin, 0 );
            }

            seg_count = 0;

            int err = FT_Outline_Decompose(&outline, &funcs, NULL);

            if ( err ) {
                fprintf(stderr, "Error %d outline decomposition  %c\n", err, i);
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
                                insert_kern_pair((uint32_t) k, (uint32_t) i, (float)kern_offset);

                                kern_char_length++;
                                kern_list_size++;
                            }
                        }
                    }
                }
            }

            float x_advance = (float)face->glyph->advance.x;

            if ( (i == ' ') && (x_advance_overwrite == 0) ) {
                x_advance_font      = x_advance;
                x_advance_overwrite = 1;
            }

            //print nema_vg_glyph_t fields
            fprintf(p2_dot_h, "  { %6u, %6u, %6u, %6u, %8.2ff, %4d, %4u, %6d, %6d, %6d, %6d},   // 0x%08X",
                data_offset,
                data_count,
                seg_offset,
                seg_count,
                x_advance,
                kern_char_offset,
                kern_char_length,
                (int) bbox.xMin,
                (int) bbox.yMin,
                (int) bbox.xMax,
                (int) bbox.yMax,
                i);

            data_offset += data_count;
            seg_offset  += seg_count;

            //print ascii chars
            if ( (i >= ' ') && (i <= '~') ) {
                fprintf(p2_dot_h, " '%c'", i);
            }

            fprintf(p2_dot_h, "\n");

            CPUmem_text_count32+= nema_vg_glyph_t_size;
            CPUmem_text_count64+= nema_vg_glyph_t_size;

            FT_Done_Glyph(glyph);
        } // for glyph first->last

        data_length = data_offset;
        seg_length  = seg_offset;
    } // for range

    fprintf(p1_dot_h,  "\n};\n\n");
    fprintf(p2_dot_h,  "\n};\n\n");
    fprintf(dot_c_file,"\n};\n\n"); // End data array

    // --------------------------------------------------------------
    // print kerning table
    // --------------------------------------------------------------
    int kerning_mem_overhead = 0;
    strcpy(kern_str, "NULL");
    if ( (kerning == 1) && (kern_list_size > 0) ){
        fprintf(p1_dot_h, "nema_vg_kern_pair_t kerning_%s[] = {\n", fontName);

        kern_pair_t *pair = kern_list_head;

        while ( pair != NULL ) {
            fprintf(p1_dot_h, "  { 0x%x, %8.2ff}, //0x%x, 0x%x", pair->left, pair->x_offset, pair->left, pair->right);

            if((pair->left >= ' ') && (pair->left <= '~') && (pair->right >= ' ') && (pair->right <= '~')) {
                fprintf(p1_dot_h, " || %c, %c", pair->left, pair->right);
            }

            fprintf(p1_dot_h, "\n");
            kerning_mem_overhead += nema_vg_kern_pair_t_size;

            pair = pair->next;
        }

        fprintf(p1_dot_h, "  { 0, 0} //end of array\n");
        fprintf(p1_dot_h, "};\n\n");
        sprintf(kern_str, "&kerning_%s[0]", fontName);

        kerning_mem_overhead += nema_vg_kern_pair_t_size;
    }

    KernMem += kerning_mem_overhead;

    kern_list_clear();
// --------------------------------------------------------------

    append_file(dot_c_file, p1_dot_h);
    append_file(dot_c_file, p2_dot_h);

    float font_ascender  = (float) (abs(face->ascender));   //ensure positive value
    float font_descender = (float) (-abs(face->descender)); //ensure negative value
    float default_size   =  (float) (font_ascender - font_descender);

    //If we have space ' ' in range, x_advance_font = space->xAdvance, otherwise default
    if ( x_advance_overwrite == 0 ) {
        x_advance_font = (float)face->max_advance_width;
    }

    fprintf(dot_c_file, "static const nema_vg_font_range_t %s_ranges[] = {\n", fontName);
    for (int r = 0; r < range_count; ++r) {
        fprintf(dot_c_file, "  {0x%08x, 0x%08x, %sGlyphs%d},\n", range_start[r], range_end[r], fontName, r);
        CPUmem_text_count32+=nema_vg_font_range_t_size32;
        CPUmem_text_count64+=nema_vg_font_range_t_size64;
    }

    fprintf(dot_c_file,"  {0, 0, NULL}\n");
    CPUmem_text_count32+=nema_vg_font_range_t_size32;
    CPUmem_text_count64+=nema_vg_font_range_t_size64;
    fprintf(dot_c_file, "};\n\n");

    fprintf(dot_c_file, "nema_vg_font_t %s = {\n", fontName);
    fprintf(dot_c_file, "    %u,\n", NEMA_VG_FONT_VERSION);
    fprintf(dot_c_file, "    %s_ranges,\n", fontName);
    fprintf(dot_c_file, "    %s_data,\n", fontName);
    fprintf(dot_c_file, "    %d,\n", data_length);
    fprintf(dot_c_file, "    %s_segments,\n", fontName);
    fprintf(dot_c_file, "    %d,\n", seg_length);
    fprintf(dot_c_file, "    %6.2ff, //size\n", default_size);
    fprintf(dot_c_file, "    %6.2ff, //xAdvance\n", x_advance_font);
    fprintf(dot_c_file, "    %6.2ff, //ascender\n", font_ascender);
    fprintf(dot_c_file, "    %6.2ff, //descender\n", font_descender);
    fprintf(dot_c_file, "    %s,\n", kern_str);
    fprintf(dot_c_file, "    0x0\n");
    fprintf(dot_c_file, "};\n\n");
    fprintf(dot_c_file, "#endif //%s_C\n", fontName_upper);

    CPUmem_data_count32+=nema_vg_font_t_size32;
    CPUmem_data_count64+=nema_vg_font_t_size64;

    FT_Done_FreeType(library);

    // -------------------- Header File ------------------------------
    {
        fprintf(dot_h_file, "#ifndef %s_H\n", fontName_upper);
        fprintf(dot_h_file, "#define %s_H\n\n", fontName_upper);
        fprintf(dot_h_file, "#include \"nema_vg_font.h\"\n\n");
        fprintf(dot_h_file, "extern nema_vg_font_t %s;\n\n", fontName);
        fprintf(dot_h_file, "\n#endif // %s_H\n\n", fontName_upper);

        fflush(dot_h_file);
        fclose(dot_h_file);
    }

    fclose(p1_dot_h);
    fclose(p2_dot_h);
    fclose(dot_c_file);

    remove("2.h");
    remove("1.h");

    // ---------------------------------------------------------------

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
    printf("\tTotal Memory    : ~%.0f Kb (%d bytes)\n", round((float)(CPUmem_count32)/1000.f), CPUmem_count32);
    printf("\tHost Memory     : ~%.0f Kb (%d bytes)\n", round((float)CPUmem_count32/1000.f), CPUmem_count32);

    printf("\t\t(.text) : %d bytes\n", CPUmem_text_count32);
    if ( kerning == 1 ) {
        printf("\t\t(.data) : %d bytes (%d bytes for kerning)\n", CPUmem_data_count32, KernMem);
    } else {
        printf("\t\t(.data) : %d bytes\n", CPUmem_data_count32);
    }
    printf("\n64-bit system:\n");
    printf("\tTotal Memory    : ~%.0f Kb (%d bytes)\n", round((float)(CPUmem_count64)/1000.f), CPUmem_count64);
    printf("\tHost Memory     : ~%.0f Kb (%d bytes)\n", round((float)CPUmem_count64/1000.f), CPUmem_count64);
    printf("\t\t(.text) : %d bytes\n", CPUmem_text_count64);
    if ( kerning == 1 ) {
        printf("\t\t(.data) : %d bytes (%d bytes for kerning)\n", CPUmem_data_count64, KernMem);
    } else {
        printf("\t\t(.data) : %d bytes\n", CPUmem_data_count64);
    }
    return 0;
}
