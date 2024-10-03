//*****************************************************************************
//
//! @file hci_vs_cooper.c
//!
//! @brief  HCI vendor specific functions for generic controllers.
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
#include "wsf_types.h"
#include "wsf_msg.h"
#include "wsf_trace.h"
#include "bda.h"
#include "bstream.h"
#include "hci_core.h"
#include "hci_api.h"
#include "hci_main.h"
#include "hci_cmd.h"
#include "am_mcu_apollo.h"
#include "hci_dbg_trc.h"
#include "hci_drv_cooper.h"

#if HCI_VS_TARGET == HCI_VS_GENERIC

// #### INTERNAL BEGIN ####
#define READ_COOPER_INFO0_DURING_RESET_SEQUENCE         0
#if READ_COOPER_INFO0_DURING_RESET_SEQUENCE
#include "am_util_debug.h"

#define COOPER_INFO0_BASE                               0x60000000
#define COOPER_INFO0_SIGNATURE_OFFSET                   0x00
#define COOPER_INFO0_LDOCCR_TRIM_OFFSET                 0x04
#define COOPER_INFO0_BGCR_TRIM_OFFSET                   0x08
#define COOPER_INFO0_CLOCK_TRIM_OFFSET                  0x0C
#define COOPER_INFO0_DACSPICR_TRIM_OFFSET               0x10
#define COOPER_INFO0_DCDC_TRIM_OFFSET                   0x14
#define COOPER_INFO0_BOR_TRIM_OFFSET                    0x18
#define COOPER_INFO0_RF_TRIM_OFFSET                     0x1C
#define COOPER_INFO0_TX_POWER_6DBM_TRIM_OFFSET          0x20
#define COOPER_INFO0_TX_POWER_4DBM_TRIM_OFFSET          0x24
#define COOPER_INFO0_TX_POWER_3DBM_TRIM_OFFSET          0x28
#define COOPER_INFO0_TX_POWER_0DBM_TRIM_OFFSET          0x2C
#define COOPER_INFO0_TX_POWER_M5DBM_TRIM_OFFSET         0x30
#define COOPER_INFO0_TX_POWER_M10DBM_TRIM_OFFSET        0x34
#define COOPER_INFO0_TX_POWER_M15DBM_TRIM_OFFSET        0x38
#define COOPER_INFO0_TX_POWER_M20DBM_TRIM_OFFSET        0x3C
#define COOPER_INFO0_2M_TX_POWER_6DBM_TRIM_OFFSET       0x40
#define COOPER_INFO0_2M_TX_POWER_4DBM_TRIM_OFFSET       0x44
#define COOPER_INFO0_2M_TX_POWER_3DBM_TRIM_OFFSET       0x48
#define COOPER_INFO0_2M_TX_POWER_0DBM_TRIM_OFFSET       0x4C
#define COOPER_INFO0_2M_TX_POWER_M5DBM_TRIM_OFFSET      0x50
#define COOPER_INFO0_2M_TX_POWER_M10DBM_TRIM_OFFSET     0x54
#define COOPER_INFO0_2M_TX_POWER_M15DBM_TRIM_OFFSET     0x58
#define COOPER_INFO0_2M_TX_POWER_M20DBM_TRIM_OFFSET     0x5C
#define COOPER_INFO0_MODEX_TRIM_VER_OFFSET              0x60
#define COOPER_INFO0_CHIP_VER_OFFSET                    0xF4
#define COOPER_INFO0_AUTH_DISABLE_OFFSET                0xF8
#define COOPER_INFO0_WRITE_DISABLE_OFFSET               0xFC
#define COOPER_INFO0_CHIPID0_OFFSET                     0x100
#define COOPER_INFO0_CHIPID1_OFFSET                     0x104

#define READ_INFO0_1M_TX_POWER_TRIM_VALUE               0
#define READ_INFO0_2M_TX_POWER_TRIM_VALUE               0

