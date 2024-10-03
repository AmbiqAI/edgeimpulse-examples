//*****************************************************************************
//
//! @file peripheral_main.c
//!
//! @brief Peripheral sample application for the L2C CoC connection.
//!
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#include <string.h>
#include "wsf_types.h"
#include "util/bstream.h"
#include "wsf_msg.h"
#include "wsf_trace.h"
#include "hci_api.h"
#include "dm_api.h"
#include "att_api.h"
#include "smp_api.h"
#include "app_api.h"
#include "app_db.h"
#include "app_ui.h"
#include "app_hw.h"
#include "app_main.h"
#include "svc_ch.h"
#include "svc_core.h"
#include "gatt/gatt_api.h"
#include "atts_main.h"
#include "l2c_api.h"
#include "l2c_handler.h"
#include "am_util_stdio.h"
#include "peripheral_main.h"

/**************************************************************************************************
  Macros
**************************************************************************************************/
#define PERIPHERAL_CONN_MAX                  1

/**************************************************************************************************
  Data Types
**************************************************************************************************/

/* CoC channel registration ID */
l2cCocRegId_t adaptCoCId;

/*! Application message type */
typedef union
{
  wsfMsgHdr_t     hdr;
  dmEvt_t         dm;
  attsCccEvt_t    ccc;
  attEvt_t        att;
} peripheral_Msg_t;

/**************************************************************************************************
  Configurable Parameters
**************************************************************************************************/

/*! configurable parameters for advertising */
static const appAdvCfg_t peripheral_AdvCfg =
{
  {0,     0,     0},                  /*! Advertising durations in ms */
  {  800,     0,     0}                   /*! Advertising intervals in 0.625 ms units */
};

/*! configurable parameters for slave */
static const appSlaveCfg_t peripheral_SlaveCfg =
{
  PERIPHERAL_CONN_MAX,                           /*! Maximum connections */
};

/*! configurable parameters for security */
static const appSecCfg_t peripheral_SecCfg =
{
#ifndef PAIRING_TEST
  DM_AUTH_BOND_FLAG |
#endif
    DM_AUTH_SC_FLAG,    /*! Authentication and bonding flags */
  0,                                      /*! Initiator key distribution flags */
  DM_KEY_DIST_LTK,                        /*! Responder key distribution flags */
  FALSE,                                  /*! TRUE if Out-of-band pairing data is present */
  FALSE                                    /*! TRUE to initiate security upon connection */
};

/*! configurable parameters for connection parameter update */
static const appUpdateCfg_t peripheral_UpdateCfg =
{
  3000,                                      /*! Connection idle period in ms before attempting
                                              connection parameter update; set to zero to disable */
  48,                                    /*! Minimum connection interval in 1.25ms units */
  60,                                    /*! Maximum connection interval in 1.25ms units */
  4,                                      /*! Connection latency */
  600,                                    /*! Supervision timeout in 10ms units */
  5                                       /*! Number of update attempts before giving up */
};

/*! SMP security parameter configuration */
static const smpCfg_t peripheral_SmpCfg =
{
  3000,                                   /*! 'Repeated attempts' timeout in msec */
  SMP_IO_NO_IN_NO_OUT,                    /*! I/O Capability */
  7,                                      /*! Minimum encryption key length */
  16,                                     /*! Maximum encryption key length */
  3,                                      /*! Attempts to trigger 'repeated attempts' timeout */
  0,                                      /*! Device authentication requirements */
  64000,                                  /*! Maximum repeated attempts timeout in msec */
  64000,                                  /*! Time msec before attemptExp decreases */
  2                                       /*! Repeated attempts multiplier exponent */
};

/**************************************************************************************************
  Advertising Data
**************************************************************************************************/

/*! advertising data, discoverable mode */
static const uint8_t peripheral_AdvDataDisc[] =
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
  9,                                      /*! length */
  DM_ADV_TYPE_16_UUID,                    /*! AD type */
  UINT16_TO_BYTES(ATT_UUID_HEART_RATE_SERVICE),
  UINT16_TO_BYTES(ATT_UUID_RUNNING_SPEED_SERVICE),
  UINT16_TO_BYTES(ATT_UUID_DEVICE_INFO_SERVICE),
  UINT16_TO_BYTES(ATT_UUID_BATTERY_SERVICE)
};

