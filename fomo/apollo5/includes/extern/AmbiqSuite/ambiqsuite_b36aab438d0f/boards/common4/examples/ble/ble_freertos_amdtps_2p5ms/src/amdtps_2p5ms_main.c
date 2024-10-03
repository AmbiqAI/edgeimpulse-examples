// ****************************************************************************
//
//! @file amdtps_2p5ms_main.c
//!
//! @brief This is BLE peripheral's application code for
//!        2.5ms connection interval testing based on Amdtps code.
//!
//! @addtogroup ble_examples BLE Examples
//!
//! @defgroup amdtps_2p5ms_main BLE FreeRTOS AMDTP Example
//! @ingroup ble_examples
//! @{
//!
//
// ****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#include <string.h>
#include "wsf_types.h"
#include "bstream.h"
#include "wsf_trace.h"
#include "wsf_msg.h"
#include "dm_api.h"
#include "att_api.h"
#include "smp_api.h"
#include "app_api.h"
#include "app_db.h"
#include "app_ui.h"
#include "svc_core.h"
#include "svc_dis.h"
#include "amdtps_2p5ms_api.h"
#include "amdtps_api.h"
#include "svc_amdtp.h"
#include "am_util.h"
#include "gatt_api.h"
#include "atts_main.h"
#include "hci_drv_cooper.h"
#include "hci_core.h"
#include "hci_drv_apollo.h"

/**************************************************************************************************
  Macros
**************************************************************************************************/

/*! WSF message event starting value */
#define AMDTP_MSG_START               0xA0

/*! WSF message event enumeration */
enum
{
    AMDTP_TIMER_IND = AMDTP_MSG_START,  /*! AMDTP tx timeout timer expired */
    AMDTP_CON_PARAM_UPDATE_TIMER_IND,   /*! AMDTP connection parameter update timer */
    AMDTP_SEND_DATA_TIMER_IND,          /*! Start sending data timer*/
    AMDTP_PHY_UPDATE_TIMER_IND,         /*! PHY update timer*/
    AMDTP_READ_EVT_CNT_TIMER_IND,       /*! Read event counter timer*/
};

/**************************************************************************************************
  Data Types
**************************************************************************************************/

/*! Application message type */
typedef union
{
    wsfMsgHdr_t     hdr;
    dmEvt_t         dm;
    attsCccEvt_t    ccc;
    attEvt_t        att;
} amdtpMsg_t;

// Period to initiate connection update, in second unit
#define AMDTP_CON_UPDATE_PER_SEC_MAX       (10)
#define AMDTP_CON_UPDATE_PER_SEC_MIN       (5)

// Maximum/mimimum value for connection latency
#define AMDTP_CON_LATENCY_MAX              (100)
#define AMDTP_CON_LATENCY_MIN              (10)

// The timer to start connection prameter update, in second unit
#define AMDTP_START_CON_UPDATE_TIMER_SEC   (5)

// Supervision timeout, in 10ms unit
#define AMDTP_SUP_TIMEOUT_10MS             (600)

// Timer to start sending data, in second unit
#define AMDTP_CON_START_TX_TIMER_SEC       (10)
// Maximum tx counter to display the sent packet number
#define AMDTP_TX_CNT_MAX                   (500)
// Start from the offset to fill in the data packet from the pre-defined data buffer, the maximum offset is 8
#define AMDTP_DATA_OFFSET_MAX              (8)
// Minimum data packet length
#define AMDTP_SEND_DATA_LEN_MIN            (1)
// Maximum data packet length
#define AMDTP_SEND_DATA_LEN_MAX            (244)
// Maximum data packet length for 2.5ms connection interval, it's calculated from controller side
#define AMDTP_SEND_DATA_LEN_2P5MS_MAX      (30)

// Period to initiate PHY update, in millisecond unit
#define AMDTP_PHY_UPDATE_PER_MS_MAX       (100)
#define AMDTP_PHY_UPDATE_PER_MS_MIN       (50)

// Read event connection counter timer, in second unit
#define AMDTP_READ_EVT_CNT_TIMER_SEC      (10)

typedef enum
{
    AMDTP_CONN_INT_IDX_2P5_MS,
    AMDTP_CONN_INT_IDX_10_MS,
    AMDTP_CONN_INT_IDX_15_MS,
    AMDTP_CONN_INT_IDX_MAX
}amdtpConnIntIdx_t;

// Connection interval configuration for stress testing
static uint16_t conn_interval[AMDTP_CONN_INT_IDX_MAX] =
{
    2.5 / 1.25,
    10 / 1.25,
    15 / 1.25,
};

