
/**
 *  \file btsnoop_pl.h
 *
 */

/*
 *  Copyright (C) 2016. Mindtree Ltd.
 *  All rights reserved.
 */

#ifndef _H_BTSNOOP_PL_
#define _H_BTSNOOP_PL_

/* --------------------------------------------- Header File Inclusion */
#include "BT_common.h"
#include "BT_fops.h"

#ifdef BT_SNOOP
/* --------------------------------------------- Global Definitions */
#define BT_SNOOP_TIMESTAMPED_FILE

/* Snoop file name */
#ifdef BT_SNOOP_TIMESTAMPED_FILE
#define BT_SNOOP_FILE_NAME_PREFIX "ethermind_"
#else
#define BT_SNOOP_FILE_NAME "ethermind"
#endif /* BT_SNOOP_TIMESTAMPED_FILE */


/* --------------------------------------------- Structures/Data Types */

/* --------------------------------------------- Macros */

/* --------------------------------------------- Internal Functions */

/* --------------------------------------------- API Declarations */
void btsnoop_init_pl (void);

API_RESULT btsnoop_open_pl (void);
API_RESULT btsnoop_close_pl (void);

UINT16 btsnoop_write_pl (void * buffer, UINT16 size);
UINT64 btsnoop_get_us_timestamp_pl(void);

#endif /* BT_SNOOP */

#endif /* _H_BTSNOOP_PL_ */