/*! scan data, discoverable mode */
static const uint8_t peripheral_ScanDataDisc[] =
{
  /*! device name */
  15,                                      /*! length */
  DM_ADV_TYPE_LOCAL_NAME,                 /*! AD type */
  'P',
  'E',
  'R',
  'I',
  'P',
  'H',
  'E',
  'R',
  'A',
  'L',
  '_',
  'C',
  'o',
  'C'
};

/**************************************************************************************************
  Client Characteristic Configuration Descriptors
**************************************************************************************************/

/*! enumeration of client characteristic configuration descriptors */
enum
{
  PERIPHERAL_GATT_SC_CCC_IDX,                    /*! GATT service, service changed characteristic */
  PERIPHERAL_NUM_CCC_IDX
};

#if 1
/*! client characteristic configuration descriptors settings, indexed by above enumeration */
static const attsCccSet_t peripheral_CccSet[PERIPHERAL_NUM_CCC_IDX] =
{
  /* cccd handle          value range               security level */
  {GATT_SC_CH_CCC_HDL,    ATT_CLIENT_CFG_INDICATE,  DM_SEC_LEVEL_NONE}   /* PERIPHERAL_GATT_SC_CCC_IDX */
};
#endif

/**************************************************************************************************
  Global Variables
**************************************************************************************************/

/*! WSF handler ID */
wsfHandlerId_t peripheral_HandlerId;


/*************************************************************************************************/
/*!
 *  \brief  Application DM callback.
 *
 *  \param  pDmEvt  DM callback event
 *
 *  \return None.
 */
/*************************************************************************************************/
static void peripheral_DmCback(dmEvt_t *pDmEvt)
{
  dmEvt_t *pMsg;
  uint16_t len;

  len = DmSizeOfEvt(pDmEvt);

  if ((pMsg = WsfMsgAlloc(len)) != NULL)
  {
    memcpy(pMsg, pDmEvt, len);
    WsfMsgSend(peripheral_HandlerId, pMsg);
  }
}

/*************************************************************************************************/
/*!
 *  \brief  Application ATT callback.
 *
 *  \param  pEvt    ATT callback event
 *
 *  \return None.
 */
/*************************************************************************************************/
static void peripheral_AttCback(attEvt_t *pEvt)
{
  attEvt_t *pMsg;

  if ((pMsg = WsfMsgAlloc(sizeof(attEvt_t) + pEvt->valueLen)) != NULL)
  {
    memcpy(pMsg, pEvt, sizeof(attEvt_t));
    pMsg->pValue = (uint8_t *) (pMsg + 1);
    memcpy(pMsg->pValue, pEvt->pValue, pEvt->valueLen);
    WsfMsgSend(peripheral_HandlerId, pMsg);
  }
}

/*************************************************************************************************/
/*!
 *  \brief  Application ATTS client characteristic configuration callback.
 *
 *  \param  pDmEvt  DM callback event
 *
 *  \return None.
 */
/*************************************************************************************************/
static void peripheral_CccCback(attsCccEvt_t *pEvt)
{
  attsCccEvt_t  *pMsg;
  appDbHdl_t    dbHdl;

  /* If CCC not set from initialization and there's a device record and currently bonded */
  if ((pEvt->handle != ATT_HANDLE_NONE) &&
      ((dbHdl = AppDbGetHdl((dmConnId_t) pEvt->hdr.param)) != APP_DB_HDL_NONE) &&
      AppCheckBonded((dmConnId_t)pEvt->hdr.param))
  {
    /* Store value in device database. */
    AppDbSetCccTblValue(dbHdl, pEvt->idx, pEvt->value);
  }

  if ((pMsg = WsfMsgAlloc(sizeof(attsCccEvt_t))) != NULL)
  {
    memcpy(pMsg, pEvt, sizeof(attsCccEvt_t));
    WsfMsgSend(peripheral_HandlerId, pMsg);
  }
}