typedef enum
{
    AMDTP_PHY_UPDATE_IDX_2M,
    AMDTP_PHY_UPDATE_IDX_1M,
    AMDTP_PHY_UPDATE_IDX_MAX
}amdtpPhyUPdateIdx_t;

// PHY update configuration for stress testing
static uint8_t upate_phy[AMDTP_PHY_UPDATE_IDX_MAX] =
{
    HCI_PHY_LE_2M_BIT,
    HCI_PHY_LE_1M_BIT,
};

amdtpsConnEnv_t amdtpsConnEnv;

/**************************************************************************************************
  Configurable Parameters
**************************************************************************************************/

/*! configurable parameters for advertising */
static const appAdvCfg_t amdtpAdvCfg =
{
    {  0,     0,     0},                /*! Advertising durations in ms */
    {  800,   800,     0}               /*! Advertising intervals in 0.625 ms units */
};

/*! configurable parameters for slave */
static const appSlaveCfg_t amdtpSlaveCfg =
{
    AMDTP_CONN_MAX,                           /*! Maximum connections */
};

/*! configurable parameters for security */
static const appSecCfg_t amdtpSecCfg =
{
    DM_AUTH_BOND_FLAG | DM_AUTH_SC_FLAG,    /*! Authentication and bonding flags */
    0,                                      /*! Initiator key distribution flags */
    DM_KEY_DIST_LTK,                        /*! Responder key distribution flags */
    FALSE,                                  /*! TRUE if Out-of-band pairing data is present */
    FALSE                                   /*! TRUE to initiate security upon connection */
};

/*! configurable parameters for AMDTP connection parameter update */
static const appUpdateCfg_t amdtpUpdateCfg =
{
    0,                                   /*! Connection idle period in ms before attempting
                                              connection parameter update; set to zero to disable */
    8,
    18,
    0,                                      /*! Connection latency */
    600,                                    /*! Supervision timeout in 10ms units */
    5                                       /*! Number of update attempts before giving up */
};

/*! SMP security parameter configuration */
static const smpCfg_t amdtpSmpCfg =
{
    3000,                                   /*! 'Repeated attempts' timeout in msec */
    SMP_IO_NO_IN_NO_OUT,                    /*! I/O Capability */
    7,                                      /*! Minimum encryption key length */
    16,                                     /*! Maximum encryption key length */
    3,                                      /*! Attempts to trigger 'repeated attempts' timeout */
    0,                                      /*! Device authentication requirements */
};

/**************************************************************************************************
  Advertising Data
**************************************************************************************************/

/*! advertising data, discoverable mode */
static const uint8_t amdtpAdvDataDisc[] =
{
    /*! flags */
    2,                                      /*! length */
    DM_ADV_TYPE_FLAGS,                      /*! AD type */
    DM_FLAG_LE_GENERAL_DISC |               /*! flags */
    DM_FLAG_LE_BREDR_NOT_SUP,

    /*! tx power */
    2,                                      /*! length */
    DM_ADV_TYPE_TX_POWER,                   /*! AD type */
    0,                                      /*! tx power */

    /*! service UUID list */
    3,                                      /*! length */
    DM_ADV_TYPE_16_UUID,                    /*! AD type */
    UINT16_TO_BYTES(ATT_UUID_DEVICE_INFO_SERVICE),

    17,                                      /*! length */
    DM_ADV_TYPE_128_UUID,                    /*! AD type */
    ATT_UUID_AMDTP_SERVICE
};

/*! scan data, discoverable mode */
static const uint8_t amdtpScanDataDisc[] =
{
    /*! device name */
    6,                                      /*! length */
    DM_ADV_TYPE_LOCAL_NAME,                 /*! AD type */
    'A',
    'm',
    'd',
    't',
    'p'
};

/**************************************************************************************************
  Client Characteristic Configuration Descriptors
**************************************************************************************************/

/*! enumeration of client characteristic configuration descriptors */
enum
{
    AMDTP_GATT_SC_CCC_IDX,                  /*! GATT service, service changed characteristic */
    AMDTP_AMDTPS_TX_CCC_IDX,                /*! AMDTP service, tx characteristic */
    AMDTP_AMDTPS_RX_ACK_CCC_IDX,            /*! AMDTP service, rx ack characteristic */
    AMDTP_NUM_CCC_IDX
};

