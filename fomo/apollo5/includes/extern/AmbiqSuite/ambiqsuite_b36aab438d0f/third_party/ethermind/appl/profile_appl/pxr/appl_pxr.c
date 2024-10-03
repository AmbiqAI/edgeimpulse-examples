
/**
 *  \file appl_pxr.c
 *
 *  This file contains the Health Thermometer Sensor application.
 */

/*
 *  Copyright (C) 2013. Mindtree Ltd.
 *  All rights reserved.
 */

/* --------------------------------------------- Header File Inclusion */
#include "appl.h"

#ifdef PXR

/* --------------------------------------------- External Global Variables */

/* --------------------------------------------- Exported Global Variables */
/* Tx Power Level */
#define APPL_PX_TX_POWER_LEVEL_LENGTH    0x01

/* --------------------------------------------- Static Global Variables */
static UCHAR appl_px_configured;
static UCHAR appl_px_read_tx_power_pending;
static UCHAR appl_px_tx_power_level;

/* --------------------------------------------- Functions */

void appl_pxr_init (void)
{
    appl_px_configured = BT_FALSE;
    appl_px_read_tx_power_pending = BT_FALSE;
}

API_RESULT appl_pxr_set_alert_level
           (
                GATT_DB_HANDLE    * handle,
                ATT_VALUE         * value
           )
{
    API_RESULT   retval;

    retval = APPL_PXR_INVALID_ALRT_VALUE;

    if (GATT_CHAR_LL_ALRT_LVL_INST == handle->char_id)
    {
        /**
         *  Permissible Alert Levels are:
         *     0 - No Alert
         *     1 - Mild Alert
         *     2 - High Alert
         */
         if (BT_FALSE != APPL_PXR_VALID_ALERT_LEVEL(value->val[0]))
         {
             /* APPL_PXR_SET_ALERT (handle->char_id, value->val[0]); */
             appl_px_configured = BT_TRUE;
             retval = API_SUCCESS;
         }
    }
    else if (GATT_CHAR_IM_ALRT_LVL_INST == handle->char_id)
    {
        APPL_TRC (
        "\n\n\n[APPL]: ------------------------User Alert --------------------\n\n\n");
        APPL_TRC (
        "[APPL]: %s Alert => IMMEDIATE ALERT \n",appl_pxr_get_alert_str(value->val[0]));
        APPL_TRC (
        "\n\n\n[APPL]: ------------------------User Alert --------------------\n\n");
    }

    return retval;
}

void appl_pxr_alert_user
     (
          APPL_HANDLE    * appl_handle,
          UCHAR            alert_reason
     )
{
    GATT_DB_HANDLE    db_handle;
#ifndef DONT_USE_STANDARD_IO
    API_RESULT        retval;
#endif /* DONT_USE_STANDARD_IO */
    UCHAR             ll_alert_level;
    UCHAR             im_alert_level;
    ATT_VALUE         value;

    db_handle.device_id = APPL_GET_DEVICE_HANDLE((*appl_handle));
    value.len = 1;

    /* Get Alert Level Configured for LL */
    value.val = &ll_alert_level;
    db_handle.service_id = GATT_SER_LINK_LOSS_INST;
    db_handle.char_id = GATT_CHAR_LL_ALRT_LVL_INST;

#ifndef DONT_USE_STANDARD_IO
    retval = 
#endif /* DONT_USE_STANDARD_IO */
    BT_gatt_db_get_char_val(&db_handle,&value);

#ifndef DONT_USE_STANDARD_IO
    APPL_TRC (
    "[APPL]: Fetched Alert (0x%02X) for Device 0x%02X for Link Loss Service "
    "with result 0x%04X\n",ll_alert_level,APPL_GET_DEVICE_HANDLE((*appl_handle)),retval);
#endif /* DONT_USE_STANDARD_IO */

    /* Get Alert Level Configured for IM */
    value.val = &im_alert_level;
    db_handle.service_id = GATT_SER_IMMDT_ALRT_INST;
    db_handle.char_id = GATT_CHAR_IM_ALRT_LVL_INST;

#ifndef DONT_USE_STANDARD_IO
    retval = 
#endif /* DONT_USE_STANDARD_IO */
    BT_gatt_db_get_char_val(&db_handle,&value);
    
#ifndef DONT_USE_STANDARD_IO
    APPL_TRC (
    "[APPL]: Fetched Alert (0x%02X) for Device 0x%02X for Immediate Alert "
    "Servicebwith result 0x%04X\n",im_alert_level,APPL_GET_DEVICE_HANDLE((*appl_handle)),retval);
#endif /* DONT_USE_STANDARD_IO */

    if (BT_FALSE == appl_px_configured)
    {
        return;
    }
    APPL_TRC (
    "\n\n\n[APPL]: ------------------------User Alert --------------------\n\n\n");
    switch (alert_reason)
    {
    case APPL_PXR_LINK_LOSS:
        APPL_TRC (
        "[APPL]: %s Alert => LINK LOST\n",appl_pxr_get_alert_str(ll_alert_level));
        break;

    default:
        APPL_TRC (
        "[APPL]: %s Alert => IMMEDIATE ALERT \n",appl_pxr_get_alert_str(im_alert_level));
        break;

    }
    APPL_TRC (
    "\n\n\n[APPL]: ------------------------User Alert --------------------\n\n");
}

