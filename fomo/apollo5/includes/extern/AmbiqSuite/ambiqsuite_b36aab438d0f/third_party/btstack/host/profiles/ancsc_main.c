//*****************************************************************************
//
//! aancsc_main.c
//! @file
//!
//! @brief This file provides the main application for the ANCS client data handle.
//!
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include "am_logger.h"
#include "rpc_common.h"
#include "ancsc_api.h"

/*
 * GLOBAL VARIABLE
 ****************************************************************************************
 */


/*
 * LOCAL FUNCTION DEFINITIONS
 ****************************************************************************************
 */

/*! use to print the received attribute data*/
static char g_attrString[ANCSC_REV_ATTR_STRING_PRINT_SIZE];

/*************************************************************************************************/
/*!
 *  \fn     app_ancs_data_process
 *
 *  \brief  Process the received ANCS data from phone
 *
 *  \para   ancs_data  Pointer of ANCS data.
 */
/*************************************************************************************************/
void app_ancs_data_process(uint8_t *ancs_data)
{
    uint8_t attr_len = 0;
    uint8_t attr_id = 0;
    uint8_t event_id = 0;
    uint8_t event_flag = 0;
    uint8_t category_id = 0;
    uint8_t category_count = 0;
    uint32_t notification_uid = 0;
    uint16_t handle = 0;

    bool bAttrPrint = false;

    uint8_t *p_pkt_buf = ancs_data;

    p_pkt_buf += 2; // skip packet length

    BSTREAM_TO_UINT8(attr_len, p_pkt_buf);
    BSTREAM_TO_UINT8(attr_id, p_pkt_buf);

    BSTREAM_TO_UINT8(event_id, p_pkt_buf);
    BSTREAM_TO_UINT8(event_flag, p_pkt_buf);
    BSTREAM_TO_UINT8(category_id, p_pkt_buf);
    BSTREAM_TO_UINT8(category_count, p_pkt_buf);

    BSTREAM_TO_UINT32(notification_uid, p_pkt_buf);

    switch (attr_id)
    {
    case BLE_ANCS_NOTIF_ATTR_ID_APP_IDENTIFIER:
    {
        am_info_printf(true,
            "************************************************************\r\n");
        am_info_printf(true, "* Notification Received \r\n");
        am_info_printf(true, "* UID             = %d, handle:%d\r\n", notification_uid, handle);
        switch (category_id)
        {
        case BLE_ANCS_CATEGORY_ID_OTHER:
            am_info_printf(true, "* Category        = Other\r\n");
            break;
        case BLE_ANCS_CATEGORY_ID_INCOMING_CALL:
            am_info_printf(true, "* Category        = Incoming Call\r\n");
            break;
        case BLE_ANCS_CATEGORY_ID_MISSED_CALL:
            am_info_printf(true, "* Category        = Missed Call\r\n");
            break;
        case BLE_ANCS_CATEGORY_ID_VOICE_MAIL:
            am_info_printf(true, "* Category        = Voice Mail\r\n");
            break;
        case BLE_ANCS_CATEGORY_ID_SOCIAL:
            am_info_printf(true, "* Category        = Social\r\n");
            break;
        case BLE_ANCS_CATEGORY_ID_SCHEDULE:
            am_info_printf(true, "* Category        = Schedule\r\n");
            break;
        case BLE_ANCS_CATEGORY_ID_EMAIL:
            am_info_printf(true, "* Category        = Email\r\n");
            break;
        case BLE_ANCS_CATEGORY_ID_NEWS:
            am_info_printf(true, "* Category        = News\r\n");
            break;
        case BLE_ANCS_CATEGORY_ID_HEALTH_AND_FITNESS:
            am_info_printf(true, "* Category        = Health and Fitness\r\n");
            break;
        case BLE_ANCS_CATEGORY_ID_BUSINESS_AND_FINANCE:
            am_info_printf(true,
                           "* Category        = Business and Finance\r\n");
            break;
        case BLE_ANCS_CATEGORY_ID_LOCATION:
            am_info_printf(true, "* Category        = Location\r\n");
            break;
        case BLE_ANCS_CATEGORY_ID_ENTERTAINMENT:
            am_info_printf(true, "* Category        = Entertainment\r\n");
            break;
        default:
            break;
        }

        switch (event_id)
        {
        case BLE_ANCS_EVENT_ID_NOTIFICATION_ADDED:
            am_info_printf(true, "* Event ID        = Added\r\n");
            break;
        case BLE_ANCS_EVENT_ID_NOTIFICATION_MODIFIED:
            am_info_printf(true, "* Event ID        = Modified\r\n");
            break;
        case BLE_ANCS_EVENT_ID_NOTIFICATION_REMOVED:
            am_info_printf(true, "* Event ID        = Removed\r\n");
            break;
        default:
            break;
        }

        am_info_printf(true, "* EventFlags      = \r\n");
        for (uint16_t i = 0; i < 5; i++)
        {
            if (event_flag &
                (0x00000001 << i))
            {
                switch (i)
                {
                case 0:
                    am_info_printf(true, "Silent \r\n");
                    break;
                case 1:
                    am_info_printf(true, "Important \r\n");
                    break;
                case 2:
                    am_info_printf(true, "PreExisting \r\n");
                    break;
                case 3:
                    am_info_printf(true, "PositiveAction ");
                    break;
                case 4:
                    am_info_printf(true, "NegativeAction \r\n");
                    break;
                default:
                    break;
                }
            }
        }
        am_info_printf(true, "* Category Count  = %d\r\n",
                       category_count);
    }
    break;

    case BLE_ANCS_NOTIF_ATTR_ID_TITLE:
    {
        if (attr_len != 0)
        {
            am_info_printf(true, "* Title           = \r\n");
            bAttrPrint = true;
        }
    }
    break;

    case BLE_ANCS_NOTIF_ATTR_ID_SUBTITLE:
    {
        if (attr_len != 0)
        {
            am_info_printf(true, "* Subtitle        = \r\n");
            bAttrPrint = true;
        }
    }
    break;

    case BLE_ANCS_NOTIF_ATTR_ID_MESSAGE:
    {
        if (attr_len != 0)
        {
            am_info_printf(true, "* Message         = \r\n");
            bAttrPrint = true;
        }
    }
    break;

    case BLE_ANCS_NOTIF_ATTR_ID_DATE:
    {
        if (attr_len != 0)
        {
            am_info_printf(true, "* Date & Time     = \r\n");
            bAttrPrint = true;
        }
    }
    break;

    case BLE_ANCS_NOTIF_ATTR_ID_POSITIVE_ACTION_LABEL:
    {
        if (attr_len != 0)
        {
            am_info_printf(true, "* Positive Action = \r\n");
            bAttrPrint = true;
        }
    }
    break;

    case BLE_ANCS_NOTIF_ATTR_ID_NEGATIVE_ACTION_LABEL:
    {
        if (attr_len != 0)
        {
            am_info_printf(true, "* Negative Action = \r\n");
            bAttrPrint = true;
        }
    }
    break;

    default:
        break;
    }

    if (bAttrPrint)
    {
        uint16_t printLen =
            (attr_len < ANCSC_REV_ATTR_STRING_PRINT_SIZE)
                ? attr_len
                : ANCSC_REV_ATTR_STRING_PRINT_SIZE;

        memset(&g_attrString[0], 0, sizeof(g_attrString));
        strncpy(g_attrString, (char *)p_pkt_buf, printLen);

        am_info_printf(true, "%s\r\n", g_attrString);
    }
}