/*! client characteristic configuration descriptors settings, indexed by above enumeration */
static const attsCccSet_t amdtpCccSet[AMDTP_NUM_CCC_IDX] =
{
    /* cccd handle                value range               security level */
    {GATT_SC_CH_CCC_HDL,          ATT_CLIENT_CFG_INDICATE,  DM_SEC_LEVEL_NONE},   /* AMDTP_GATT_SC_CCC_IDX */
    {AMDTPS_TX_CH_CCC_HDL,        ATT_CLIENT_CFG_NOTIFY,    DM_SEC_LEVEL_NONE},   /* AMDTP_AMDTPS_TX_CCC_IDX */
    {AMDTPS_ACK_CH_CCC_HDL,       ATT_CLIENT_CFG_NOTIFY,    DM_SEC_LEVEL_NONE}    /* AMDTP_AMDTPS_RX_ACK_CCC_IDX */
};

/**************************************************************************************************
  Global Variables
**************************************************************************************************/

/*! WSF handler ID */
wsfHandlerId_t amdtpHandlerId;

/*************************************************************************************************/
/*!
 *  \fn     amdtpsConnUpdate
 *
 *  \brief  Initiate connection update procedure according to the specified interval index
 *
 *  \param  conn_int  connection interval, in 1.25ms unit
 *  \param  conn_latency  connection latency
 *
 *  \return -1 if the input parameter is invlid, 0 if there is no connection, else connection identifier value
*/
/*************************************************************************************************/
static int8_t amdtpsConnUpdate(uint16_t conn_int, uint8_t conn_latency)
{
    if ( (conn_int < HCI_CONN_INTERVAL_MIN) || (conn_int > HCI_CONN_INTERVAL_MAX )
        || (conn_latency > HCI_CONN_LATENCY_MAX))
    {
        APP_TRACE_ERR0("Invalid connecion parameters");
        return -1;
    }
    else
    {
        dmConnId_t connId = AppConnIsOpen();

        if (connId != DM_CONN_ID_NONE)
        {
            hciConnSpec_t connSpec;

            connSpec.connIntervalMin = conn_int;
            connSpec.connIntervalMax = conn_int;
            connSpec.connLatency = conn_latency;
            connSpec.supTimeout = AMDTP_SUP_TIMEOUT_10MS;
            connSpec.minCeLen = 0;
            connSpec.maxCeLen = 0;

            DmConnUpdate(connId, &connSpec);
            // Disable latency if update connection interval is less than 7.5ms,
            if ( conn_int < HCI_CONN_INTERVAL_7P5MS )
            {
                HciVsSetSlaveLatency(connId, 0x00); //disable latency
            }
        }

        return connId;
    }
}

/*************************************************************************************************/
/*!
 *  \fn     amdtpsConnUpdateTimerExpire
 *
 *  \brief Each time the connection update timer expires,
 *         it will use the next configured connection interval
 *         and randomized connection latency(except 2.5ms connection interval) for the next connection update.
 *         If the connection exists, then re-start randomized timer for next time connection update.
 *
*/
/*************************************************************************************************/
static void amdtpsConnUpdateTimerExpire(void)
{
    uint16_t conn_int = 0;
    uint16_t conn_latency = 0;

    conn_int = amdtpsConnEnv.pConnTestInt[(amdtpsConnEnv.connUpdateCnt) % AMDTP_CONN_INT_IDX_MAX];

    // Only if the connection interval is NOT 2.5ms then apply connection latency
    if ( (amdtpsConnEnv.connUpdateCnt % AMDTP_CONN_INT_IDX_MAX) != AMDTP_CONN_INT_IDX_2P5_MS )
    {
        conn_latency = random_num(AMDTP_CON_LATENCY_MAX, AMDTP_CON_LATENCY_MIN);
    }

    amdtpsConnEnv.connUpdateCnt++;

    if ( amdtpsConnUpdate(conn_int, conn_latency) <= DM_CONN_ID_NONE )
    {
        APP_TRACE_ERR0("Connection update failed");
        WsfTimerStop(&amdtpsConnEnv.connUpdateTimer);
    }
    else
    {
        // Start randomized timer for next connection parameter update
        WsfTimerStartSec(&amdtpsConnEnv.connUpdateTimer, random_num(AMDTP_CON_UPDATE_PER_SEC_MAX, AMDTP_CON_UPDATE_PER_SEC_MIN));
    }
}

