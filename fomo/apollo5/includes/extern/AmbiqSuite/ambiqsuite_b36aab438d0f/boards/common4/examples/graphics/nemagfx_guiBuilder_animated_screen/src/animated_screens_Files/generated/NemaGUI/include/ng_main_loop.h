/*******************************************************************************
 * Copyright (c) 2021 Think Silicon S.A.
 *
   Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this header file and/or associated documentation files to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies of the
 * Materials, and to permit persons to whom the Materials are furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Materials.
 *
 * MODIFICATIONS TO THIS FILE MAY MEAN IT NO LONGER ACCURATELY REFLECTS
 * NEMAGUI API.
 *
 *  The software is provided 'as is', without warranty of any kind, express or
 *  implied, including but not limited to the warranties of merchantability,
 *  fitness for a particular purpose and noninfringement. In no event shall
 *  Think Silicon S.A. be liable for any claim, damages or other liability, whether
 *  in an action of contract, tort or otherwise, arising from, out of or in
 *  connection with the software or the use or other dealings in the software.
 ******************************************************************************/

/**
 * @file
 * @brief NemaGUI main loop function. 
 * 
 * This file must be included inside the main function's file. It contains the main loop function of a NemaGUI application.
 *
 */

#ifndef NG_MAIN_LOOP_H__
#define NG_MAIN_LOOP_H__

/**
 * @brief The applications main loop function
 * 
 * @param run_once if this is zero, the application will enter an endless loop, otherwise the main loop will run for one iteration.
 */
void ng_main_loop(const int run_once);

#endif //NG_MAIN_LOOP_H_
