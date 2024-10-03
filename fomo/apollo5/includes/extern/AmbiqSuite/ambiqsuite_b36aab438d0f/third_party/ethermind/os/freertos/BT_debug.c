
/**
 *  \file BT_debug.c
 *
 *  This file contains source codes for the EtherMind Debug Library
 *  Implementation for Windows (User-mode).
 */

/*
 *  Copyright (C) 2013. Mindtree Ltd.
 *  All rights reserved.
 */

/* --------------------------------------------------- Header File Inclusion */
#include "BT_debug_internal.h"
#include "am_mcu_apollo.h"
#include "am_util.h"

/* --------------------------------------------------- Global Definitions */
/* Debug file handle */
char bt_debug_fd[256];

/* ------------------------------------------------- Static Global Variables */
#ifdef BT_ENABLE_DISABLE_RUNTIME_DEBUG
/* Runtime debug level */
DECL_STATIC UCHAR bt_runtime_debug_level;

/* Module Specific debug enabled/disabled flag */
DECL_STATIC UINT32 bt_runtime_debug_flag[BT_MAX_MODULE_PAGE_COUNT];
#endif /* BT_ENABLE_DISABLE_RUNTIME_DEBUG */

/* ------------------------------------------------- Functions */
void BT_Init_Debug_Library ( void )
{
#ifdef BT_ENABLE_DISABLE_RUNTIME_DEBUG
    UINT32 enable_all_bit_mask, page_index;
#endif /* BT_ENABLE_DISABLE_RUNTIME_DEBUG */

#ifdef BT_ENABLE_DISABLE_RUNTIME_DEBUG
    /* Initialize runtime debug level */
    bt_runtime_debug_level = BT_DEBUG_LEVEL_TRC;

#ifndef BT_DISABLE_DEBUG_LOG_ON_STARTUP
    /* Enable all module debug log - by default */
    enable_all_bit_mask = 0xFFFFFFFF;
#else
    enable_all_bit_mask = 0x00000000;
#endif /* BT_DISABLE_DEBUG_LOG_ON_STARTUP */

    for (page_index = 0; page_index < BT_MAX_MODULE_PAGE_COUNT; page_index++)
    {
        bt_runtime_debug_flag[page_index] = enable_all_bit_mask;
    }
#endif /* BT_ENABLE_DISABLE_RUNTIME_DEBUG */

    return;
}

void debug_bt_init (void)
{
    return;
}

void debug_bt_shutdown (void)
{
    am_hal_itm_not_busy();
    return;
}

#ifndef BT_DISABLE_ALL_DEBUG

INT32 BT_debug_printf(UCHAR msg_type, UINT32 module_id, const CHAR *fmt, ...)
{
#ifdef BT_ENABLE_DISABLE_RUNTIME_DEBUG
    UINT32 page_index;
    UINT32 module_bit_mask;
#endif /* BT_ENABLE_DISABLE_RUNTIME_DEBUG */


    va_list parg;


#ifdef BT_ENABLE_DISABLE_RUNTIME_DEBUG
    /* Check Message Level */
    if (msg_type > bt_runtime_debug_level)
    {
        return -1;
    }

    /* Extract Page Index and Module Bit Mask */
    BT_GET_PAGE_IDX_MODULE_BIT_MASK(module_id, page_index, module_bit_mask);

    /* Check if the module debug log is enabled */
    if ((bt_runtime_debug_flag[page_index] & module_bit_mask) != module_bit_mask)
    {
        return -1;
    }
#endif /* BT_ENABLE_DISABLE_RUNTIME_DEBUG */

    uint32_t ui32PrintLen;

    va_start (parg, fmt);
    ui32PrintLen = am_util_stdio_vsprintf (bt_debug_fd, fmt, parg);
    va_end (parg);

    if(ui32PrintLen >= 256)
    {
        //
        // Getting here means we tried to print too much data.
        //
        while(1);
    }

    am_util_stdio_printf(bt_debug_fd);

    return 0;
}