//*****************************************************************************
//
// Send notification packet to Client
//
//*****************************************************************************
static void amdtpsSendNotifyData(void)
{
    dmConnId_t      connId;

    if ((connId = AppConnIsOpen()) != DM_CONN_ID_NONE)
    {
        amdtpsConnEnv.txCounter++;

        // Output the sending packet counter every AMDTP_TX_CNT_MAX packets.
        if ( (amdtpsConnEnv.txCounter != 0 )
            && ((amdtpsConnEnv.txCounter % AMDTP_TX_CNT_MAX) == 0))
        {
            APP_TRACE_INFO1("Tx cnt = %d\n", amdtpsConnEnv.txCounter);
        }

        AttsHandleValueNtf(connId, AMDTPS_TX_HDL, amdtpsConnEnv.txLen, &amdtpsConnEnv.txData[amdtpsConnEnv.txOffset]);

        if (++amdtpsConnEnv.txOffset == AMDTP_DATA_OFFSET_MAX)
        {
            amdtpsConnEnv.txOffset = 0;
        }
    }
    else
    {
        APP_TRACE_ERR0("Connection NOT opened!");
    }
}

/*************************************************************************************************/
/*!
 *  \fn     amdtpsPhyUpdateTimerExpire
 *
 *  \brief  Each time the PHY update timer expires, it will use the next configured PHY for PHY update.
 *          The first time, it will update to 2M PHY as the used PHY is 1M by default.
 *
*/
/*************************************************************************************************/
static void amdtpsPhyUpdateTimerExpire(void)
{
    uint8_t conn_id = AppConnIsOpen();

    uint8_t update_phy = amdtpsConnEnv.pUpdatePhy[(amdtpsConnEnv.phyUpdateCnt) % AMDTP_PHY_UPDATE_IDX_MAX];

    DmSetPhy(conn_id, HCI_ALL_PHY_ALL_PREFERENCES, update_phy, update_phy, HCI_PHY_OPTIONS_NONE);

    amdtpsConnEnv.phyUpdateCnt++;
}

/*************************************************************************************************/
/*!
 *  \fn     amdtpsConnEnvCleanup
 *
 *  \brief  Clean up the resources used during testing
 *
*/
/*************************************************************************************************/
static void amdtpsConnEnvCleanup(void)
{
    WsfTimerStop(&amdtpsConnEnv.connUpdateTimer);
    WsfTimerStop(&amdtpsConnEnv.dataTxTimer);
    WsfTimerStop(&amdtpsConnEnv.readEvtCntTimer);
    amdtpsConnEnv.connUpdateCnt = 0;
    amdtpsConnEnv.txCounter = 0;
    amdtpsConnEnv.phyUpdateCnt = 0;
    amdtpsConnEnv.firstTimeRdEvtCnt = false;
}

/*************************************************************************************************/
/*!
 *  \fn     amdtpDmCback
 *
 *  \brief  Application DM callback.
 *
 *  \param  pDmEvt  DM callback event
 *
 *  \return None.
 */
/*************************************************************************************************/
static void amdtpDmCback(dmEvt_t *pDmEvt)
{
    dmEvt_t *pMsg;
    uint16_t len;

    len = DmSizeOfEvt(pDmEvt);

    if ((pMsg = WsfMsgAlloc(len)) != NULL)
    {
        memcpy(pMsg, pDmEvt, len);
        WsfMsgSend(amdtpHandlerId, pMsg);
    }
}

/*************************************************************************************************/
/*!
 *  \fn     amdtpAttCback
 *
 *  \brief  Application ATT callback.
 *
 *  \param  pEvt    ATT callback event
 *
 *  \return None.
 */
/*************************************************************************************************/
static void amdtpAttCback(attEvt_t *pEvt)
{
    attEvt_t *pMsg;

    if ((pMsg = WsfMsgAlloc(sizeof(attEvt_t) + pEvt->valueLen)) != NULL)
    {
        memcpy(pMsg, pEvt, sizeof(attEvt_t));
        pMsg->pValue = (uint8_t *) (pMsg + 1);
        memcpy(pMsg->pValue, pEvt->pValue, pEvt->valueLen);
        WsfMsgSend(amdtpHandlerId, pMsg);
    }
}

/*************************************************************************************************/
/*!
 *  \fn     amdtpCccCback
 *
 *  \brief  Application ATTS client characteristic configuration callback.
 *
 *  \param  pDmEvt  DM callback event
 *
 *  \return None.
 */
/*************************************************************************************************/
static void amdtpCccCback(attsCccEvt_t *pEvt)
{
    attsCccEvt_t  *pMsg;
    appDbHdl_t    dbHdl;

    /* if CCC not set from initialization and there's a device record */
    if ((pEvt->handle != ATT_HANDLE_NONE) &&
        ((dbHdl = AppDbGetHdl((dmConnId_t) pEvt->hdr.param)) != APP_DB_HDL_NONE))
    {
        /* store value in device database */
        AppDbSetCccTblValue(dbHdl, pEvt->idx, pEvt->value);
    }

    if ((pMsg = WsfMsgAlloc(sizeof(attsCccEvt_t))) != NULL)
    {
        memcpy(pMsg, pEvt, sizeof(attsCccEvt_t));
        WsfMsgSend(amdtpHandlerId, pMsg);
    }
}