char * appl_pxr_get_alert_str (UCHAR  alert_level)
{
    switch (alert_level)
    {
    case APPL_PXR_NO_ALERT:
        return "NO";
    case APPL_PXR_MILD_ALERT:
        return "MILD";
    case APPL_PXR_HIGH_ALERT:
        return "HIGH";
    default:
        break;
    }

    return NULL;
}

API_RESULT appl_pxr_get_tx_power_level
           (
                GATT_DB_HANDLE    * handle,
                ATT_VALUE         * value
           )
{
    API_RESULT     retval, api_return;

    BT_DEVICE_ADDR peer_bd_addr;
    UINT16         conn_handle;


    /**
     * Initiate Read Current Tx Power Level.
     *
     * The response will be sent once the TX Power Level is read.
     */
    api_return = device_queue_get_remote_addr
                 (
                     &handle->device_id,
                     &peer_bd_addr
                 );

    if (API_SUCCESS == api_return)
    {
        api_return = BT_hci_get_le_connection_handle
                     (
                         &peer_bd_addr,
                         &conn_handle
                     );

        if (API_SUCCESS == api_return)
        {
            api_return = BT_hci_read_transmit_power_level
                         (
                             conn_handle,
                             HCI_READ_CUR_TX_POWER_LEVEL
                         );
        }
    }

    if (API_SUCCESS == api_return)
    {
        appl_px_read_tx_power_pending = BT_TRUE;
        retval = GATT_DB_DELAYED_RESPONSE;
    }
    else
    {
        APPL_TRC(
        "[APPL] Failed to Read Current Tx Power Level");

        retval = (ATT_UNLIKELY_ERROR | ATT_ERR_ID);
    }

    return retval;
}

API_RESULT appl_pxr_get_tx_power_level_complete
           (
                UCHAR     status,
                UINT16    conn_handle,
                UCHAR     tx_power_level
           )
{
    ATT_HANDLE         * att_handle;
    ATT_READ_RSP_PARAM   rsp_param;
    ATT_ERROR_RSP_PARAM  err_param;
    API_RESULT           retval;
    ATT_ATTR_HANDLE      tx_power_level_attr_handle;

#if (BT_MAX_REMOTE_DEVICES > 1)
    UCHAR                index;
#endif /* BT_MAX_REMOTE_DEVICES > 1 */

    retval = API_SUCCESS;

    if (BT_FALSE != appl_px_read_tx_power_pending)
    {
        att_handle = NULL;

        /* Get ATT Handle */
    #if (BT_MAX_REMOTE_DEVICES == 1)
        if (conn_handle == appl_peer_device_list[0].cxn_handle)
        {
            att_handle = &appl_peer_device_list[0].att_handle;
        }
    #else
        for (index = 0; index < BT_MAX_REMOTE_DEVICES; index++)
        {
            if (conn_handle == appl_peer_device_list[index].cxn_handle)
            {
                att_handle = &appl_peer_device_list[index].att_handle;
                break;
            }
        }
    #endif /* BT_MAX_REMOTE_DEVICES */

        if  (0x00 == status)
        {
            /* Save Current Tx Power Level */
            appl_px_tx_power_level = tx_power_level;

            /* Access the application handle and send back read response */
            rsp_param.val = &appl_px_tx_power_level;
            rsp_param.len = APPL_PX_TX_POWER_LEVEL_LENGTH;

            /* Get ATT Handle */
            if (NULL != att_handle)
            {
                retval = BT_att_read_rsp
                         (
                             att_handle,
                             &rsp_param
                         );
            }
        }
        else
        {
            /* Failed to read Tx Power Level. Send Error */
            /* Not remembering if it was a read blob request */

            /* TBD: Remove Hardcoding */
            tx_power_level_attr_handle = 0x0015;

            err_param.handle = tx_power_level_attr_handle;
            err_param.op_code = ATT_READ_REQ;
            err_param.error_code = ATT_UNLIKELY_ERROR;

            retval = BT_att_send_error_rsp
                     (
                         att_handle,
                         &err_param
                     );
            APPL_TRC (
            "[APPL]: Sent Error Response with result 0x%04X",retval);
        }

        /* Reset Read Tx Power Pending flag */
        appl_px_read_tx_power_pending = BT_FALSE;
    }

    return retval;
}

#endif /* PXR */

