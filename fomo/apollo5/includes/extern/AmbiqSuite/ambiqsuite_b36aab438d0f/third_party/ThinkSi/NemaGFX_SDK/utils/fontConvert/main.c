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

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <ctype.h>

#include "font_convert.h"

static struct option long_options[] = {
    {"size"     , required_argument , 0, 's'},
    {"bpp"      , required_argument , 0, 'b'},
    {"bpp1"     , no_argument       , 0, '1'},
    {"bpp2"     , no_argument       , 0, '2'},
    {"bpp4"     , no_argument       , 0, '4'},
    {"bpp8"     , no_argument       , 0, '8'},
    {"range"    , required_argument , 0, 'r'},
    {"help"     , no_argument       , 0, 'h'},
    {"ascii"    , no_argument       , 0, 'a'},
    {"greek"    , no_argument       , 0, 'g'},
    {"kerning"  , no_argument       , 0, 'k'},
    {"indices"  , no_argument       , 0, 'i'},
    {0, 0, 0, 0}
};

static char *option_help[] = {
    "font size",
    "set bits per pixel, e.g.: -b 8",
    "set bits per pixel to 1",
    "set bits per pixel to 2",
    "set bits per pixel to 4",
    "set bits per pixel to 8",
    "add range of characters (start-end), e.g.: -r 0x20-0x7e, -r 32-127",
    "display this help and exit",
    "add ascii range. Equivalent to -r 0x20-0x7e",
    "add greek range. Equivalent to -r 0x370-0x3ff",
    "add kerning",
    "add glyph indices in array form e.g.: -i [4,17,20] (do not use spaces between them)",
    ""
};

static char *name        = "nema_font_convert - convert TTF fonts to .bin, .c and .h, compatible with NEMA|gfx graphics API";
static char *synopsis    = "nema_font_convert [OPTION]... [FILE]...";
static char *description = "Convert TTF fonts to .bin, .c and .h, compatible with NEMA|gfx graphics API";

static void print_help(void) {
    const int size = (int)( sizeof(long_options) / sizeof(long_options[0]) );

    printf("NAME\n");
    printf("\t%s\n\n", name);
    printf("SYNOPSIS\n");
    printf("\t%s\n\n", synopsis);
    printf("DESCRIPTION\n");
    printf("\t%s\n\n", description);

    for (int i = 0; i < size-1; ++i) {
        printf("\t-%c, --%s\n", long_options[i].val, long_options[i].name);
        printf("\t\t%s\n", option_help[i]);
    }
}


// this is such an overkill..
#define MAX_RANGES    100
#define MAX_GLYPH_IDS 200

int size = 8;
int bpp = 8;
int range_start[MAX_RANGES];
int range_end[MAX_RANGES];
int range_count = 0;
char *filename;
int kerning = 0;
int glyph_id[MAX_GLYPH_IDS];
int glyph_id_count = 0;

int main(int argc, char *argv[]) {

    if ( argc <= 1 ) {
        print_help();
        return 0;
    }

    while (1) {
        char *ptr = NULL;
        /* getopt_long stores the option index here. */
        int option_index = 0;

        int c = getopt_long (argc, argv, "kga1248hr:b:s:i:",
                         long_options, &option_index);

        /* Detect the end of the options. */
        if (c == -1)
            break;

        switch (c) {

        case 'h':
            print_help();
            return 0;
            break;

        case '1':
        case '2':
        case '4':
        case '8':
            bpp = c-'0';
            printf ("bpp = %d\n", bpp);
            break;
        case 'b':
            bpp = strtol(optarg, &ptr, 0);
            printf ("bpp = %d\n", bpp);
            break;
        case 's':
            size = strtol(optarg, &ptr, 0);
            printf ("size = %d\n", size);
            break;

        case 'r':
            if (range_count < MAX_RANGES) {
                range_start[range_count] = strtol(optarg, &ptr, 0);
                ++ptr;
                range_end[range_count]   = strtol(ptr, &ptr, 0);
                if (range_end[range_count] <= 0) {
                    range_end[range_count] = range_start[range_count];
                }
                if (range_end[range_count] < range_start[range_count]) {
                    int tmp = range_end[range_count];
                    range_end[range_count] = range_start[range_count];
                    range_start[range_count] = tmp;
                }
                printf ("range: 0x%x - 0x%x\n", range_start[range_count], range_end[range_count]);
                ++range_count;
            }
            break;

        case 'a':
            if (range_count < MAX_RANGES) {
                range_start[range_count] = 0x20;
                range_end[range_count]   = 0x7e;
                printf ("range: 0x%x - 0x%x\n", range_start[range_count], range_end[range_count]);
                ++range_count;
            }
            break;

        case 'g':
            if (range_count < MAX_RANGES) {
                range_start[range_count] = 0x370;
                range_end[range_count]   = 0x3ff;
                printf ("range: 0x%x - 0x%x\n", range_start[range_count], range_end[range_count]);
                ++range_count;
            }
            break;

        case 'k':
            kerning = 1;
            break;
        case 'i':
            if (glyph_id_count < MAX_GLYPH_IDS) {
                ptr = optarg;
                while (*ptr) {
                    if ( isdigit(*ptr) || ((*ptr=='-'||*ptr=='+') && isdigit(*(ptr+1))) ) {
                        // Found a number
                        glyph_id[glyph_id_count] = (int)strtol(ptr, &ptr, 10); // Read number
                        glyph_id_count++;
                    } else {
                        ptr++;
                    }
                }
            }

            break;
        case '?':
            /* getopt_long already printed an error message. */
            break;

        default:
            break;
        }
    }

    /* Print any remaining command line arguments (not options). */
    if (optind < argc) {
        int ind = optind;
        filename = argv[ind];
    } else {
        printf("Missing .ttf file path\n");
        return -1;
    }

    if ( (range_count <= 0) && (glyph_id_count <= 0) ) {
        printf("No range or glyph indices defined\n");
        return -1;
    }

    if (glyph_id_count != 0 && range_count != 0) {
        printf("Indexed fonts - ignoring range arguments\n");
        range_count = 0;
    }

    printf("Converting...\n");
    printf("\tfile         : %s\n", filename);
    printf("\tsize         : %d\n", size);
    printf("\tbpp          : %d\n", bpp);
    printf("\tkerning      : %d\n", kerning);

    for (int r = 0; r < range_count; ++r) {

        int start = range_start[r];
        int end   = range_end[r];
        printf("\trange        : %d - %d (0x%x - 0x%x)\n", start, end, start, end);
    }

    for (int i = 0; i < glyph_id_count; ++i) {
        if ( i == 0 ) {
            printf("\tglyph indices: %d,", glyph_id[i]);
        } else if ( i == glyph_id_count -1 ) {
            printf(" %d\n", glyph_id[i]);
        } else {
            printf(" %d,", glyph_id[i]);
        }
    }

    (void) font_convert(filename, size, bpp, range_start, range_end, range_count, kerning, glyph_id, glyph_id_count);
}