/*************************************************************************************************/
/*!
 *  \fn     amdtpProcCccState
 *
 *  \brief  Process CCC state change.
 *
 *  \param  pMsg    Pointer to message.
 *
 *  \return None.
 */
/*************************************************************************************************/
static void amdtpProcCccState(amdtpMsg_t *pMsg)
{
    APP_TRACE_INFO3("ccc state ind value:%d handle:%d idx:%d", pMsg->ccc.value, pMsg->ccc.handle, pMsg->ccc.idx);

    /* AMDTPS TX CCC */
    if (pMsg->ccc.idx == AMDTP_AMDTPS_TX_CCC_IDX)
    {
        if (pMsg->ccc.value == ATT_CLIENT_CFG_NOTIFY)
        {
            // notify enabled
            amdtps_start((dmConnId_t)pMsg->ccc.hdr.param, AMDTP_TIMER_IND, AMDTP_AMDTPS_TX_CCC_IDX);
        }
        else
        {
            // notify disabled
            amdtps_stop((dmConnId_t)pMsg->ccc.hdr.param);
        }
    }
}

/*************************************************************************************************/
/*!
 *  \fn     amdtpSetup
 *
 *  \brief  Set up advertising and other procedures that need to be performed after
 *          device reset.
 *
 *  \param  pMsg    Pointer to message.
 *
 *  \return None.
 */
/*************************************************************************************************/
static void amdtpSetup(amdtpMsg_t *pMsg)
{
    /* set advertising and scan response data for discoverable mode */
    AppAdvSetData(APP_ADV_DATA_DISCOVERABLE, sizeof(amdtpAdvDataDisc), (uint8_t *) amdtpAdvDataDisc);
    AppAdvSetData(APP_SCAN_DATA_DISCOVERABLE, sizeof(amdtpScanDataDisc), (uint8_t *) amdtpScanDataDisc);

    /* set advertising and scan response data for connectable mode */
    AppAdvSetData(APP_ADV_DATA_CONNECTABLE, sizeof(amdtpAdvDataDisc), (uint8_t *) amdtpAdvDataDisc);
    AppAdvSetData(APP_SCAN_DATA_CONNECTABLE, sizeof(amdtpScanDataDisc), (uint8_t *) amdtpScanDataDisc);

    /* start advertising; automatically set connectable/discoverable mode and bondable mode */
    AppAdvStart(APP_MODE_AUTO_INIT);
}

/*************************************************************************************************/
/*!
 *  \fn     amdtpBtnCback
 *
 *  \brief  Button press callback.
 *
 *  \param  btn    Button press.
 *
 *  \return None.
 */
/*************************************************************************************************/
static void amdtpBtnCback(uint8_t btn)
{
    dmConnId_t      connId;

    /* button actions when connected */
    if ((connId = AppConnIsOpen()) != DM_CONN_ID_NONE)
    {
        switch (btn)
        {
            case APP_UI_BTN_1_SHORT:
            break;

            case APP_UI_BTN_2_SHORT:
            break;

            default:
            break;
        }
    }
    /* button actions when not connected */
    else
    {
        switch (btn)
        {
            case APP_UI_BTN_1_SHORT:
            break;

            case APP_UI_BTN_2_SHORT:
            break;

            default:
            break;
        }
    }
}

/*************************************************************************************************/
/*!
 *  \fn     amdtpProcMsg
 *
 *  \brief  Process messages from the event handler.
 *
 *  \param  pMsg    Pointer to message.
 *
 *  \return None.
 */