/*************************************************************************************************/
/*!
 *  \brief  Process CCC state change.
 *
 *  \param  pMsg    Pointer to message.
 *
 *  \return None.
 */
/*************************************************************************************************/
static void peripheral_ProcCccState(peripheral_Msg_t *pMsg)
{
  APP_TRACE_INFO3("ccc state ind value:%d handle:%d idx:%d", pMsg->ccc.value, pMsg->ccc.handle, pMsg->ccc.idx);
}

/*************************************************************************************************/
/*!
 *  \brief  Set up advertising and other procedures that need to be performed after
 *          device reset.
 *
 *  \param  pMsg    Pointer to message.
 *
 *  \return None.
 */
/*************************************************************************************************/
static void peripheral_Setup(peripheral_Msg_t *pMsg)
{
  /* set advertising and scan response data for discoverable mode */
  AppAdvSetData(APP_ADV_DATA_DISCOVERABLE, sizeof(peripheral_AdvDataDisc), (uint8_t *) peripheral_AdvDataDisc);
  AppAdvSetData(APP_SCAN_DATA_DISCOVERABLE, sizeof(peripheral_ScanDataDisc), (uint8_t *) peripheral_ScanDataDisc);

  /* set advertising and scan response data for connectable mode */
  AppAdvSetData(APP_ADV_DATA_CONNECTABLE, sizeof(peripheral_AdvDataDisc), (uint8_t *) peripheral_AdvDataDisc);
  AppAdvSetData(APP_SCAN_DATA_CONNECTABLE, sizeof(peripheral_ScanDataDisc), (uint8_t *) peripheral_ScanDataDisc);

  /* start advertising; automatically set connectable/discoverable mode and bondable mode */
  AppAdvStart(APP_MODE_AUTO_INIT);
}

/*************************************************************************************************/
/*!
 *  \brief  Button press callback.
 *
 *  \param  btn    Button press.
 *
 *  \return None.
 */
/*************************************************************************************************/
static void peripheral_BtnCback(uint8_t btn)
{
  dmConnId_t      connId;
 // static uint8_t  heartRate = 78;    /* for testing/demonstration */

  /* button actions when connected */
  if ((connId = AppConnIsOpen()) != DM_CONN_ID_NONE)
  {
    switch (btn)
    {
      case APP_UI_BTN_1_SHORT:
        break;

      case APP_UI_BTN_1_MED:
        break;

      case APP_UI_BTN_1_LONG:
        AppConnClose(connId);
        break;

      case APP_UI_BTN_2_SHORT:
        break;

      case APP_UI_BTN_2_MED:
        break;

      case APP_UI_BTN_2_LONG:
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
        /* start or restart advertising */
        AppAdvStart(APP_MODE_AUTO_INIT);
        break;

      case APP_UI_BTN_1_MED:
        /* enter discoverable and bondable mode mode */
        AppSetBondable(TRUE);
        AppAdvStart(APP_MODE_DISCOVERABLE);
        break;

      case APP_UI_BTN_1_LONG:
        /* clear all bonding info */
        AppSlaveClearAllBondingInfo();

        /* restart advertising */
        AppAdvStart(APP_MODE_AUTO_INIT);
        break;

      default:
        break;
    }
  }
}

/*************************************************************************************************/
/*!
 *  \brief  Process messages from the event handler.
 *
 *  \param  pMsg    Pointer to message.
 *
 *  \return None.
 */
