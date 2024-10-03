//____________________________________________________________________
//Copyright Info      : All Rights Reserved - (c) Arasan Chip Systems Inc.,
//
// Name                 :  debug.c
//
// Creation Date     :  15-Dec-2006
//
// Description         :  Contains Debug macros useful debugging the SDIO Functions
//                              Driver main entry source file.
// Changed History :
//<Date>                  <Author>             <Version>             < Description >
//21-Nov-2006             VS                         0.1                          base code
//____________________________________________________________________


#ifndef _NEX_DEBUG_H
#define _NEX_DEBUG_H

#include "nex_porting.h"

extern INT32 debug_level;

static const CHAR str_en[] = "Entering->";
static const CHAR str_ex[] = "Exiting ->";
static const CHAR str[]    = "";
static const CHAR str_w[]  = "WARNING ";
static const CHAR str_e[]  = "ERROR ";
static const CHAR str_c[]  = "CRITICAL ";

#if defined(EN_PRINT)

#define DBG_TRACE_INIT()  \
if (debug_level >= 5) \
	DPRINT(str_en, __func__, __LINE__)

#define DBG_TRACE_EXIT()  \
if (debug_level >= 5) \
	DPRINT(str_ex, __func__, __LINE__)

#define DBG_PRINT(format, args...) \
if (debug_level >= 4) \
 	KPRINT(2, str, __func__, __LINE__, format, ##args)

#define REG_DUMP(slot_num, name, value)

#define INFO_PRINT(format, args...) \
if (debug_level >= 5) \
 	KPRINT(2, str, __func__, __LINE__, format, ##args)

#define ERR_PRINT(format, args...)       \
if (debug_level >= 0)                    \
 	KPRINT(2, str_e, __func__, __LINE__, format, ##args)

#define REL_PRINT(format, args...) RPRINT(format, ##args)

#elif defined(NO_PRINT)

#define DBG_TRACE_INIT(format, args...)
#define DBG_TRACE_EXIT(format, args...)
#define DBG_PRINT(format, args...)
#define REG_DUMP(slot_num, name, value)
#define INFO_PRINT(format, args...)
#define ERR_PRINT(format, args...) 
#define REL_PRINT(format, args...)

#endif

#endif
