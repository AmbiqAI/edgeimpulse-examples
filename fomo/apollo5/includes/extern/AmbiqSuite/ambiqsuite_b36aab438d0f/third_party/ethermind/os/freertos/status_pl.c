/**
 *  \file status_pl.c
 *
 *  This file contains platform specific routines for EtherMind Status Flag.
 */

/*
 *  Copyright (C) 2013. Mindtree Ltd.
 *  All rights reserved.
 */

/* ------------------------------------------- Header File Inclusion */
#include "BT_common.h"
#include "BT_status.h"


#ifdef BT_STATUS

/* ------------------------------------------- External Global Variables */


/* ------------------------------------------- Exported Global Variables */


/* ------------------------------------------- Static Global Variables */


/* ------------------------------------------- Functions */
/**
 *
 *  \fn status_set_bit_pl
 *
 *  \par Description
 *  Platform specific function to handle change in EtherMind Status Flag
 *
 *  \param [in] flag
 *         The current Status FLag value
 *
 *  \return void 
 */
void status_set_bit_pl
     (
         /* IN */ UINT32    flag
     )
{
    BT_debug_info(BT_MODULE_ID_STATUS,
    "[Status] Change in EtherMind Status Flag = 0x%08lX\n",
    flag);
}

#endif /* BT_STATUS */

