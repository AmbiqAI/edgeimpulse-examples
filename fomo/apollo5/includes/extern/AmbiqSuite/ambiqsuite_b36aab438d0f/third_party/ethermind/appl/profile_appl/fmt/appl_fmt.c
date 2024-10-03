
/**
 *  \file appl_fmt.c
 *
 *  This file contains the Find Me Target application.
 */

/*
 *  Copyright (C) 2013. Mindtree Ltd.
 *  All rights reserved.
 */

/* --------------------------------------------- Header File Inclusion */
#include "appl.h"

#ifdef FMT

/* --------------------------------------------- External Global Variables */

/* --------------------------------------------- Exported Global Variables */

/* --------------------------------------------- Static Global Variables */

/* --------------------------------------------- Functions */

void appl_fmt_init (void)
{
}

API_RESULT appl_fmt_set_alert_level
           (
                GATT_DB_HANDLE    * handle,
                ATT_VALUE         * value
           )
{
    API_RESULT   retval;

    retval = APPL_FMT_INVALID_ALRT_VALUE;

    if (GATT_CHAR_IM_ALRT_LVL_INST == handle->char_id)
    {
        APPL_TRC (
        "\n\n\n[APPL]: ------------------------User Alert --------------------\n\n\n");
        APPL_TRC (
        "[APPL]: %s Alert => IMMEDIATE ALERT \n",appl_fmt_get_alert_str(value->val[0]));
        APPL_TRC (
        "\n\n\n[APPL]: ------------------------User Alert --------------------\n\n");
    }

    return retval;
}

void appl_fmt_alert_user
     (
          DEVICE_HANDLE    * handle
     )
{
    GATT_DB_HANDLE    db_handle;
    API_RESULT        retval;
    UCHAR             im_alert_level;
    ATT_VALUE         value;

    db_handle.device_id = (*handle);
    value.len = 1;

    /* Get Alert Level Configured for IM */
    value.val = &im_alert_level;
    db_handle.service_id = GATT_SER_IMMDT_ALRT_INST;
    db_handle.char_id = GATT_CHAR_IM_ALRT_LVL_INST;

    retval = BT_gatt_db_get_char_val(&db_handle,&value);
    APPL_TRC (
    "[APPL]: Fetched Alert (0x%02X) for Device 0x%02X for Immediate Alert "
    "Servicebwith result 0x%04X\n",im_alert_level,(*handle),retval);

    APPL_TRC (
    "\n\n\n[APPL]: ------------------------User Alert --------------------\n\n\n");

    APPL_TRC (
    "[APPL]: %s Alert => IMMEDIATE ALERT \n",appl_fmt_get_alert_str(im_alert_level));

    APPL_TRC (
    "\n\n\n[APPL]: ------------------------User Alert --------------------\n\n");
}

char * appl_fmt_get_alert_str (UCHAR  alert_level)
{
    switch (alert_level)
    {
    case APPL_FMT_NO_ALERT:
        return "NO";
    case APPL_FMT_MILD_ALERT:
        return "MILD";
    case APPL_FMT_HIGH_ALERT:
        return "HIGH";
    default:
        break;
    }

    return NULL;
}

void appl_fmt_reinitialize (void)
{
}


#endif /* FMT */

