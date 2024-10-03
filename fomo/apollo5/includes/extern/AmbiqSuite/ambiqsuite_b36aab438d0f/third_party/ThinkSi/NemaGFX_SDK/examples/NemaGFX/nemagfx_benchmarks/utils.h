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

#ifndef __UTILS_H_
#define __UTILS_H_

#include "nema_hal.h"
#include "nema_utils.h"
#include "nema_graphics.h"

#ifndef RESX
#define RESX 800
#endif

#ifndef RESY
#define RESY 600
#endif

#define TEST_MAX    36

#ifndef TIMEOUT_S
#define TIMEOUT_S 1.f
#endif

extern TLS_VAR img_obj_t fb;

typedef enum {
    CPU_GPU   = 0,
    CPU_BOUND = 1,
    GPU_BOUND = 2
} execution_mode;

int parse_params(int* testno, execution_mode* mode, int argc, char* argv[]);

void suite_init();
void suite_terminate();

void bench_start(int testno);
void bench_stop(int testno, int pix_count);

#endif