/*************************************************************************************************/
static void peripheral_ProcMsg(peripheral_Msg_t *pMsg)
{
  uint8_t uiEvent = APP_UI_NONE;
  uint16_t ret;

  switch ( pMsg->hdr.event )
  {
    case ATTS_CCC_STATE_IND:
      peripheral_ProcCccState(pMsg);
      break;

    case ATT_MTU_UPDATE_IND:
      APP_TRACE_INFO1("Negotiated MTU %d", ((attEvt_t *)pMsg)->mtu);
      break;

    case DM_RESET_CMPL_IND:
      // set database hash calculating status to true until a new hash is generated after reset
      attsCsfSetHashUpdateStatus(TRUE);

      // Generate ECC key if configured support secure connection,
      // else will calcualte ATT database hash
      if ( peripheral_SecCfg.auth & DM_AUTH_SC_FLAG )
      {
          DmSecGenerateEccKeyReq();
      }
      else
      {
          AttsCalculateDbHash();
      }

      uiEvent = APP_UI_RESET_CMPL;
      break;

    case ATTS_DB_HASH_CALC_CMPL_IND:
      peripheral_Setup(pMsg);
      break;
    case DM_ADV_SET_START_IND:
      uiEvent = APP_UI_ADV_SET_START_IND;
      break;

    case DM_ADV_SET_STOP_IND:
      uiEvent = APP_UI_ADV_SET_STOP_IND;
      break;

     case DM_ADV_START_IND:
      uiEvent = APP_UI_ADV_START;
      break;

    case DM_ADV_STOP_IND:
      uiEvent = APP_UI_ADV_STOP;
      break;

    case DM_CONN_OPEN_IND:
      ret = L2cCocConnectReq((dmConnId_t)pMsg->hdr.param, adaptCoCId, L2C_CoC_PSM);
      am_util_stdio_printf("l2c Coc req, ret:%d\r\n", ret);
      uiEvent = APP_UI_CONN_OPEN;
      break;

    case DM_CONN_CLOSE_IND:
      uiEvent = APP_UI_CONN_CLOSE;
      break;

    case DM_PHY_UPDATE_IND:
      APP_TRACE_INFO3("DM_PHY_UPDATE_IND status: %d, RX: %d, TX: %d", pMsg->dm.phyUpdate.status, pMsg->dm.phyUpdate.rxPhy, pMsg->dm.phyUpdate.txPhy);
      break;

    case DM_SEC_PAIR_CMPL_IND:
      DmSecGenerateEccKeyReq();
      uiEvent = APP_UI_SEC_PAIR_CMPL;
      break;

    case DM_SEC_PAIR_FAIL_IND:
      DmSecGenerateEccKeyReq();
      uiEvent = APP_UI_SEC_PAIR_FAIL;
      break;

    case DM_SEC_ENCRYPT_IND:
      uiEvent = APP_UI_SEC_ENCRYPT;
      break;

    case DM_SEC_ENCRYPT_FAIL_IND:
      uiEvent = APP_UI_SEC_ENCRYPT_FAIL;
      break;

    case DM_SEC_AUTH_REQ_IND:
      AppHandlePasskey(&pMsg->dm.authReq);
      break;

    case DM_SEC_ECC_KEY_IND:
      DmSecSetEccKey(&pMsg->dm.eccMsg.data.key);
      // Only calculate database hash if the calculating status is in progress
      if ( attsCsfGetHashUpdateStatus() )
      {
        AttsCalculateDbHash();
      }
      break;

    case DM_SEC_COMPARE_IND:
      AppHandleNumericComparison(&pMsg->dm.cnfInd);
      break;

    case DM_PRIV_CLEAR_RES_LIST_IND:
      APP_TRACE_INFO1("Clear resolving list status 0x%02x", pMsg->hdr.status);
      break;

    case DM_HW_ERROR_IND:
      uiEvent = APP_UI_HW_ERROR;
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
 *  \brief  Application handler init function called during system initialization.
 *
 *  \param  handlerID  WSF handler ID.
 *
 *  \return None.
 */
/*************************************************************************************************/
void PeripheralHandlerInit(wsfHandlerId_t handlerId)
{
  APP_TRACE_INFO0("PeripheralHandlerInit");

  /* store handler ID */
  peripheral_HandlerId = handlerId;

  /* Set configuration pointers */
  pAppAdvCfg = (appAdvCfg_t *) &peripheral_AdvCfg;
  pAppSlaveCfg = (appSlaveCfg_t *) &peripheral_SlaveCfg;
  pAppSecCfg = (appSecCfg_t *) &peripheral_SecCfg;
  pAppUpdateCfg = (appUpdateCfg_t *) &peripheral_UpdateCfg;

  /* Initialize application framework */
  AppSlaveInit();
  AppServerInit();

  /* Set stack configuration pointers */
  pSmpCfg = (smpCfg_t *) &peripheral_SmpCfg;
}

/*************************************************************************************************/
/*!
 *  \brief  WSF event handler for application.
 *
 *  \param  event   WSF event mask.
 *  \param  pMsg    WSF message.
 *
 *  \return None.
 */
/*************************************************************************************************/
void PeripheralHandler(wsfEventMask_t event, wsfMsgHdr_t *pMsg)
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
    peripheral_ProcMsg((peripheral_Msg_t *) pMsg);
  }
}

