// -----------------------------------------------------------------------------
// Copyright (c) 2021 Think Silicon S.A.
// Think Silicon S.A. Confidential Proprietary
// -----------------------------------------------------------------------------
//     All Rights reserved - Unpublished -rights reserved under
//         the Copyright laws of the European Union
//
//  This file includes the Confidential information of Think Silicon S.A.
//  The receiver of this Confidential Information shall not disclose
//  it to any third party and shall protect its confidentiality by
//  using the same degree of care, but not less than a reasonable
//  degree of care, as the receiver uses to protect receiver's own
//  Confidential Information. The entire notice must be reproduced on all
//  authorised copies and copies may only be made to the extent permitted
//  by a licensing agreement from Think Silicon S.A..
//
//  The software is provided 'as is', without warranty of any kind, express or
//  implied, including but not limited to the warranties of merchantability,
//  fitness for a particular purpose and noninfringement. In no event shall
//  Think Silicon S.A. be liable for any claim, damages or other liability, whether
//  in an action of contract, tort or otherwise, arising from, out of or in
//  connection with the software or the use or other dealings in the software.
//
//
//                    Think Silicon S.A.
//                    http://www.think-silicon.com
//                    Patras Science Park
//                    Rion Achaias 26504
//                    Greece
// -----------------------------------------------------------------------------

#include "ng_globals.h"
#include "nema_utils.h"

static int i_hour = 0.f;
static int i_min  = 0.f;
static int i_sec  = 0.f;


static void clock_time_hh_mm_ss(char *str_time) {
    char str_hour[3];
    int2str(str_hour, 3, i_hour);

    char str_min[3];
    int2str(str_min, 3, i_min);

    char str_sec[3];
    int2str(str_sec, 3, i_sec);

    if ( i_hour < 10 ) {
        str_time[0] = '0';
        str_time[1] = str_hour[0];
    } else {
        str_time[0] = str_hour[0];
        str_time[1] = str_hour[1];
    }

    str_time[2] = ':';

    if ( i_min < 10 ) {
        str_time[3] = '0';
        str_time[4] = str_min[0];
    } else {
        str_time[3] = str_min[0];
        str_time[4] = str_min[1];
    }

    str_time[5] = ':';

    if ( i_sec < 10 ) {
        str_time[6] = '0';
        str_time[7] = str_sec[0];
    } else {
        str_time[6] = str_sec[0];
        str_time[7] = str_sec[1];
    }
    str_time[8] = '\0';
}

static void clock_time_hh_mm(char *str_time) {
    char str_hour[3];
    int2str(str_hour, 3, i_hour);

    char str_min[3];
    int2str(str_min, 3, i_min);

    if ( i_hour < 10 ) {
        str_time[0] = '0';
        str_time[1] = str_hour[0];
    } else {
        str_time[0] = str_hour[0];
        str_time[1] = str_hour[1];
    }

    str_time[2] = ':';

    if ( i_min < 10 ) {
        str_time[3] = '0';
        str_time[4] = str_min[0];
    } else {
        str_time[3] = str_min[0];
        str_time[4] = str_min[1];
    }

    str_time[5] = '\0';
}

static void clock_time_h_mm(char *str_time) {
    char str_hour[3];
    int2str(str_hour, 3, i_hour);

    char str_min[3];
    int2str(str_min, 3, i_min);

    if ( i_hour < 10 ) {
        str_time[0] = ' ';
        str_time[1] = str_hour[0];
    } else {
        str_time[0] = str_hour[0];
        str_time[1] = str_hour[1];
    }

    str_time[2] = ':';

    if ( i_min < 10 ) {
        str_time[3] = '0';
        str_time[4] = str_min[0];
    } else {
        str_time[3] = str_min[0];
        str_time[4] = str_min[1];
    }

    str_time[5] = '\0';
}

static void clock_time_hh(char *str_time) {
    char str_hour[3];
    int2str(str_hour, 3, i_hour);

    if ( i_hour < 10 ) {
        str_time[0] = '0';
        str_time[1] = str_hour[0];
    } else {
        str_time[0] = str_hour[0];
        str_time[1] = str_hour[1];
    }

    str_time[2] = '\n';
}

static void clock_time_h(char *str_time) {
    char str_hour[3];
    int2str(str_hour, 3, i_hour);

    if ( i_hour < 10 ) {
        str_time[0] = ' ';
        str_time[1] = str_hour[0];
    } else {
        str_time[0] = str_hour[0];
        str_time[1] = str_hour[1];
    }

    str_time[2] = '\n';
}

static void clock_time_mm(char *str_time) {
    char str_min[3];
    int2str(str_min, 3, i_min);

    if ( i_min < 10 ) {
        str_time[0] = '0';
        str_time[1] = str_min[0];
    } else {
        str_time[0] = str_min[0];
        str_time[1] = str_min[1];
    }

    str_time[2] = '\n';
}

static void clock_time_ss(char *str_time) {
    char str_sec[3];
    int2str(str_sec, 3, i_sec);

    if ( i_sec < 10 ) {
        str_time[0] = '0';
        str_time[1] = str_sec[0];
    } else {
        str_time[0] = str_sec[0];
        str_time[1] = str_sec[1];
    }

    str_time[2] = '\n';
}

static void (*time_to_string[]) (char *str_time) = {
    clock_time_hh_mm_ss,       
    clock_time_hh_mm,
    clock_time_h_mm,
    clock_time_hh,
    clock_time_h,
    clock_time_mm,
    clock_time_ss
};

DRAW_FUNC(ng_digital_clock_draw) {
    gitem_digital_clock_t *clock = NG_DIGITAL_CLOCK(git);

    nema_bind_font(clock->font);
	nema_set_blend_fill(NEMA_BL_SRC);

    char text[9];

    time_to_string[clock->time_format](text);

    nema_print(text, clock->x + x_off, clock->y + y_off, clock->w, clock->h, clock->text_color, clock->alignment);
}

void ng_digital_clock_update(gitem_base_t *git) {
    #ifdef WALL_TIME_CLOCKS
    i_sec  = (int) NG_WALL_TIME;
    i_min  = (int) NG_WALL_TIME;
    i_hour = (int) NG_WALL_TIME;

    i_sec  = i_sec % 60;
    i_min  = (i_min/60) % 60;
    i_hour = (i_hour/3600) % 24;
    #else
    //User code for updating variables i_sec, i_min, i_hour goes here
    #endif
}