INT32 BT_debug_dump_bytes(UINT32 module_id, UCHAR *buffer, UINT32 length)
{
    char hex_stream[49];
    char char_stream[17];
    UINT32 i;
    UINT16 offset, count;
    UCHAR c;

    BT_debug_dump(module_id, "-- Dumping %d Bytes --\n",
    (int)length);

    BT_debug_dump(module_id,
    "-------------------------------------------------------------------\n");

    count = 0;
    offset = 0;
    for(i = 0; i < length; i ++)
    {
        c =  buffer[i];
        am_util_stdio_sprintf(hex_stream + offset, "%02X ", c);

        if ( (c >= 0x20) && (c <= 0x7E) )
        {
            char_stream[count] = c;
        }
        else
        {
            char_stream[count] = '.';
        }

        count ++;
        offset += 3;

        if(16 == count)
        {
            char_stream[count] = '\0';
            count = 0;
            offset = 0;

            BT_debug_dump(module_id, "%s   %s\n",
            hex_stream, char_stream);

            BT_mem_set(hex_stream, 0, 49);
            BT_mem_set(char_stream, 0, 17);
        }
    }

    if(0 != offset)
    {
        char_stream[count] = '\0';

        /* Maintain the alignment */
        BT_debug_dump(module_id, "%-48s   %s\n",
        hex_stream, char_stream);
    }

    BT_debug_dump(module_id,
    "-------------------------------------------------------------------\n");

    return 0;
}


INT32 BT_debug_dump_decimal(UINT32 module_id, UCHAR *buffer, UINT32 length)
{
    char stream[100];
    UINT32 i;
    UINT16 offset, count;

    BT_debug_dump(module_id, "Dumping %d Bytes (In Decimal): ------>\n",
    (int)length);

    count = 0;
    offset = 0;
    for(i = 0; i < length; i ++)
    {
        am_util_stdio_sprintf(stream + offset, "%3d ", (unsigned int)buffer[i]);
        count ++;
        offset += 4;

        if(16 == count)
        {
            count = 0;
            offset = 0;

            BT_debug_dump(module_id, "%s\n", stream);

            BT_mem_set(stream, 0, 100);
        }
    }

    if(0 != offset)
    {
        BT_debug_dump(module_id, "%s\n", stream);
    }

    BT_debug_dump(module_id, "<------------------------------------>\n");

    return 0;
}
#endif /* BT_DISABLE_ALL_DEBUG */

#ifdef BT_LOG_TIMESTAMP
UCHAR * BT_debug_get_current_timestamp (void)
{
    return NULL;
}
#endif /* BT_LOG_TIMESTAMP */

#ifdef BT_ENABLE_DISABLE_RUNTIME_DEBUG
void BT_set_debug_level(UCHAR level)
{
    /* Parameter validation */
    if (BT_DEBUG_LEVEL_ALL < level)
    {
        /* Failure */
        return;
    }

    /* Set runtime debug level */
    bt_runtime_debug_level = level;

    return;
}

void BT_update_module_debug_flag(UINT32 module_id, UCHAR flag)
{
    UINT32 page_index;
    UINT32 module_bit_mask;

    /* Parameter Validation */
    if (BT_DEBUG_ENABLE < flag)
    {
        return;
    }

    /* Check if the request is for enable/disable all */
    if (BT_MODULE_ALL == module_id)
    {
        if (BT_DEBUG_ENABLE == flag)
        {
            module_bit_mask = 0xFFFFFFFF;
        }
        else
        {
            module_bit_mask = 0x00000000;
        }

        for (page_index = 0; page_index < BT_MAX_MODULE_PAGE_COUNT; page_index++)
        {
            bt_runtime_debug_flag[page_index] = module_bit_mask;
        }

        return;
    }

    /* Extract Page Index and Module Bit Mask */
    BT_GET_PAGE_IDX_MODULE_BIT_MASK(module_id, page_index, module_bit_mask);

    /* Enable/disable based on the flag */
    if (BT_DEBUG_ENABLE == flag)
    {
        bt_runtime_debug_flag[page_index] |= (module_bit_mask);
    }
    else
    {
        bt_runtime_debug_flag[page_index] &= ~(module_bit_mask);
    }
}
#endif /* BT_ENABLE_DISABLE_RUNTIME_DEBUG */