/*************************************************************************************************/
static void amdtpProcMsg(amdtpMsg_t *pMsg)
{
    uint8_t uiEvent = APP_UI_NONE;

    switch ( pMsg->hdr.event )
    {
        case AMDTP_TIMER_IND:
        {
            amdtps_proc_msg(&pMsg->hdr);
        }
        break;

        case ATTS_HANDLE_VALUE_CNF:
        {
            if (pMsg->att.hdr.status == ATT_SUCCESS)
            {
                amdtpsSendNotifyData();
            }
        }
        break;

        case ATTS_CCC_STATE_IND:
        {
            amdtpProcCccState(pMsg);
        }
        break;

        case ATT_MTU_UPDATE_IND:
        {
            APP_TRACE_INFO1("Negotiated MTU %d", ((attEvt_t *)pMsg)->mtu);
        }
        break;

        case DM_CONN_DATA_LEN_CHANGE_IND:
        {
            APP_TRACE_INFO2("DM_CONN_DATA_LEN_CHANGE_IND, Tx=%d, Rx=%d", ((hciLeDataLenChangeEvt_t*)pMsg)->maxTxOctets, ((hciLeDataLenChangeEvt_t*)pMsg)->maxRxOctets);
        }
        break;

        case AMDTP_CON_PARAM_UPDATE_TIMER_IND:
        {
            amdtpsConnUpdateTimerExpire();
        }
        break;

        case AMDTP_SEND_DATA_TIMER_IND:
        {
            amdtpsSendNotifyData();
        }
        break;

        case AMDTP_PHY_UPDATE_TIMER_IND:
        {
            amdtpsPhyUpdateTimerExpire();
        }
        break;

        case AMDTP_READ_EVT_CNT_TIMER_IND:
        {
            uint8_t conn_id = AppConnIsOpen();
            HciVsGetConEventCounter(conn_id);
            WsfTimerStartSec(&amdtpsConnEnv.readEvtCntTimer, AMDTP_READ_EVT_CNT_TIMER_SEC);
        }
        break;

        case DM_RESET_CMPL_IND:
        {
            // set database hash calculating status to true until a new hash is generated after reset
            attsCsfSetHashUpdateStatus(TRUE);

            // Generate ECC key if configured support secure connection,
            // else will calcualte ATT database hash
            if ( amdtpSecCfg.auth & DM_AUTH_SC_FLAG )
            {
                DmSecGenerateEccKeyReq();
            }
            else
            {
                AttsCalculateDbHash();
            }

            uiEvent = APP_UI_RESET_CMPL;
        }
        break;

        case ATTS_DB_HASH_CALC_CMPL_IND:
        {
            amdtpSetup(pMsg);
        }
        break;

        case DM_ADV_START_IND:
        {
            uiEvent = APP_UI_ADV_START;
        }
        break;

        case DM_ADV_STOP_IND:
        {
            uiEvent = APP_UI_ADV_STOP;
        }
        break;

        case DM_CONN_OPEN_IND:
        {
            amdtps_proc_msg(&pMsg->hdr);
            DmConnSetDataLen(1, 251, 0x848);
            // start timer to read connection event counter
            WsfTimerStartSec(&amdtpsConnEnv.readEvtCntTimer, AMDTP_READ_EVT_CNT_TIMER_SEC);

            uiEvent = APP_UI_CONN_OPEN;
        }
        break;

        case DM_CONN_CLOSE_IND:
        {
            amdtps_proc_msg(&pMsg->hdr);

            APP_TRACE_INFO1("DM_CONN_CLOSE_IND, reason = 0x%x", pMsg->dm.connClose.reason);

            amdtpsConnEnvCleanup();

            uiEvent = APP_UI_CONN_CLOSE;
        }
        break;

        case DM_CONN_UPDATE_IND:
        {
            amdtps_proc_msg(&pMsg->hdr);

            hciLeConnUpdateCmplEvt_t *evt = (hciLeConnUpdateCmplEvt_t*) pMsg;

            if (evt->status == 0)
            {
                // Enable single tx if connection interval is less than 7.5ms, otherwise, disable it
                if ( evt->connInterval < HCI_CONN_INTERVAL_7P5MS )
                {
                    HciVsEnableSingleTx(pMsg->hdr.param, true);

                    amdtpsConnEnv.txLen = random_num(AMDTP_SEND_DATA_LEN_2P5MS_MAX, AMDTP_SEND_DATA_LEN_MIN);
                }
                else
                {
                    HciVsEnableSingleTx(pMsg->hdr.param, false);

                    amdtpsConnEnv.txLen = random_num(AMDTP_SEND_DATA_LEN_MAX, AMDTP_SEND_DATA_LEN_MIN);

                    if ( evt->connLatency )
                    {
                        //Enable latency
                        HciVsSetSlaveLatency(pMsg->hdr.param, evt->connLatency);
                    }
                }

                // Start randomized timer for PHY update after connection parameter update completes,
                //  or else there maybe transaction conflict.
                WsfTimerStartMs(&amdtpsConnEnv.phyUpdateTimer, random_num(AMDTP_PHY_UPDATE_PER_MS_MAX, AMDTP_PHY_UPDATE_PER_MS_MIN));
            }
            else
            {
                APP_TRACE_INFO1("Peripheral failed to update parameter,%d.", evt->status);
            }
        }
        break;

        case DM_PHY_UPDATE_IND:
        {
            APP_TRACE_INFO3("DM_PHY_UPDATE_IND status: %d, RX: %d, TX: %d", pMsg->dm.phyUpdate.status, pMsg->dm.phyUpdate.rxPhy, pMsg->dm.phyUpdate.txPhy);
        }
        break;

        case DM_SEC_PAIR_CMPL_IND:
        {
            DmSecGenerateEccKeyReq();

            uiEvent = APP_UI_SEC_PAIR_CMPL;
        }
        break;

        case DM_SEC_PAIR_FAIL_IND:
        {
            DmSecGenerateEccKeyReq();

            APP_TRACE_INFO1("DM_SEC_PAIR_FAIL_IND, status = 0x%x", pMsg->dm.pairCmpl.hdr.status);

            uiEvent = APP_UI_SEC_PAIR_FAIL;
        }
        break;

        case DM_SEC_ENCRYPT_IND:
        {
            // Will trigger connection update 5 seconds after encrytion
            WsfTimerStartSec(&amdtpsConnEnv.connUpdateTimer, AMDTP_START_CON_UPDATE_TIMER_SEC);
            // Will trigger sending date 10 seconds after encryption
            WsfTimerStartSec(&amdtpsConnEnv.dataTxTimer, AMDTP_CON_START_TX_TIMER_SEC);
            uiEvent = APP_UI_SEC_ENCRYPT;
        }
        break;

        case DM_SEC_ENCRYPT_FAIL_IND:
        {
            uiEvent = APP_UI_SEC_ENCRYPT_FAIL;
        }
        break;

        case DM_SEC_AUTH_REQ_IND:
        {
            AppHandlePasskey(&pMsg->dm.authReq);
        }
        break;

        case DM_SEC_ECC_KEY_IND:
        {
            DmSecSetEccKey(&pMsg->dm.eccMsg.data.key);
            // Only calculate database hash if the calculating status is in progress
            if ( attsCsfGetHashUpdateStatus() )
            {
                AttsCalculateDbHash();
            }
        }
        break;

        case DM_SEC_COMPARE_IND:
        {
            AppHandleNumericComparison(&pMsg->dm.cnfInd);
        }
        break;

        case DM_HW_ERROR_IND:
        {
            uiEvent = APP_UI_HW_ERROR;
        }
        break;

        case DM_VENDOR_SPEC_CMD_CMPL_IND:
        {
            switch (pMsg->dm.vendorSpecCmdCmpl.opcode)
            {
                case HCI_DBG_GET_CON_EVT_CNT_CMD_OPCODE:
                {
                    if ( pMsg->dm.vendorSpecCmdCmpl.hdr.status == HCI_SUCCESS )
                    {
                        uint8_t link_id = 0;
                        uint8_t *p_evt = pMsg->dm.vendorSpecCmdCmpl.param;

                        BSTREAM_TO_UINT8(link_id, p_evt);

                        if ( !amdtpsConnEnv.firstTimeRdEvtCnt )
                        {
                            amdtpsConnEnv.firstTimeRdEvtCnt = true;
                            BSTREAM_TO_UINT16(amdtpsConnEnv.lastConEvtCnt, p_evt);
                            APP_TRACE_INFO1("last evt cnt:%d", amdtpsConnEnv.lastConEvtCnt);
                        }
                        else
                        {
                            uint16_t current_con_evt_cnt = 0;
                            BSTREAM_TO_UINT16(current_con_evt_cnt, p_evt);
                            APP_TRACE_INFO2("link_id:%d, con evt cnt:%d\r\n", link_id, current_con_evt_cnt);

                            if ( current_con_evt_cnt == amdtpsConnEnv.lastConEvtCnt )
                            {
                                APP_TRACE_ERR0("connection lost\r\n");

                                HciDrvRadioShutdown();
                                HciDrvRadioBoot(0);
                                DmDevReset();

                            }
                            amdtpsConnEnv.lastConEvtCnt = current_con_evt_cnt;
                        }
                    }
                    else
                    {
                        APP_TRACE_ERR1("rd con evt cmd failed, status:0x%x\r\n", pMsg->dm.vendorSpecCmdCmpl.hdr.status);

                        HciDrvRadioShutdown();
                        HciDrvRadioBoot(0);
                        DmDevReset();
                    }
                }
                break;

                default:
                {
                    APP_TRACE_INFO2("VSC 0x%0x complete status %x",
                        pMsg->dm.vendorSpecCmdCmpl.opcode,
                        pMsg->hdr.status);
                }
                break;
            }
        }
        break;

        default:
        break;
    }

    if (uiEvent != APP_UI_NONE)
    {
        AppUiAction(uiEvent);
    }
}