/*************************************************************************************************/
/*!
 *  \brief  The L2CAP CoC connection callback function.
 *
 *  \param  pMsg    Pointer to message structure.
 *
 *  \return None.
 */
/*************************************************************************************************/
void ConnectionCallback(l2cCocEvt_t *pMsg)
{
  switch (pMsg->hdr.event)
  {
    case L2C_COC_CONNECT_IND:
    {
     uint8_t test_data[] = {0x12, 0x34, 0x56, 0x78, 0xAB};
     APP_TRACE_INFO1("L2C_COC_CONNECT_IND- [%s]", __func__);
     /*Send an L2CAP data packet through connection oriented CID */
     APP_TRACE_INFO1("cid:0x%x\r\n", pMsg->connectInd.cid);
     L2cCocDataReq(pMsg->connectInd.cid, sizeof(test_data), test_data);
    }
     break;

    case L2C_COC_DISCONNECT_IND:
     APP_TRACE_INFO1("L2C_COC_DISCONNECT_IND- [%s]", __func__);
     L2cCocDisconnectReq(adaptCoCId);
     break;

    case L2C_COC_DATA_IND:
     APP_TRACE_INFO1("L2C_COC_DATA_IND- [%s]", __func__);
     for ( uint8_t i = 0; i < pMsg->dataInd.dataLen; i++ )
     {
         am_util_stdio_printf("%02x ", pMsg->dataInd.pData[i]);
     }
     break;

    case L2C_COC_DATA_CNF:
     APP_TRACE_INFO1("L2C_COC_DATA_CNF- [%s]", __func__);
     break;

    default:
     break;
  }
}

/*************************************************************************************************/
/*!
 *  \brief  Start the application.
 *
 *  \return None.
 */
/*************************************************************************************************/
void Peripheral_Start(void)
{
  /* Register to CoC channel */
  l2cCocReg_t ConnectionRegistration;
  /* Protocol service multiplexer*/
  ConnectionRegistration.psm = L2C_CoC_PSM;
  /* Maximum receive data packet size */
  ConnectionRegistration.mtu = 251;
  /* Maximum receive PDU fragment size */
  ConnectionRegistration.mps = 251;
  /* Data packet receive credits for this channel */
  ConnectionRegistration.credits = 10;
  /* TRUE if authorization is required */
  ConnectionRegistration.authoriz = FALSE;
  /* Channel minimum security level requirements*/
  ConnectionRegistration.secLevel = 0;
  /* Channel initiator/acceptor role*/
  ConnectionRegistration.role = (L2C_COC_ROLE_INITIATOR | L2C_COC_ROLE_ACCEPTOR);

  adaptCoCId = L2cCocRegister(ConnectionCallback, &ConnectionRegistration);
  am_util_stdio_printf("L2Cap CoC Register: %d\r\n", adaptCoCId);


  /* Register for stack callbacks */
  DmRegister(peripheral_DmCback);
  DmConnRegister(DM_CLIENT_ID_APP, peripheral_DmCback);
  AttRegister(peripheral_AttCback);
  AttConnRegister(AppServerConnCback);
  AttsCccRegister(PERIPHERAL_NUM_CCC_IDX, (attsCccSet_t *) peripheral_CccSet, peripheral_CccCback);

  /* Register for app framework callbacks */
  AppUiBtnRegister(peripheral_BtnCback);

  /* Initialize attribute server database */
  SvcCoreGattCbackRegister(GattReadCback, GattWriteCback);
  SvcCoreAddGroup();

  /* Set Service Changed CCCD index. */
  GattSetSvcChangedIdx(PERIPHERAL_GATT_SC_CCC_IDX);

  /* Reset the device */
  DmDevReset();
}