static uint8_t cooper_info0_read_offset[] = {COOPER_INFO0_SIGNATURE_OFFSET,
#if READ_INFO0_1M_TX_POWER_TRIM_VALUE
                                             COOPER_INFO0_TX_POWER_6DBM_TRIM_OFFSET,
                                             COOPER_INFO0_TX_POWER_4DBM_TRIM_OFFSET,
                                             COOPER_INFO0_TX_POWER_3DBM_TRIM_OFFSET,
                                             COOPER_INFO0_TX_POWER_0DBM_TRIM_OFFSET,
                                             COOPER_INFO0_TX_POWER_M5DBM_TRIM_OFFSET,
                                             COOPER_INFO0_TX_POWER_M10DBM_TRIM_OFFSET,
                                             COOPER_INFO0_TX_POWER_M15DBM_TRIM_OFFSET,
                                             COOPER_INFO0_TX_POWER_M20DBM_TRIM_OFFSET,
#endif
#if READ_INFO0_2M_TX_POWER_TRIM_VALUE
                                             COOPER_INFO0_2M_TX_POWER_6DBM_TRIM_OFFSET,
                                             COOPER_INFO0_2M_TX_POWER_4DBM_TRIM_OFFSET,
                                             COOPER_INFO0_2M_TX_POWER_3DBM_TRIM_OFFSET,
                                             COOPER_INFO0_2M_TX_POWER_0DBM_TRIM_OFFSET,
                                             COOPER_INFO0_2M_TX_POWER_M5DBM_TRIM_OFFSET,
                                             COOPER_INFO0_2M_TX_POWER_M10DBM_TRIM_OFFSET,
                                             COOPER_INFO0_2M_TX_POWER_M15DBM_TRIM_OFFSET,
                                             COOPER_INFO0_2M_TX_POWER_M20DBM_TRIM_OFFSET,

#endif
                                             COOPER_INFO0_MODEX_TRIM_VER_OFFSET,
                                             COOPER_INFO0_CHIP_VER_OFFSET};
#endif
// #### INTERNAL END ####
/**************************************************************************************************
  Local Functions
**************************************************************************************************/

static void hciCoreReadResolvingListSize(void);
static void hciCoreReadMaxDataLen(void);
/*************************************************************************************************/
/*!
 *  \fn     hciCoreReadResolvingListSize
 *
 *  \brief  Read resolving list command.
 *
 *  \return None.
 */
/*************************************************************************************************/
static void hciCoreReadResolvingListSize(void)
{
  /* if LL Privacy is supported by Controller and included */
  if ((hciCoreCb.leSupFeat & HCI_LE_SUP_FEAT_PRIVACY) &&
      (hciLeSupFeatCfg & HCI_LE_SUP_FEAT_PRIVACY))
  {
    /* send next command in sequence */
    HciLeReadResolvingListSize();
  }
  else
  {
    hciCoreCb.resListSize = 0;

    /* send next command in sequence */
    hciCoreReadMaxDataLen();
  }
}

/*************************************************************************************************/
/*!
 *  \fn     hciCoreReadMaxDataLen
 *
 *  \brief  Read maximum data length command.
 *
 *  \return None.
 */
/*************************************************************************************************/
static void hciCoreReadMaxDataLen(void)
{
  /* if LE Data Packet Length Extensions is supported by Controller and included */
  if ((hciCoreCb.leSupFeat & HCI_LE_SUP_FEAT_DATA_LEN_EXT) &&
      (hciLeSupFeatCfg & HCI_LE_SUP_FEAT_DATA_LEN_EXT))
  {
    /* send next command in sequence */
    HciLeReadMaxDataLen();
  }
  else
  {
    /* send next command in sequence */
    HciLeRandCmd();
  }
}

/*************************************************************************************************/
/*!
 *  \fn     hciCoreResetStart
 *
 *  \brief  Start the HCI reset sequence.
 *
 *  \return None.
 */
/*************************************************************************************************/
void hciCoreResetStart(void)
{
  /* send an HCI Reset command to start the sequence */
  //HciResetCmd();

  //Free up any unsent HCI commandss
  hciClearCmdQueue();

  /* send VS NVDS command to start the sequence */
  HciVscUpdateNvdsParam();

  // update Bluetooth Address to controller
  //HciVscUpdateBDAddress();

}

/*************************************************************************************************/
/*!
 *  \fn     hciCoreResetSequence
 *
 *  \brief  Implement the HCI reset sequence.
 *
 *  \param  pMsg    HCI event message from previous command in the sequence.
 *
 *  \return None.
 */