/*************************************************************************************************/
/*!
 *  \fn     AmdtpHandlerInit
 *
 *  \brief  Application handler init function called during system initialization.
 *
 *  \param  handlerID  WSF handler ID.
 *
 *  \return None.
 */
/*************************************************************************************************/
void AmdtpHandlerInit(wsfHandlerId_t handlerId)
{
    APP_TRACE_INFO0("AmdtpHandlerInit");

    /* store handler ID */
    amdtpHandlerId = handlerId;

    /* Set configuration pointers */
    pAppAdvCfg = (appAdvCfg_t *) &amdtpAdvCfg;
    pAppSlaveCfg = (appSlaveCfg_t *) &amdtpSlaveCfg;
    pAppSecCfg = (appSecCfg_t *) &amdtpSecCfg;
    pAppUpdateCfg = (appUpdateCfg_t *) &amdtpUpdateCfg;
    pSmpCfg = (smpCfg_t *) &amdtpSmpCfg;

    amdtpsConnEnv.connUpdateTimer.handlerId = handlerId;
    amdtpsConnEnv.connUpdateTimer.msg.event = AMDTP_CON_PARAM_UPDATE_TIMER_IND;
    amdtpsConnEnv.pConnTestInt = conn_interval;

    amdtpsConnEnv.phyUpdateTimer.handlerId = handlerId;
    amdtpsConnEnv.phyUpdateTimer.msg.event = AMDTP_PHY_UPDATE_TIMER_IND;
    amdtpsConnEnv.pUpdatePhy = upate_phy;
    amdtpsConnEnv.dataTxTimer.handlerId = handlerId;
    amdtpsConnEnv.dataTxTimer.msg.event = AMDTP_SEND_DATA_TIMER_IND;

    amdtpsConnEnv.readEvtCntTimer.handlerId = handlerId;
    amdtpsConnEnv.readEvtCntTimer.msg.event = AMDTP_READ_EVT_CNT_TIMER_IND;

    // Initialize Tx Data
    for (uint16_t i = 0; i < DATA_LEN_MAX; i++)
    {
        amdtpsConnEnv.txData[i] = (i & 0xFF) % 0x100;
    }

    /* Initialize application framework */
    AppSlaveInit();
    AppServerInit();
}

