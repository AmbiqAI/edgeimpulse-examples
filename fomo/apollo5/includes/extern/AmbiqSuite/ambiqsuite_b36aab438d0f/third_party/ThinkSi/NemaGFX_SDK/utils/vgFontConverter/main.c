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

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#include "vg_font_convert.h"

static struct option long_options[] = {
    {"range"    , required_argument , 0, 'r'},
    {"help"     , no_argument       , 0, 'h'},
    {"ascii"    , no_argument       , 0, 'a'},
    {"greek"    , no_argument       , 0, 'g'},
    {"kerning"  , no_argument       , 0, 'k'},
    {0, 0, 0, 0}
};

static char *option_help[] = {
    "add range of characters (start-end), e.g.: -r 0x20-0x7e, -r 32-127",
    "display this help and exit",
    "add ascii range. Equivalent to -r 0x20-0x7e",
    "add greek range. Equivalent to -r 0x370-0x3ff",
    "add kerning",
    ""
};

static char *name        = "nemavg_font_convert - convert TTF fonts to .bin, .c and .h vector fonts, compatible with NEMA|VG API";
static char *synopsis    = "nemavg_font_convert [OPTION]... [FILE]...";
static char *description = "Convert TTF fonts to .bin, .c and .h vector fonts, compatible with NEMA|VG  API";

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
#define MAX_RANGES 100

int range_start[MAX_RANGES];
int range_end[MAX_RANGES];
int range_count = 0;
char *filename;
int kerning = 0;

int main(int argc, char *argv[]) {

    if ( argc <= 1 ) {
        print_help();
        return 0;
    }

    while (1) {
        char *ptr = NULL;
        /* getopt_long stores the option index here. */
        int option_index = 0;

        int c = getopt_long (argc, argv, "kga1248hr:b:s:",
                         long_options, &option_index);

        /* Detect the end of the options. */
        if (c == -1)
            break;

        switch (c) {

        case 'h':
            print_help();
            return 0;
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

    if (range_count <= 0) {
        printf("No range defined\n");
        return -1;
    }

    printf("Converting...\n");
    printf("\tfile   : %s\n", filename);
    printf("\tkerning: %d\n", kerning);

    for (int r = 0; r < range_count; ++r) {

        int start = range_start[r];
        int end   = range_end[r];
        printf("\trange  : %d - %d (0x%x - 0x%x)\n", start, end, start, end);
    }
    (void) vg_font_convert(filename, range_start, range_end, range_count, kerning);
}