/*************************************************************************************************/
void hciCoreResetSequence(uint8_t *pMsg)
{
  uint16_t       opcode;
  wsfMsgHdr_t    hdr;
  static uint8_t randCnt;
// #### INTERNAL BEGIN ####
#if READ_COOPER_INFO0_DURING_RESET_SEQUENCE
  static uint8_t info0Cnt;
#endif
// #### INTERNAL END ####

  /* if event is a command complete event */
  if (*pMsg == HCI_CMD_CMPL_EVT)
  {
    /* parse parameters */
    pMsg += HCI_EVT_HDR_LEN;
    pMsg++;                   /* skip num packets */
    BSTREAM_TO_UINT16(opcode, pMsg);
    pMsg++;                   /* skip status */

    /* decode opcode */
    switch (opcode)
    {
      case HCI_VSC_UPDATE_NVDS_CFG_CMD_OPCODE:
        HciVscUpdateLinklayerFeature();
      break;

      case HCI_VSC_UPDATE_LL_FEATURE_CFG_CMD_OPCODE:
        HciResetCmd();
        HciVscUpdateBDAddress();
      break;

      case HCI_OPCODE_RESET:
        /* initialize rand command count */
        randCnt = 0;

        #ifdef ENABLE_BLE_CTRL_TRACE
        HciVscSetTraceBitMap(TRACE_BITMAP);
        #endif

        #if ENABLE_SPECIFIED_EVENT_MASK
        HciVscConfigEvtMask(CFG_EVENT_MASK);
        #endif

        /* send next command in sequence */
        HciSetEventMaskCmd((uint8_t *) hciEventMask);
        break;

      case HCI_OPCODE_SET_EVENT_MASK:
        /* send next command in sequence */
        HciLeSetEventMaskCmd((uint8_t *) hciLeEventMask);
        break;

      case HCI_OPCODE_LE_SET_EVENT_MASK:
        /* send next command in sequence */
        HciSetEventMaskPage2Cmd((uint8_t *) hciEventMaskPage2);
        break;

      case HCI_OPCODE_SET_EVENT_MASK_PAGE2:
        /* send next command in sequence */
        HciReadBdAddrCmd();
        break;

      case HCI_OPCODE_READ_BD_ADDR:
        /* parse and store event parameters */
        BdaCpy(hciCoreCb.bdAddr, pMsg);

        /* send next command in sequence */
        HciLeReadBufSizeCmd();
        break;

      case HCI_OPCODE_LE_READ_BUF_SIZE:
        /* parse and store event parameters */
        BSTREAM_TO_UINT16(hciCoreCb.bufSize, pMsg);
        BSTREAM_TO_UINT8(hciCoreCb.numBufs, pMsg);
        hciCoreCb.availBufs = hciCoreCb.numBufs;

        /* send next command in sequence */
        HciLeReadSupStatesCmd();
        break;

      case HCI_OPCODE_LE_READ_SUP_STATES:
        /* parse and store event parameters */
        memcpy(hciCoreCb.leStates, pMsg, HCI_LE_STATES_LEN);

        /* send next command in sequence */
        HciLeReadWhiteListSizeCmd();
        break;

      case HCI_OPCODE_LE_READ_WHITE_LIST_SIZE:
        /* parse and store event parameters */
        BSTREAM_TO_UINT8(hciCoreCb.whiteListSize, pMsg);

        /* send next command in sequence */
        HciLeReadLocalSupFeatCmd();
        break;

      case HCI_OPCODE_LE_READ_LOCAL_SUP_FEAT:
        /* parse and store event parameters */
        BSTREAM_TO_UINT16(hciCoreCb.leSupFeat, pMsg);

        /* send next command in sequence */
        hciCoreReadResolvingListSize();
        break;

      case HCI_OPCODE_LE_READ_RES_LIST_SIZE:
        /* parse and store event parameters */
        BSTREAM_TO_UINT8(hciCoreCb.resListSize, pMsg);

        /* send next command in sequence */
        hciCoreReadMaxDataLen();
        break;

      case HCI_OPCODE_LE_READ_MAX_DATA_LEN:
        {
          uint16_t maxTxOctets;
          uint16_t maxTxTime;

          BSTREAM_TO_UINT16(maxTxOctets, pMsg);
          BSTREAM_TO_UINT16(maxTxTime, pMsg);

          /* use Controller's maximum supported payload octets and packet duration times
           * for transmission as Host's suggested values for maximum transmission number
           * of payload octets and maximum packet transmission time for new connections.
           */
          HciLeWriteDefDataLen(maxTxOctets, maxTxTime);
        }
        break;

      case HCI_OPCODE_LE_WRITE_DEF_DATA_LEN:
        if (hciCoreCb.extResetSeq)
        {
          /* send first extended command */
          (*hciCoreCb.extResetSeq)(pMsg, opcode);
        }
        else
        {
          /* initialize extended parameters */
          hciCoreCb.maxAdvDataLen = 0;
          hciCoreCb.numSupAdvSets = 0;
          hciCoreCb.perAdvListSize = 0;

          /* send next command in sequence */
          HciLeRandCmd();
        }
        break;

      case HCI_OPCODE_LE_READ_MAX_ADV_DATA_LEN:
      case HCI_OPCODE_LE_READ_NUM_SUP_ADV_SETS:
      case HCI_OPCODE_LE_READ_PER_ADV_LIST_SIZE:
      case HCI_OPCODE_READ_LOCAL_VER_INFO:
        if (hciCoreCb.extResetSeq)
        {
          /* send next extended command in sequence */
          (*hciCoreCb.extResetSeq)(pMsg, opcode);
        }
        break;

      case HCI_OPCODE_LE_RAND:
        /* check if need to send second rand command */
        if (randCnt < (HCI_RESET_RAND_CNT-1))
        {
          randCnt++;
          HciLeRandCmd();
        }
// #### INTERNAL BEGIN ####
#if READ_COOPER_INFO0_DURING_RESET_SEQUENCE
        else
        {
          info0Cnt = 0;
          HciVscReadReg(COOPER_INFO0_BASE + cooper_info0_read_offset[0]);
        }
        break;

      case HCI_VSC_REG_RD_CMD_OPCODE:
        if (info0Cnt < sizeof(cooper_info0_read_offset))
        {
          uint32_t info0Addr;
          uint32_t info0Value;

          BSTREAM_TO_UINT32(info0Addr, pMsg);
          BSTREAM_TO_UINT32(info0Value, pMsg);

          am_util_debug_printf("Cooper Info0 address 0x%08x, value 0x%08x\n", info0Addr, info0Value);

          info0Cnt++;
          HciVscReadReg(COOPER_INFO0_BASE + cooper_info0_read_offset[info0Cnt]);
        }
#endif
// #### INTERNAL END ####
        else
        {
          /* last command in sequence; set resetting state and call callback */
          hciCb.resetting = FALSE;
          hdr.param = 0;
          hdr.event = HCI_RESET_SEQ_CMPL_CBACK_EVT;
          (*hciCb.evtCback)((hciEvt_t *) &hdr);
        }
        break;

      default:
        break;
    }
  }
}