/*************************************************************************************************/
/*!
 *  \fn     AmdtpHandler
 *
 *  \brief  WSF event handler for application.
 *
 *  \param  event   WSF event mask.
 *  \param  pMsg    WSF message.
 *
 *  \return None.
 */
/*************************************************************************************************/
void AmdtpHandler(wsfEventMask_t event, wsfMsgHdr_t *pMsg)
{
    if (pMsg != NULL)
    {
        /* process ATT messages */
        if (pMsg->event >= ATT_CBACK_START && pMsg->event <= ATT_CBACK_END)
        {
            /* process server-related ATT messages */
            AppServerProcAttMsg(pMsg);
        }
        /* process DM messages */
        else if (pMsg->event >= DM_CBACK_START && pMsg->event <= DM_CBACK_END)
        {
            /* process advertising and connection-related messages */
            AppSlaveProcDmMsg((dmEvt_t *) pMsg);

            /* process security-related messages */
            AppSlaveSecProcDmMsg((dmEvt_t *) pMsg);
        }

        /* perform profile and user interface-related operations */
        amdtpProcMsg((amdtpMsg_t *) pMsg);
    }
}

/*************************************************************************************************/
/*!
 *  \fn     AmdtpStart
 *
 *  \brief  Start the application.
 *
 *  \return None.
 */
/*************************************************************************************************/
void AmdtpStart(void)
{
    /* Register for stack callbacks */
    DmRegister(amdtpDmCback);
    DmConnRegister(DM_CLIENT_ID_APP, amdtpDmCback);
    AttRegister(amdtpAttCback);
    AttConnRegister(AppServerConnCback);
    AttsCccRegister(AMDTP_NUM_CCC_IDX, (attsCccSet_t *) amdtpCccSet, amdtpCccCback);

    /* Register for app framework callbacks */
    AppUiBtnRegister(amdtpBtnCback);

    /* Initialize attribute server database */
    SvcCoreGattCbackRegister(GattReadCback, GattWriteCback);
    SvcCoreAddGroup();
    SvcDisAddGroup();
    SvcAmdtpsCbackRegister(NULL, amdtps_write_cback);
    SvcAmdtpsAddGroup();

    /* Set Service Changed CCCD index. */
    GattSetSvcChangedIdx(AMDTP_GATT_SC_CCC_IDX);
    /* Reset the device */
    DmDevReset();
}

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************