/*************************************************************************************************/
/*!
 *  \fn     hciCoreVsCmdCmplRcvd
 *
 *  \brief  Perform internal HCI processing of vendor specific command complete events.
 *
 *  \param  opcode  HCI command opcode.
 *  \param  pMsg    Pointer to input HCI event parameter byte stream.
 *  \param  len     Parameter byte stream length.
 *
 *  \return HCI callback event code or zero.
 */
/*************************************************************************************************/
uint8_t hciCoreVsCmdCmplRcvd(uint16_t opcode, uint8_t *pMsg, uint8_t len)
{
  return HCI_VENDOR_SPEC_CMD_CMPL_CBACK_EVT;
}

/*************************************************************************************************/
/*!
 *  \fn     hciCoreVsEvtRcvd
 *
 *  \brief  Perform internal HCI processing of vendor specific HCI events.
 *
 *  \param  p       Pointer to input HCI event parameter byte stream.
 *  \param  len     Parameter byte stream length.
 *
 *  \return HCI callback event code or zero.
 */
/*************************************************************************************************/
uint8_t hciCoreVsEvtRcvd(uint8_t *p, uint8_t len)
{
  return HCI_VENDOR_SPEC_EVT;
}

/*************************************************************************************************/
/*!
 *  \fn     hciCoreHwErrorRcvd
 *
 *  \brief  Perform internal HCI processing of hardware error event.
 *
 *  \param  p       Pointer to input HCI event parameter byte stream.
 *
 *  \return HCI callback event code or zero.
 */
/*************************************************************************************************/
uint8_t hciCoreHwErrorRcvd(uint8_t *p)
{
  return 0;
}

/*************************************************************************************************/
/*!
 *  \fn     HciVsInit
 *
 *  \brief  Vendor-specific controller initialization function.
 *
 *  \param  param    Vendor-specific parameter.
 *
 *  \return None.
 */
/*************************************************************************************************/
void HciVsInit(uint8_t param)
{

}


#endif
